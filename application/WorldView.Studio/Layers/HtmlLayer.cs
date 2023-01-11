using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using Awesomium.Core;
using Awesomium.Windows.Forms;
using Pyxis.UI.Layers;
using Pyxis.WorldView.Studio.Layers.Html;
using Tao.OpenGl;

namespace Pyxis.WorldView.Studio.Layers
{
    internal class HtmlLayer : BaseLayer
    {
        private OpenGLSurface OpenGLSurface { get; set; }

        private BitmapSurface Surface
        {
            get { return OpenGLSurface.BitmapSurface; }
        }

        public WebView WebView { get; private set; }

        public Uri Uri
        {
            get { return WebView.Source; }
            set { WebView.Source = value; }
        }

        public bool Ready { get; private set; }
        public double Opacity { get; set; }

        public event EventHandler DocumentLoadingCompleted;

        private int m_overlayHandle = -1;
        private Size m_currentSize;

        private OpenGLProgram m_program;

        private bool m_followMouseEvents;
        private bool m_consumeNextMouseClick;
        private MouseButton m_followMouseDownButton = MouseButton.Left;


        public HtmlLayer(Size size, WebSession session)
            : this(size, session, null)
        {
        }

        public HtmlLayer(Size size, WebSession session, IEnumerable<IAssetProvider> assetProviders)
            : base("HTML")
        {
            m_currentSize = size;
            Opacity = 1.0;

            if (assetProviders != null)
            {
                foreach (var provider in assetProviders)
                {
                    session.AddDataSource(provider.Name, new DataSourceWrapper(provider));
                }
            }

            WebView = WebCore.CreateWebView(m_currentSize.Width, m_currentSize.Height, session, WebViewType.Offscreen);

            WebView.IsTransparent = true;

            OpenGLSurface = new OpenGLSurface();
            WebView.Surface = OpenGLSurface;

            WebView.DocumentReady += WebView_DocumentReady;
            WebView.LoadingFrameComplete += WebView_LoadingFrameComplete;
            WebView.ConsoleMessage += (s, e) =>
            {
                Trace.info(String.Format("{0} : {1}[{2}] : {3}", e.EventType, e.Source, e.LineNumber, e.Message));
            };
            WebView.ShowCreatedWebView += (s, e) =>
            {
                e.Cancel = true;
                Process.Start(e.TargetURL.ToString());
            };
        }

        public void ApplyCursorChange(Control control)
        {
            WebView.CursorChanged += (s, e) => {
                                                   control.Cursor = e.GetCursor();
            };
        }

        private void WebView_LoadingFrameComplete(object sender, FrameEventArgs frameEventArgs)
        {
            if (!frameEventArgs.IsMainFrame)
            {
                //we are waiting on main frame only
                return;
            }

            if (m_proxyFactory == null)
            {
                m_proxyFactory = new JsProxyFactory(WebView, "PYX");

                foreach (var request in m_proxiesRequests)
                {
                    var proxyName = request.Key;
                    var createProxyAction = request.Value;

                    m_proxyFactory.CreateProxy(proxyName, createProxyAction);
                }

                m_proxiesRequests.Clear();
            }

            var handler = DocumentLoadingCompleted;

            if (handler != null)
            {
                handler.Invoke(this, new EventArgs());
            }
        }

        private void WebView_DocumentReady(object sender, UrlEventArgs e)
        {
            Ready = true;
            WebView.FocusView();
        }

        public override void HandleDispose()
        {
            if (m_program != null)
            {
                m_program.Delete();
            }
            WebView.Dispose();
            Surface.Dispose();
        }

        private JsProxyFactory m_proxyFactory;

        private readonly Dictionary<string, Action<JsProxy>> m_proxiesRequests =
            new Dictionary<string, Action<JsProxy>>();

        /// <summary>
        /// Register a Js Proxy to provide the embedded browser access to 
        /// C# callbacks.
        /// </summary>
        /// <param name="proxyName">name of the proxy</param>
        /// <param name="createProxyAction">A function that add C# bindings to the JS proxy</param>
        public void RegisterProxy(string proxyName, Action<JsProxy> createProxyAction)
        {
            if (m_proxyFactory != null)
            {
                m_proxyFactory.CreateProxy(proxyName, createProxyAction);
            }
            else
            {
                m_proxiesRequests[proxyName] = createProxyAction;
            }
        }

