using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Awesomium.Core;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.Publishing;
using Pyxis.UI.Layers;
using Pyxis.Utilities;
using Pyxis.WorldView.Studio.JsAPI;
using Pyxis.WorldView.Studio.Layers;
using Pyxis.WorldView.Studio.Layers.Html;
using Pyxis.WorldView.Studio.Properties;

namespace Pyxis.WorldView.Studio
{
    public partial class ApplicationForm : Form
    {
        /// <summary>
        /// Pyxis Engine to be used by the application. 
        /// </summary>
        public Engine Engine { get; set; }

        /// <summary>
        /// The Pyxis.Core.EngineConfig that is used
        /// </summary>
        public EngineConfig EngineConfig { get; set; }

        /// <summary>
        /// The main UI HTML layer that gets rendered as OpenGL Layer on top of the globe
        /// </summary>
        internal HtmlLayer HtmlLayer { get; set; }

        /// <summary>
        /// The main UI HTML web session (store all cookies and cache)
        /// </summary>
        private WebSession WebSession { get; set; }

        /// <summary>
        /// A temporary HTML layer that is used for displaying the loading screen. this layer is been destroyed after the main HTML is get loaded.
        /// </summary>
        private HtmlLayer LoadingScreenHtmlLayer { get; set; }

        /// <summary>
        /// The Pyxis GlobeLayer. This layer render the 3d globe using OpenGL
        /// </summary>
        internal GlobeLayer GlobeLayer { get; set; }

        /// <summary>
        /// This variable indicate if the globe API need to be register once the engine is ready
        /// </summary>
        private bool m_globeApiReadyToRegister;
        
        internal ApplicationAPI ApplicationJsAPI { get; private set; }
        internal EngineAPI EngineJsAPI { get; private set; }
        internal GlobeAPI GlobeJsAPI { get; private set; }

        /// <summary>
        /// A service for the application to parse command line and DDE commands
        /// </summary>
        internal CommandLineParser CommandLineParser { get; set; }

        /// <summary>
        /// A service for the application to collect command line messages in a queue for the JS to parse
        /// </summary>
        internal CommandLineQueue CommandLineQueue { get; set; }

        /// <summary>
        /// The task that starts the engine.
        /// </summary>
        private Task StartEngine { get; set; }

        /// <summary>
        /// Handle to crash reporter object to help us generate mini dump
        /// </summary>
        private CrashReporter CrashReporter { get; set; }

