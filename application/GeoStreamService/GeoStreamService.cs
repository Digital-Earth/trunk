/******************************************************************************
GeoStreamService.cs

begin		: May 13, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using HoytSoft.Common.Services;
using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Services.PipelineLibrary.Repositories.JsonRepository;
using Pyxis.Utilities.Shell;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using PyxNet.Pyxis;

namespace GeoStreamService
{
    [Service("PYXISGeoStreamServer",
        "PYXIS GeoStreamServer Service",
        "Publishes and shares geospatial data to be consumed by PYXIS WorldView clients.",
        AutoInstall = false,
        ServiceType = ServiceType.OwnProcess,
        ServiceStartType = ServiceStartType.AutoStart,
        ServiceControls = ServiceControls.StartAndStop | ServiceControls.Shutdown,
        LogName = "PYXISGeoStreamServer",
        ServiceAccessType = ServiceAccessType.AllAccess,
        ServiceErrorControl = ServiceErrorControl.Normal
    )]
    public partial class GeoStreamService : ServiceBase
    {
        /// <summary>
        /// place holder for service instance.
        /// this object is used to publish a certificate for the GeoStream Service that
        /// identifies what type of service we are.
        /// </summary>
        private PyxNet.GeoStreamServer.GeoStreamServerService m_geoStreamServerService = null;

        private PublishingManager m_publishingManager;

        /// <summary>
        /// Initializes a new instance of the <see cref="GeoStreamService"/> class.
        /// </summary>
        public GeoStreamService()
            : base()
        {
            ServerTypes parsedType;
            if (Enum.TryParse<ServerTypes>(Properties.Settings.Default.ServerType, out parsedType))
            {
                ServerType = parsedType;
            }
            else
            {
                ServerType = ServerTypes.Publisher;
            }
        }

        public static ServerTypes ServerType { get; private set; }

        public enum ServerTypes
        {
            Test,
            Processor,
            Publisher,
        }

        public enum TraceCategory
        {
            Import,
            Report,
            Download,
            Process,
            Publish,
            Error,
            CleanUp,
            GalleryTest
        }

        private static Dictionary<TraceCategory, ConsoleColor> s_color = new Dictionary<TraceCategory, ConsoleColor>()
        {
            {TraceCategory.Import,ConsoleColor.DarkCyan},
            {TraceCategory.Error,ConsoleColor.Red},
            {TraceCategory.Download,ConsoleColor.Yellow},
            {TraceCategory.Process,ConsoleColor.Gray},
            {TraceCategory.Report,ConsoleColor.DarkGray},
            {TraceCategory.Publish,ConsoleColor.Magenta},
            {TraceCategory.CleanUp,ConsoleColor.DarkRed},
            {TraceCategory.GalleryTest,ConsoleColor.Green}
        };

        static internal void WriteLine(TraceCategory category, string format, params object[] o)
        {
            Console.ForegroundColor = s_color[category];
            WriteLine(format, o);
            Console.ForegroundColor = ConsoleColor.White;
        }

        static internal void WriteLine(string format, params object[] o)
        {
            if (o.Length == 0)
            {
                Trace.info(format);
                Console.WriteLine(format);
            }
            else
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendFormat(format, o);

                Trace.info(sb.ToString());
                Console.WriteLine(sb.ToString());
            }
        }

        /// <summary>
        /// Initializes the GeoStream Server.
        /// Register message handlers.
        /// </summary>
        internal void InitializeGeoStreamServer()
        {
            m_geoStreamServerService =
                new PyxNet.GeoStreamServer.GeoStreamServerService(PyxNet.StackSingleton.Stack);

            // attach a channel to the stack to ensure we connect to the correct license server
            var channel = new Pyxis.Publishing.Channel(Properties.Settings.Default.LicenseServerRestAPI);
            PyxNet.StackSingleton.Stack.AttachChannel(channel);

            Trace.info(string.Format("GeoStreamServerService ID: {0}",
                PyxNet.GeoStreamServer.GeoStreamServerService.GeoStreamServerServiceId));

            Trace.info("Turn on usage reports.");
            Pyxis.Utilities.UsageReports.Initialize(AppServices.getLibraryPath().ToString(),
                                                     new UsageReportsHelper()
                                                     );
            PyxNet.StackSingleton.Stack.RegisterHandler(PyxNet.Publishing.UsageReportsMessage.MessageID, OnUsageReportsMessage);

            //--
            //-- dump out configuration settings.
            //--
            uint maxDataCacheSize = pyxlib.getAppProperty("WorldView", "MaxDataCacheSizeInMB", 2048);
            bool unlimitedCache = pyxlib.getAppProperty("WorldView", "Unlimited_Cache", false);

            Trace.info(string.Format("MaxDataCacheSizeInMB: {0}", maxDataCacheSize));
            Trace.info(string.Format("Unlimited_Cache: {0}", unlimitedCache.ToString()));
            Trace.info(string.Format("WorkingPath: {0}", AppServices.getWorkingPath().ToString()));
            Trace.info(string.Format("CachePath: {0}", AppServices.getBaseCachePath().ToString()));

            m_publishingManager = new PublishingManager(Properties.Settings.Default.LicenseServerRestAPI);
        }

        /// <summary>
        /// Uninitializes the GeoStream Server.
        /// Unregsiter message handlers.
        /// </summary>
        internal void UninitializeGeoStreamServer()
        {
            PyxNet.StackSingleton.Stack.UnregisterHandler(PyxNet.Publishing.UsageReportsMessage.MessageID, OnUsageReportsMessage);

            Trace.info("Turn off usage reports.");
            Pyxis.Utilities.UsageReports.Uninitialize();
        }

        #region Duplicated From WorldView

        /// <summary>
        ///
        /// </summary>
        private ApplicationUtility.ManagedChecksumCalculator m_managedChecksumCalculator;

        /// <summary>
        /// An instance of the database pipeline resolver.
        /// </summary>
        private static Pyxis.Services.PipelineLibrary.Repositories.Resolver m_resolver =
            new Pyxis.Services.PipelineLibrary.Repositories.Resolver();

        /// <summary>
        /// Initialize everything about the application libraries.
        /// </summary>
        internal void InitializeServices()
        {
            m_managedChecksumCalculator = new ApplicationUtility.ManagedChecksumCalculator(
                Pyxis.Utilities.ChecksumSingleton.Checksummer);
            ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr(
                m_managedChecksumCalculator));

            // Read the user's application space path
            String userAppPath = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            System.Diagnostics.Debug.Assert(!userAppPath.Equals(""));

#warning Hardcoded elements in application path.
            userAppPath += System.IO.Path.DirectorySeparatorChar +
                            "PYXIS" +//Properties.Resources.ParentApplicationFolder +
                            System.IO.Path.DirectorySeparatorChar +
                            "WorldView";// Properties.Resources.WorldViewFolder;

            // Create user application directory if it does not exist
            if (!System.IO.Directory.Exists(userAppPath))
            {
                System.IO.Directory.CreateDirectory(userAppPath);
            }

            //--
            //-- read WorldViewDataPath property, if it is set,
            //-- then use it as userAppPath to data
            //--
            string dataPath = Properties.Settings.Default.WorldViewDataPath;
            if (string.IsNullOrEmpty(dataPath) == false)
            {
                userAppPath = dataPath;
            }

            string applicationPath =
                System.Reflection.Assembly.GetExecutingAssembly().Location;
            applicationPath = System.IO.Path.GetDirectoryName(applicationPath);

            if (Environment.CurrentDirectory != applicationPath)
            {
                Log("GeoStreamService::CurrentDirectory was " + Environment.CurrentDirectory, EventLogEntryType.Warning);

                //GDAL module require the current directory to be the same as installed application directory to find "plugins\gdal_data" directory.
                Environment.CurrentDirectory = applicationPath;

                Log("GeoStreamService::CurrentDirectory set to " + Environment.CurrentDirectory, EventLogEntryType.Information);
            }

            PYXLibInstance.initialize(
                System.Reflection.Assembly.GetExecutingAssembly().GetName().Name,
                false,
                userAppPath, applicationPath);
            Trace.getInstance().setLevels(519);

            Pyxis.Utilities.ChecksumSingleton.Checksummer.ReadChecksumCache(
                AppServices.getCacheDir(
                ApplicationUtility.ManagedChecksumCalculator.GetDefaultCacheDirectory()) +
                ApplicationUtility.ManagedChecksumCalculator.GetDefaultCacheFilename());

            Log("GeoStreamService::InitializeServices\npath=" + userAppPath, EventLogEntryType.Information);
            Trace.info("userAppPath: " + userAppPath);

            ApplicationUtility.ManagedCSharpFunctionProvider provider =
                new ApplicationUtility.ManagedCSharpFunctionProvider();
            CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr(
                provider));

            ApplicationUtility.ManagedXMLDocumentProvider xmlProvider =
              new ApplicationUtility.ManagedXMLDocumentProvider();
            CSharpXMLDocProvider.setCSharpXMLDocProvider(new CSharpXMLDocProvider_SPtr(xmlProvider));

            ApplicationUtility.ManagedHttpRequestProvider httpProvider =
               new ApplicationUtility.ManagedHttpRequestProvider();
            HttpRequestProvider.setHttpRequestProvider(new HttpRequestProvider_SPtr(httpProvider));

            ApplicationUtility.ManagedBitmapServer.StartServer();

            InitializeDatabase();

            FileNotificationManager.getFileNeededNotifier().Event +=
                Pyxis.Core.Services.PyxlibService.HandleFileNeeded;

            FileNotificationManager.getPipelineFilesDownloadNeededNotifier().Event +=
                Pyxis.Core.Services.PyxNetService.HandlePipelineFilesNeeded;
        }

        /// <summary>
        /// Initializes NHibernate and opens a session to the underlying
        /// SQLite database.
        /// </summary>
        private static void InitializeDatabase()
        {
            var configuration = new Dictionary<string, string>();
            configuration.Add(
                "connection.provider",
                "NHibernate.Connection.DriverConnectionProvider");
            configuration.Add(
                "connection.driver_class",
                "NHibernate.Driver.SQLite20Driver");
            configuration.Add(
                "connection.connection_string",
                string.Format("Data Source={0};Version=3;",
                AppServices.getWorkingPath().ToString() +
                System.IO.Path.DirectorySeparatorChar +
                "PYXLibrary" + System.IO.Path.DirectorySeparatorChar +
#if FALSE
 "GeoStreamServer." + // Gene wants to use a unique library for each app.
#endif
 "PipelineLibrary.db"));
            configuration.Add(
                "dialect",
                "NHibernate.Dialect.SQLiteDialect");
            configuration.Add(
                "query.substitutions", "true=1;false=0");
            configuration.Add(
                "proxyfactory.factory_class",
                "NHibernate.ByteCode.Castle.ProxyFactoryFactory, NHibernate.ByteCode.Castle");
            configuration.Add(
                "show_sql",
                "false");

            PipelineRepository.SetPipelineImplementationType<JsonPipelineRepository>();

            // initialize NHibernate with the above configuration
            PipelineRepository.Instance.Initialize(configuration);

            // use of director to register the C# LibraryProcessResolver
            LibraryProcessResolver.set(
                new LibraryProcessResolver_SPtr(m_resolver));
            PipeManager.pushProcessResolver(new ProcessResolver_SPtr(
                LibraryProcessResolver.create().get()));

            Pyxis.Services.PipelineLibrary.Repositories.PipelineRepository.Instance.OnFatalError += (s, e) =>
            {
                Environment.FailFast("Database Error");
            };
        }

        /// <summary>
        /// Initialize the PYXNet services and network connections.
        /// </summary>
        internal void InitializePYXNet()
        {
            Trace.info("Initializing PyxNet services.");

            PyxNet.StackSingleton.Configuration = new GwssStackConfiguration();

            // Initialize stack singleton and its stack.
            PyxNet.StackSingleton.Stack.TimeBetweenRetries = TimeSpan.FromMinutes(1);
            PyxNet.StackSingleton.Stack.NodeInfo.Mode = PyxNet.NodeInfo.OperatingMode.Leaf;

            // Set the friendly name of the stack.
            String stackName = String.Format("GeoStreamServer {0}", PyxNet.StackSingleton.Stack.NodeInfo.FriendlyName);
            PyxNet.StackSingleton.Stack.NodeInfo.FriendlyName = stackName;

            // Connect stack singleton's stack to the hub(s).

            //--
            //-- connect to the pyxnet hub, via the stack
            //--
            string PyxNetHub = Properties.Settings.Default.PyxNetHub;
            if (string.IsNullOrEmpty(PyxNetHub))
            {
                //--
                //-- if there is no designated pyxnet hub, then connect to one
                //-- of the production hubs using NO parameters.
                //--
                Trace.info("PyxNet.Stack.Connect: hub=default");
                PyxNet.StackSingleton.Stack.Connect();
            }
            else
            {
                Trace.info(string.Format("PyxNet.Stack.Connect: hub={0}", PyxNetHub));
                PyxNet.StackSingleton.Stack.Connect(PyxNetHub);
            }

            // Hook up the coverage downloader system, so that remote processes can provide data.
            PyxNet.Pyxis.CoverageDownloader.InitializeCoverageDownloaderSupport();
            PyxNet.Pyxis.CoveragePublisher.UploadToStorage = true;

            // Hook up process Channel
            PyxNet.Pyxis.PyxlibPyxnetChannelProvider.StartServer();


            // Make sure our file checksums are up to date
            var downloadCacheFolder = PyxNet.FileTransfer.ManifestDownloader.CacheDirectory;
            var checksummer = Pyxis.Utilities.ChecksumSingleton.Checksummer;
            var foundFileWithMissingChecksum = false;
            foreach (var file in System.IO.Directory.EnumerateFiles(downloadCacheFolder, "*", System.IO.SearchOption.AllDirectories))
            {
                if (checksummer.getFileCheckSumFromCache(file) == "")
                {
                    foundFileWithMissingChecksum = true;
                    Console.Write("checksum(" + System.IO.Path.GetFileName(file) + ") = ");
                    Console.WriteLine(checksummer.getFileCheckSum_synch(file));
                }
            }
            if (foundFileWithMissingChecksum)
            {
                // Make sure it will persist next time
                checksummer.WriteChecksumCache();
            }

            // Make sure all previously-published processes are published.
            PyxNet.Pyxis.Republisher republisher = new PyxNet.Pyxis.Republisher(PyxNet.Pyxis.PublisherSingleton.Publisher);

            if (ServerType != ServerTypes.Processor)
            {
                republisher.PublishAllPipelines();
            }
            republisher.OnPipelinePublished +=
                new EventHandler<PyxNet.Pyxis.Republisher.PipelinePublishedEventArgs>(Republisher_OnPipelinePublished);

            Trace.info("PyxNet initialization procedures complete.");
        }

        private void Republisher_OnPipelinePublished(object sender, PyxNet.Pyxis.Republisher.PipelinePublishedEventArgs e)
        {
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Publish, "\nPipeline '{0}={1}' has been successfully republished.\n",
                PipeManager.getProcess(e.ProcRef).getProcName(), pyxlib.procRefToStr(e.ProcRef));
        }

        /// <summary>
        /// Clean up any resources before the end of the program run.
        /// </summary>
        internal void DestroyServices()
        {
            UninitializeGeoStreamServer();

            Trace.info("Releasing Managed objects from GeoWeb Stream Service.");
            GC.GetTotalMemory(true);
            GC.WaitForPendingFinalizers();

            FileNotificationManager.getFileNeededNotifier().Event -=
                Pyxis.Core.Services.PyxlibService.HandleFileNeeded;

            FileNotificationManager.getPipelineFilesDownloadNeededNotifier().Event -=
                Pyxis.Core.Services.PyxNetService.HandlePipelineFilesNeeded;

            Trace.info("Turn off PYXNET services.");
            //Stop publishing
            PyxNet.Pyxis.PublisherSingleton.Publisher.StopPublishing();
            PyxNet.Pyxis.PyxlibPyxnetChannelProvider.StopServer();

            //Stop Downloaders
            PyxNet.Pyxis.CoverageDownloaderManager.Manager.DetachAllDownloaders();
            PyxNet.Pyxis.CoverageDownloader.StopCoverageDownloaderSupport();

            Trace.info("Closing the PYXNet connection.");
            PyxNet.StackSingleton.Stack.Dispose();

            Trace.info("GeoWeb Stream Service Services destruction started.");
            CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr());
            CSharpXMLDocProvider.setCSharpXMLDocProvider(new CSharpXMLDocProvider_SPtr());
            HttpRequestProvider.setHttpRequestProvider(new HttpRequestProvider_SPtr());
            ApplicationUtility.ManagedBitmapServer.StopServer();

            Pyxis.Services.PipelineLibrary.Repositories.PipelineRepository.Instance.Uninitialize();

            ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr());
            Pyxis.Utilities.ChecksumSingleton.Checksummer.WriteChecksumCache();

            GC.GetTotalMemory(true);
            GC.WaitForPendingFinalizers();

            PYXLibInstance.uninitialize();
            Trace.setTraceCallback(null);
        }

        #endregion Duplicated From WorldView

        /// <summary>
        /// Initializes the specified args.
        /// </summary>
        /// <param name="args">The args.</param>
        /// <returns></returns>
        //protected override bool Initialize(string[] args)
        //{
        //    Log("GeoStreamService::Initialize", EventLogEntryType.Information);
        //    return base.Initialize(args);
        //}

        /// <summary>
        /// Starts this instance.
        /// </summary>
        protected override void Start()
        {
            AppDomain.CurrentDomain.UnhandledException += (sender, e) =>
            {
                var huh = e.ExceptionObject as Exception;
                if (huh == null)
                {
                    huh = new NotSupportedException(
                      "Unhandled exception doesn't derive from System.Exception: "
                       + e.ExceptionObject.ToString()
                    );
                }
                FatalExceptionHandler(huh);
            };

            Log("GeoStreamService::Start", EventLogEntryType.Information);

            using (Pyxis.Utilities.DDEManager ddeManager = new Pyxis.Utilities.DDEManager())
            {
                if (ddeManager.Initialize(new string[0]) == false)
                {
                    // Another GWSS is already running.  We have already sent our
                    // command-line on to the original app during the call to
                    // ddeManager.Initialize, so we can exit now.
                    Log("GeoStreamServer is exiting because another instance is running.", EventLogEntryType.Information);

                    return;
                }

                ddeManager.DDECommandReceived += HandleDDECommandReceived;

                Log("DDE Service Initialized", EventLogEntryType.Information);

                base.Start();

#if DEBUG
                if (this.Debug == false)
                {
                    //--
                    //-- Debug build, being run as a service.
                    //-- Launch VS debugger.
                    //--
                    System.Diagnostics.Debugger.Launch();
                }
#endif

                if (this.Debug == true)
                {
                    //--
                    //-- service is being run as console application.
                    //-- route trace messages to system console.
                    //--
                    //System.Diagnostics.Trace.Listeners.Add(
                    //    new System.Diagnostics.TextWriterTraceListener(
                    //        System.Console.Out));

                    Console.WriteLine("GeoWeb Stream Server is starting...");
                }

                try
                {
                    Console.WriteLine("InitializeServices...");
                    this.InitializeServices();
                    Console.WriteLine("InitializePYXNet...");
                    this.InitializePYXNet();
                    Console.WriteLine("InitializeGeoStreamServer...");
                    this.InitializeGeoStreamServer();
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to initialize GWSS due to error : " + ex.Message);
                    throw;
                }

                Log("Initialized services and PyxNet.", EventLogEntryType.Information);

                if (this.Debug == true)
                {
                    GeoStreamService.WriteLine("=== GeoWeb Stream Server is now running ===");
                    Console.WriteLine("=== Use 'x' to stop the server or 'h' to show help ===");

                    //--
                    //-- running service as console application
                    //-- Hang out forever(ish)
                    //--

                    HandleInputKeys();
                    Close();
                }
            }
        }

        private void FatalExceptionHandler(Exception huh)
        {
            //Send the log and exit
            Logging.Categories.Crashes.Error(huh);
            Pyxis.Utilities.Logging.LogRepository.Flush();
            Environment.Exit(1);
        }

        /// <summary>
        /// Stops this instance.
        /// </summary>
        protected override void Stop()
        {
            Log("GeoStreamService::Stop", EventLogEntryType.Information);
            m_publishingManager.Stop();
            this.DestroyServices();
            base.Stop();
        }

        private void Close()
        {
            GeoStreamService.WriteLine(
                "\nGeoWeb Stream Server is shutting down...  (Waiting for jobs to exit...)");

            // waits here for currently executing jobs to complete
            Jobs.ExitManager.ShouldExit = true;

            // let the user read the above message
            Thread.Sleep(TimeSpan.FromSeconds(2));

            this.Stop();
        }

        private IEnumerable<ConsoleKey> GetKeys()
        {
            while (true)
            {
                yield return Console.ReadKey().Key;
            }
        }

        private void HandleInputKeys()
        {
            var shell = new ShellEngine();
            var exit = false;
            shell.AddAction("x|exit|quit|shutdown", "shutdown server", () => { exit = true; });
            shell.AddAction("s|status", "show server status", () => { Console.WriteLine(m_publishingManager.CurrentStatus()); });
            shell.AddActionsFromType(typeof(ShellCommands));
            shell.AddActionsFromObject(m_publishingManager);
            while (!exit)
            {
                shell.ParseAndExecute();
            }
        }

        #region DDE Command Handler

        [DllImport("user32.dll", EntryPoint = "FindWindow", SetLastError = true)]
        private static extern IntPtr FindWindowByCaption(IntPtr zeroOnly, string lpWindowName);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        /// <summary>
        /// Handler that handles DDE commands received by the DDE Server.
        /// </summary>
        public void HandleDDECommandReceived(object sender, Pyxis.Utilities.DDECommandReceivedEventArgs e)
        {
            string originalTitle = Console.Title;
            string uniqueTitle = System.Diagnostics.Process.GetCurrentProcess().Id.ToString();
            Console.Title = uniqueTitle;
            Thread.Sleep(50); // give time to change the title (required)
            var handle = FindWindowByCaption(IntPtr.Zero, uniqueTitle);
            Console.Title = originalTitle;

            if (handle != IntPtr.Zero)
            {
                // Bring to front
                SetForegroundWindow(handle);
            }
            else
                Console.WriteLine("zero handle");

            ProcessArgument(e.Command);
        }

        private void ProcessArgument(string command)
        {
            Log("Processing DDE command \"" + command + "\"", EventLogEntryType.Information);
            if (string.IsNullOrEmpty(command))
            {
                return;
            }

            if (command.Contains("--close"))
            {
                this.Close();
            }
        }

        #endregion DDE Command Handler
    }
}