        public override void Paint()
        {
            if (!Ready || !WebView.IsLive) return;

            if (m_overlayHandle == -1)
            {
                var handles = new int[1];
                Gl.glGenTextures(1, handles);
                m_overlayHandle = handles[0];
            }

            var viewport = new int[4];
            Gl.glGetIntegerv(Gl.GL_VIEWPORT, viewport);

            Gl.glMatrixMode(Gl.GL_PROJECTION);
            Gl.glLoadIdentity();
            Glu.gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
            Gl.glMatrixMode(Gl.GL_MODELVIEW);
            Gl.glLoadIdentity();

            Gl.glEnable(Gl.GL_TEXTURE_2D);
            Gl.glEnable(Gl.GL_BLEND);
            Gl.glTexEnvi(Gl.GL_TEXTURE_ENV, Gl.GL_TEXTURE_ENV_MODE, Gl.GL_MODULATE);
            Gl.glBlendFunc(Gl.GL_SRC_ALPHA, Gl.GL_ONE_MINUS_SRC_ALPHA);
            Gl.glDisable(Gl.GL_DEPTH_TEST);
            Gl.glPolygonMode(Gl.GL_FRONT_AND_BACK, Gl.GL_FILL);

            Gl.glActiveTexture(Gl.GL_TEXTURE0);
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, m_overlayHandle);