        /// <summary>
        /// Create the application form.
        /// </summary>
        /// <param name="ddeManager">The DDE manager.</param>
        /// <param name="args">The command line arguments.</param>
        public ApplicationForm(CrashReporter crashReporter, DDEManager ddeManager, string[] args)
        {
            CrashReporter = crashReporter;

            InitializeComponent();

            //setup default engine config
            EngineConfig = EngineConfig.WorldViewDefault;            

            //create JsAPI to communicate with the webCore
            ApplicationJsAPI = new ApplicationAPI(this);
            EngineJsAPI = new EngineAPI(this);
            GlobeJsAPI = new GlobeAPI(this);

            //initialize Awesomium embedded browser
            
            //TODO: we need disable this on release?
            var config = new WebConfig();
            config.RemoteDebuggingPort = 8001;
            config.LogPath = Path.Combine(EngineConfig.WorkingDirectory, "Awesomium.log");
            WebCore.Initialize(config);

            //create the main web session
            var webSessionDirectory = EngineConfig.WorkingDirectory + Path.DirectorySeparatorChar + EngineConfig.EngineName + ".WebSession";
            if (!Directory.Exists(webSessionDirectory))
            {
                Directory.CreateDirectory(webSessionDirectory);
            }

            var webSessionPreference = new WebPreferences
            {
                EnableGPUAcceleration = true,
                WebGL = true
            };

            WebSession = WebCore.CreateWebSession(webSessionDirectory, webSessionPreference);

            var imageProvider = ApplicationJsAPI.ImageStorage.CreateWebDataProvider();

            HtmlLayer = new HtmlLayer(
                PyxisView.Size,
                WebSession,
                new List<IAssetProvider>
                {
                    imageProvider
                }
            );

            HtmlLayer.WebView.AddressChanged += (s, e) =>
            {
                urlTextBox.Text = HtmlLayer.WebView.Source.ToString();
            };
            HtmlLayer.ApplyCursorChange(PyxisView);

            //adding layers 
            PyxisView.AddLayer(new BackgroundLayer());
            PyxisView.AddLayer(HtmlLayer);
            CreateLoadingLayerMask();

            //parse command line arguments
            CommandLineParser = new CommandLineParser();

            //setup the startup url (will work only on command line args)
            string startupUrl = null;
            CommandLineParser.RegisterHandler("--startup", url =>
            {
                startupUrl = url;
            });

            //set the test directory (will work only on command line args)
            string testDirectory = null;
            CommandLineParser.RegisterHandler("--testdirectory", dir =>
            {
                testDirectory = dir;
            });

            //clear the cache (will work only on command line args)
            CommandLineParser.RegisterHandler("--clearcache", arg =>
            {
                EngineConfig.ClearCache = true;
            });

            //close the application - no questions asked
            CommandLineParser.RegisterHandler("--close", arg =>
            {
                this.InvokeIfRequired(Close);
            });

            //bring the application to front
            CommandLineParser.RegisterHandler("--focus", arg =>
            {
                this.InvokeIfRequired(Activate);
            });

            //create a queue of all command lines
            CommandLineQueue = CommandLineQueue.Attach(CommandLineParser);

            //bridge CommandLineParser with ddeManager
            ddeManager.DDECommandReceived += (s, e) =>
            {
                CommandLineParser.ParseCommand(e.Command);
            };

            //Register application JS API.
            //Note: this must be before the first command line args in order to let javascript handle startup command args            
            RegisterApplicationJsApi();

            //Parse startup command line args.
            CommandLineParser.ParseCommand(args);

            // Get the startup uri
            HtmlLayer.Uri = GetStartupUri(startupUrl);

            // Configure the engine
            EngineConfig.APIUrl = GetLicenseServerFromStartupUri(HtmlLayer.Uri);
            Engine = Engine.Create(EngineConfig);            

            // Start the engine, allow user to login while the engine is starting
            StartEngine = Task.Run(() => Engine.Start());

            // Create the globe layer after the engine finished initializing
            var uiScheduler = TaskScheduler.FromCurrentSynchronizationContext();
            Engine.WhenReady(() =>
            {
                // Write the product version to the trace log
                Trace.info(String.Format("WorldView Studio version: {0}", Application.ProductVersion));

                // Allow import of all supported GeoSource formats
                Engine.EnableAllImports();

                //add globe layer just below the HTML layer
                GlobeLayer = new GlobeLayer(PyxisView, Engine);
                PyxisView.AddLayer(1, GlobeLayer);

                // Register globe API if it wasn't registered already
                if (m_globeApiReadyToRegister)
                {
                    RegisterGlobeJsApi();
                }

                // if the Studio was launched with the testdirectory command line argument
                // test the geosources in the specified directory
                if (testDirectory != null)
                {
                    Thread testThread = new Thread(() =>
                    {
                        MessageBox.Show("Press OK to test data sources in " + testDirectory,
                            "Info",
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Information);

                        try
                        {
                            var importTest = new ImportTest(Engine);
                            importTest.TestLocalDataSources(testDirectory, EngineConfig.ClearCache);

                            MessageBox.Show("Done testing data sources in " + testDirectory,
                                "Info",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Information);

                        }
                        catch (Exception e)
                        {
                            MessageBox.Show("Failed to test data sources in " + testDirectory + " Error: " + e,
                                "Info",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Information);
                        }

                    });

                    testThread.Name = "Local Data Source Test Thread";
                    testThread.Start();                    
                }

            }, uiScheduler);
            // This will check an exception occurring when the engine fails to start,
            // while still letting the application run unblocked when no exception occurs
            if (StartEngine != null)
            {
                StartEngine.Wait();
            }
        }

        /// <summary>
        /// Get the license server url based on the startup uri.
        /// ***** This logic must match www.globalgridsystems.com *****
        /// </summary>
        /// <param name="startupUri">The startup uri.</param>
        private static string GetLicenseServerFromStartupUri(Uri startupUri)
        {
            string licenseServerUrl = null;

            // use production host by default
            licenseServerUrl = ApiUrl.ProductionLicenseServerRestAPI;

            // check for development host or test host
            if (startupUri.Host.ToLower().StartsWith(Settings.Default.DevelopmentStartupUriHost.ToLower()))
            {
                licenseServerUrl = ApiUrl.DevelopmentLicenseServerRestAPI;
            }
            else if (startupUri.Host.ToLower().StartsWith(Settings.Default.TestStartupUriHost.ToLower()))
            {
                licenseServerUrl = ApiUrl.TestLicenseServerRestAPI;
            }
            else if (startupUri.Host.ToLower() == "localhost")
            {
                if (startupUri.Query.ToLower().Contains("backend=test"))
                {
                    licenseServerUrl = ApiUrl.TestLicenseServerRestAPI;
                }
                else if (startupUri.Query.ToLower().Contains("backend=dev"))
                {
                    licenseServerUrl = ApiUrl.DevelopmentLicenseServerRestAPI;
                }
            }

            return licenseServerUrl;
        }


        private void CreateLoadingLayerMask()
        {
            //show loading screen (using embedded file so it will load fast)
            LoadingScreenHtmlLayer = new HtmlLayer(PyxisView.Size, WebSession);
            LoadingScreenHtmlLayer.Uri = new Uri("data:text/html," + Resources.LoadingPageHtml);

            PyxisView.AddLayer(LoadingScreenHtmlLayer);

            //attach event handler to hide this loading screen layer once the main layer complete to load
            HtmlLayer.DocumentLoadingCompleted += RemoveLoadingLayerMask;
        }

        private void RemoveLoadingLayerMask(object sender, EventArgs e)
        {
            HtmlLayer.DocumentLoadingCompleted -= RemoveLoadingLayerMask;
            PyxisView.Animate(progress => { LoadingScreenHtmlLayer.Opacity = 1 - progress; },
                TimeSpan.FromMilliseconds(300))
                .Then(() =>
                {
                    PyxisView.RemoveLayer(LoadingScreenHtmlLayer);
                    LoadingScreenHtmlLayer = null;
                });
        }

        private void ApplicationForm_Load(object sender, EventArgs e)
        {
            PyxisView.Focus();
            Activate();
        }

        private void ApplicationForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            WebCore.Shutdown();
        }

        private void RegisterGlobeJsApi()
        {
            GlobeJsAPI.Register(HtmlLayer);
        }

        private void RegisterEngineJsApi(Engine engine)
        {
            EngineJsAPI.Register(HtmlLayer);
        }

        private void RegisterApplicationJsApi()
        {
            ApplicationJsAPI.Register(HtmlLayer);
        }

        internal void AuthenticateEngine(User user)
        {
            if (EngineConfig.User != null)
            {
                throw new InvalidOperationException("Login can be done only once.");
            }

            Engine.WhenReady(() =>
            {
                this.BeginInvokeIfRequired(() =>
                {
                    Engine.AuthenticateAs(user);

                    if (GlobeLayer != null)
                    {
                        RegisterGlobeJsApi();
                    }
                    else
                    {
                        m_globeApiReadyToRegister = true;
                    }
                    RegisterEngineJsApi(Engine);
                });
            });
        }

        /// <summary>
        /// Get the startup uri. Use the command line argument if a valid uri, otherwise get the
        /// value from settings.
        /// </summary>
        /// <param name="urlStr">The command line argument or null if none specified.</param>
        /// <returns>The startup uri.</returns>
        private Uri GetStartupUri(string urlStr)
        {
            if (!string.IsNullOrEmpty(urlStr))
            {
                Uri uri;
                if (Uri.TryCreate(urlStr, UriKind.Absolute, out uri))
                {
                    return uri;
                }
            }

            return new Uri(Settings.Default.StartupUrl);
        }
       
        #region Titlebar and windows controls events handlers

        private void closeButton_Click(object sender, EventArgs e)
        {
            Close();
            WebCore.Shutdown();
        }

        private void maximizeButton_Click(object sender, EventArgs e)
        {
            WindowState = WindowState == FormWindowState.Maximized ? FormWindowState.Normal : FormWindowState.Maximized;
        }

        private void minimizeButton_Click(object sender, EventArgs e)
        {
            WindowState = FormWindowState.Minimized;
        }