            if (OpenGLSurface.WasModified)
            {
                if (OpenGLSurface.ModifiedRect.Width == Surface.Width &&
                    OpenGLSurface.ModifiedRect.Height == Surface.Height)
                {
                    Gl.glTexImage2D(Gl.GL_TEXTURE_2D, 0, Gl.GL_RGBA, Surface.Width, Surface.Height, 0, Gl.GL_BGRA,
                        Gl.GL_UNSIGNED_BYTE, Surface.Buffer);
                }
                else
                {
                    Gl.glPixelStorei(Gl.GL_UNPACK_ROW_LENGTH, Surface.Width);
                    var offset = (OpenGLSurface.ModifiedRect.Y*Surface.Width + OpenGLSurface.ModifiedRect.X)*4;
                    Gl.glTexSubImage2D(Gl.GL_TEXTURE_2D, 0, OpenGLSurface.ModifiedRect.X, OpenGLSurface.ModifiedRect.Y,
                        OpenGLSurface.ModifiedRect.Width, OpenGLSurface.ModifiedRect.Height, Gl.GL_BGRA,
                        Gl.GL_UNSIGNED_BYTE, Surface.Buffer + offset);
                    Gl.glPixelStorei(Gl.GL_UNPACK_ROW_LENGTH, 0);
                }
                Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_MIN_FILTER, Gl.GL_LINEAR);
                Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_MAG_FILTER, Gl.GL_LINEAR);
                Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_WRAP_S, Gl.GL_CLAMP_TO_EDGE);
                Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_WRAP_T, Gl.GL_CLAMP_TO_EDGE);
                OpenGLSurface.ClearModifiedRect();
            }
            else
            {
                OpenGLSurface.ClearModifiedRect();
            }

            if (m_program == null)
            {
                m_program = new OpenGLProgram()
                {
                    FragmentShaderCode =
                        @"uniform sampler2D tex;
void main()
{
    vec4 color = texture2D(tex,gl_TexCoord[0].st);
    gl_FragColor = color;
    if (gl_FragColor.a > 0.0)
    {
        gl_FragColor.rgb /= gl_FragColor.a;
    }
    gl_FragColor *= gl_Color;
}"
                };

                if (!m_program.Compile())
                {
                    System.Diagnostics.Trace.WriteLine("failed to compile html-layer shader");
                }
            }

            m_program.StartUsing();


            var heightDelta = m_currentSize.Height - Surface.Height;

            Gl.glBegin(Gl.GL_QUADS);

            Gl.glColor4f(1f, 1f, 1f, (float) Opacity);

            Gl.glTexCoord2f(0, 1);
            Gl.glVertex3d(0, heightDelta, 0);
            Gl.glTexCoord2f(1, 1);
            Gl.glVertex3d(Surface.Width, heightDelta, 0);
            Gl.glTexCoord2f(1, 0);
            Gl.glVertex3d(Surface.Width, Surface.Height + heightDelta, 0);
            Gl.glTexCoord2f(0, 0);
            Gl.glVertex3d(0, Surface.Height + heightDelta, 0);

            Gl.glEnd();

            m_program.StopUsing();

            Gl.glActiveTexture(Gl.GL_TEXTURE0);
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, 0);

            Gl.glDisable(Gl.GL_TEXTURE_2D);
            Gl.glDisable(Gl.GL_BLEND);
            Gl.glEnable(Gl.GL_DEPTH_TEST);

            //use this if you encounter openGL errors
            //error = Gl.glGetError();
        }

        public override void HandleResize(int width, int height)
        {
            m_currentSize = new Size(width, height);
            WebView.Resize(width, height);
        }

        public override bool HandleMouseDown(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            if (IsPixelTransparent(e.Location))
            {
                return false;
            }

            switch (e.Button)
            {
                case MouseButtons.Left:
                    m_followMouseDownButton = MouseButton.Left;
                    break;
                case MouseButtons.Right:
                    m_followMouseDownButton = MouseButton.Right;
                    break;
                case MouseButtons.Middle:
                    m_followMouseDownButton = MouseButton.Middle;
                    break;
            }

            //make sure we consume all mouse move,click and up events.
            WebView.InjectMouseMove(e.X, e.Y);
            WebView.InjectMouseDown(m_followMouseDownButton);
            m_followMouseEvents = true;

            //A mouse-click event is going to be trigger just after the MouseUp.
            //We need to make sure the HtmlLayer will consume it.
            m_consumeNextMouseClick = true;

            return true;
        }

        public override bool HandleMouseMove(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            //inject events to web core about mouse move always.
            WebView.InjectMouseMove(e.X, e.Y);

            if (!m_followMouseEvents && IsPixelTransparent(e.Location))
            {
                return false;
            }

            return m_followMouseEvents;
        }

        public override bool HandleMouseUp(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            if (!m_followMouseEvents)
            {
                return false;
            }

            WebView.InjectMouseMove(e.X, e.Y);
            WebView.InjectMouseUp(m_followMouseDownButton);
            m_followMouseEvents = false;

            return true;
        }

        public override bool HandleMouseWheel(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            if (IsPixelTransparent(e.Location))
            {
                return false;
            }

            WebView.InjectMouseWheel(e.Delta, 0);
            return true;
        }

        public override bool HandleMouseClick(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            if (m_consumeNextMouseClick)
            {
                //make sure we don't auto-consume the next event
                m_consumeNextMouseClick = false;

                //consume current mouse click event
                return true;
            }

            return !IsPixelTransparent(e.Location);
        }

        public override bool HandleMouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }

            return !IsPixelTransparent(e.Location);
        }

        public override bool HandleKeyUp(object sender, KeyEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }
            WebView.InjectKeyboardEvent(e.GetKeyboardEvent(WebKeyboardEventType.KeyUp));
            return true;
        }

        public override bool HandleKeyDown(object sender, KeyEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }
            WebView.InjectKeyboardEvent(e.GetKeyboardEvent(WebKeyboardEventType.KeyDown));
            return true;
        }

        public override bool HandleKeyPress(object sender, KeyPressEventArgs e)
        {
            if (!Ready || !WebView.IsLive)
            {
                return false;
            }
            WebView.InjectKeyboardEvent(e.GetKeyboardEvent());
            return true;
        }

        private bool IsPixelTransparent(Point point)
        {
            if (point.X < 0 || point.X >= Surface.Width || point.Y < 0 || point.Y >= Surface.Height)
            {
                return true;
            }
            return Surface.GetAlphaAtPoint(point.X, point.Y) == 0;
        }
    }
}