        private void windowTitlePanel_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                DragDropHelper.StartWindowDrag(this);
            }
        }

        private void ApplicationForm_Resize(object sender, EventArgs e)
        {
            switch (WindowState)
            {
                case FormWindowState.Normal:
                    minimizeButton.Enabled = true;
                    maximizeButton.Enabled = true;
                    break;
                case FormWindowState.Maximized:
                    minimizeButton.Enabled = true;
                    maximizeButton.Enabled = true;
                    break;
                case FormWindowState.Minimized:
                    minimizeButton.Enabled = false;
                    maximizeButton.Enabled = true;
                    break;
            }
        }

        protected override void WndProc(ref Message m)
        {
            const int wmNcHitTest = 0x84;

            // Trap WM_NCHITTEST
            if (m.Msg == wmNcHitTest)
            {
                DragDropHelper.HandleNcHitTestMessage(this, ref m);
                return;
            }
            base.WndProc(ref m);
        }


        private void ApplicationForm_MouseMove(object sender, MouseEventArgs e)
        {
            const int buffer = 20;

            if (e.X < buffer)
            {
                if (e.Y < buffer)
                {
                    Cursor = Cursors.SizeNWSE;
                }
                else if (e.Y > Height - buffer)
                {
                    Cursor = Cursors.SizeNESW;
                }
                else
                {
                    Cursor = Cursors.SizeWE;
                }
            }
            else if (e.X > Width - buffer)
            {
                if (e.Y < buffer)
                {
                    Cursor = Cursors.SizeNESW;
                }
                else if (e.Y > Height - buffer)
                {
                    Cursor = Cursors.SizeNWSE;
                }
                else
                {
                    Cursor = Cursors.SizeWE;
                }
            }
            else
            {
                if (e.Y < buffer)
                {
                    Cursor = Cursors.SizeNS;
                }
                else if (e.Y > Height - buffer)
                {
                    Cursor = Cursors.SizeNS;
                }
            }
        }

        private void windowTitlePanel_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.DrawString(Text, Font, new SolidBrush(ForeColor), 20, 3);
        }

        private void urlTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode != Keys.Enter)
            {
                return;
            }

            try
            {
                HtmlLayer.Uri = new Uri(urlTextBox.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show("failed to parse uri: " + ex.Message);
            }
        }

        private void urlTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.Handled = e.SuppressKeyPress = true;
            }
        }

        private void PyxisView_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.F5)
            {
                HtmlLayer.WebView.Reload(true);
            }
            if (e.KeyCode == Keys.F12)
            {
                if (e.Shift && e.Control)
                {
                    //Ctrl+Shift+F12 = generate a dump file of current state. Useful when the UI is frozen or something bad happens before WV crashes
                    CrashReporter.CreateDump(null, MiniDump.ExceptionInfo.None);
                }
                else
                {
                    //F12 -> open up the url tab
                    urlPanel.Visible = !urlPanel.Visible;
                }
            }
            
        }
        
        #endregion Titlebar and windows controls events handlers        
    }

    /// <summary>
    /// helper class to pass arguments to screen capture api
    /// </summary>
    class ScreenCaptureArgs
    {
        public int Top { get; set; }
        public int Left { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
    }

    /// <summary>
    /// helper class to use win32 to make window drag and drop working
    /// </summary>
    static class DragDropHelper
    {
        public const int WM_NCLBUTTONDOWN = 0xA1;
        public const int HT_CAPTION = 0x2;

        [DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd,
                         int msg, int wParam, int lParam);
        [DllImport("user32.dll")]
        public static extern bool ReleaseCapture();

        public static void StartWindowDrag(Form form)
        {
            ReleaseCapture();
            SendMessage(form.Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
        }

        public static void HandleNcHitTestMessage(Form form, ref Message m)
        {
            const int htLeft = 10;
            const int htRight = 11;
            const int htTop = 12;
            const int htTopLeft = 13;
            const int htTopRight = 14;
            const int htBottom = 15;
            const int htBottomLeft = 16;
            const int htBottomRight = 17;

            int x = m.LParam.ToInt32() & 0x0000FFFF;
            int y = (int)((m.LParam.ToInt32() & 0xFFFF0000) >> 16);
            Point pt = form.PointToClient(new Point(x, y));
            Size clientSize = form.ClientSize;

            if (pt.X >= clientSize.Width - 16 && pt.Y >= clientSize.Height - 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(form.IsMirrored ? htBottomLeft : htBottomRight);
                return;
            }
            //allow resize on the lower left corner
            if (pt.X <= 16 && pt.Y >= clientSize.Height - 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(form.IsMirrored ? htBottomRight : htBottomLeft);
                return;
            }
            //allow resize on the upper right corner
            if (pt.X <= 16 && pt.Y <= 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(form.IsMirrored ? htTopRight : htTopLeft);
                return;
            }
            //allow resize on the upper left corner
            if (pt.X >= clientSize.Width - 16 && pt.Y <= 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(form.IsMirrored ? htTopLeft : htTopRight);
                return;
            }
            //allow resize on the top border
            if (pt.Y <= 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(htTop);
                return;
            }
            //allow resize on the bottom border
            if (pt.Y >= clientSize.Height - 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(htBottom);
                return;
            }
            //allow resize on the left border
            if (pt.X <= 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(htLeft);
                return;
            }
            //allow resize on the right border
            if (pt.X >= clientSize.Width - 16 && clientSize.Height >= 16)
            {
                m.Result = (IntPtr)(htRight);
                return;
            }
        }
    }
}
