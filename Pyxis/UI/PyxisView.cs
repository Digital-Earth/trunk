using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Windows.Forms;
using Pyxis.Utilities;
using Tao.OpenGl;
using Tao.Platform.Windows;

namespace Pyxis.UI
{
    /// <summary>
    /// Encapsulates an ordered list of Pyxis.UI.ILayer and handles events interacting with those layers.
    /// </summary>
    public partial class PyxisView : UserControl
    {
        private SimpleOpenGlControl m_openGlControl;

        private int m_drawingRequest;        
        private List<ILayer> Layers { get; set; }

        /// <summary>
        /// Gets the rendering timing report.
        /// </summary>
        public RenderingTimeReport RenderingReport { get; private set; }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.PyxisView class.
        /// </summary>
        public PyxisView()
        {
            InitializeComponent();

            switch (LicenseManager.UsageMode) {
                case LicenseUsageMode.Runtime:
                    CreateOpenGLContext();
                    AttachEvents();
                    break;

                case LicenseUsageMode.Designtime:
                    //make sure our users understand this is a design time display
                    Paint += PyxisView_Paint;
                    break;
            }
            
            Layers = new List<ILayer>();
        }

        private void CreateOpenGLContext()
        {
            m_openGlControl = new SimpleOpenGlControl();
            m_openGlControl.Dock = DockStyle.Fill;
            Controls.Add(m_openGlControl);
            m_openGlControl.InitializeContexts();

            m_openGlControl.Paint += OpenGLContext_Paint;
        }

        // this method is get called from the Dispose method in PyxisView.Designer.cs
        // ReSharper disable once UnusedMember.Local
        private void DisposeOpenGlContext()
        {
            if (m_openGlControl != null)
            {
                m_openGlControl.Paint -= OpenGLContext_Paint;

                m_openGlControl.DestroyContexts();
                Controls.Remove(m_openGlControl);
                m_openGlControl.Dispose();
                m_openGlControl = null;
            }
        }

        private void AttachEvents()
        {
            if (m_openGlControl != null)
            {
                m_openGlControl.MouseDown += PyxisView_MouseDown;
                m_openGlControl.MouseUp += PyxisView_MouseUp;
                m_openGlControl.MouseMove += PyxisView_MouseMove;
                m_openGlControl.MouseClick += PyxisView_MouseClick;
                m_openGlControl.MouseDoubleClick += PyxisView_MouseDoubleClick;
                m_openGlControl.MouseWheel += PyxisView_MouseWheel;

                m_openGlControl.KeyDown += PyxisView_KeyDown;
                m_openGlControl.KeyUp += PyxisView_KeyUp;
                m_openGlControl.KeyPress += PyxisView_KeyPress;

                Resize += PyxisView_Resize;

                frameTimer.Tick += frameTimer_Tick;
            }
        }

        /// <summary>
        /// Add a Pyxis.UI.ILayer to the top of the Pyxis.UI.PyxisView layers.
        /// </summary>
        /// <param name="layer">The Pyxis.UI.ILayer to insert.</param>
        public void AddLayer(ILayer layer)
        {
            AddLayer(Layers.Count, layer);
        }

        /// <summary>
        /// Add a Pyxis.UI.ILayer to the Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="index">The index to insert the layer at.  0 is the bottom layer.</param>
        /// <param name="layer">The Pyxis.UI.ILayer to insert.</param>
        /// <exception cref="System.ArgumentOutOfRangeException">The specified <paramref name="index"/> is out of the range of layers.</exception>
        public void AddLayer(int index, ILayer layer)
        {
            Layers.Insert(index, layer);
            layer.HandleResize(Width, Height);
        }

        /// <summary>
        /// Remove a Pyxis.UI.ILayer from the Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="layer">The Pyxis.UI.ILayer to remove.</param>
        public void RemoveLayer(ILayer layer)
        {
            var foundIndex = Layers.IndexOf(layer);
            if (foundIndex != -1)
            {
                Layers.RemoveAt(foundIndex);
            }
            layer.HandleDispose();
        }

        /// <summary>
        /// Get all of the Pyxis.UI.ILayer in the Pyxis.UI.PyxisView.
        /// </summary>
        /// <returns>System.Collections.Generic.IEnumerable&lt;Pyxis.UI.ILayer&gt; that contains the layers.</returns>
        public IEnumerable<ILayer> GetLayers()
        {
            return new List<ILayer>(Layers);
        }

        /// <summary>
        /// Invoke action on all layers in reverse order until the first action that return true.
        /// This logic is to make mouseClick and KeyDown events work as expected.
        /// The drawing of layers is done in order - which mean the last layer is on top.
        /// Therefore, events should start propagate from the last layer to the first.
        /// A layer event handlers return true when events has been consumed, 
        /// Therefore, we can stop propagating the event down the layers.
        /// </summary>
        /// <param name="eventName">Name of event - used for tracing exceptions</param>
        /// <param name="action">action to be invoked on layers</param>
        private void DoActionOnLayers(string eventName, Func<ILayer, bool> action)
        {
            if (Layers == null) return;

            foreach (var layer in Layers.AsEnumerable().Reverse())
            {
                try
                {
                    if (layer.Visible && action(layer))
                    {
                        return;
                    }
                }
                catch (Exception ex)
                {
                    Trace.error("Error while handling event " + eventName + " layer '" + layer.Name + "': " + ex.Message);
                }
            }
        }

        private void PyxisView_MouseDown(object sender, MouseEventArgs e)
        {
            DoActionOnLayers("MouseDown", layer => layer.HandleMouseDown(sender, e));
            OnMouseDown(e);
        }

        private Point m_lastMouseMoveLocation = Point.Empty;        

        private void PyxisView_MouseMove(object sender, MouseEventArgs e)
        {
            //show tool tip can regenerate many non needed mouse move
            //we stop it before it get to the user...
            if (e.Location != m_lastMouseMoveLocation)
            {
                DoActionOnLayers("MouseMove", layer => layer.HandleMouseMove(sender, e));
                OnMouseMove(e);
                m_lastMouseMoveLocation = e.Location;
            }
        }

        private void PyxisView_MouseUp(object sender, MouseEventArgs e)
        {
            DoActionOnLayers("MouseUp", layer => layer.HandleMouseUp(sender, e));
            OnMouseUp(e);
        }

        private void PyxisView_MouseWheel(object sender, MouseEventArgs e)
        {
            DoActionOnLayers("MouseWheel", layer => layer.HandleMouseWheel(sender, e));
            OnMouseWheel(e);
        }

        private void PyxisView_MouseClick(object sender, MouseEventArgs e)
        {
            DoActionOnLayers("MouseClick", layer => layer.HandleMouseClick(sender, e));
            OnMouseClick(e);
        }

        private void PyxisView_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            DoActionOnLayers("MouseDoubleClick", layer => layer.HandleMouseDoubleClick(sender, e));
            OnMouseDoubleClick(e);
        }

        private void PyxisView_KeyDown(object sender, KeyEventArgs e)
        {
            DoActionOnLayers("KeyDown", layer => layer.HandleKeyDown(sender, e));
            OnKeyDown(e);
        }

        private void PyxisView_KeyPress(object sender, KeyPressEventArgs e)
        {
            DoActionOnLayers("KeyPress", layer => layer.HandleKeyPress(sender, e));
            OnKeyPress(e);
        }

        private void PyxisView_KeyUp(object sender, KeyEventArgs e)
        {
            DoActionOnLayers("KeyUp", layer => layer.HandleKeyUp(sender, e));
            OnKeyUp(e);
        }

        /// <summary>
        /// Process command keys that are not normally picked up by standard
        /// key press handlers
        /// </summary>
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            var keyEvent = new KeyEventArgs(keyData);

            switch (keyEvent.KeyCode)
            {
                case Keys.Left:
                case Keys.Right:
                case Keys.Up:
                case Keys.Down:
                case Keys.PageUp:
                case Keys.PageDown:
                case Keys.Home:
                case Keys.End:
                case Keys.Tab:
                    PyxisView_KeyDown(this, keyEvent);
                    PyxisView_KeyUp(this, keyEvent);
                    return true;
            }
            
            return base.ProcessCmdKey(ref msg, keyData);
        }

        private void ForEachLayer(string actionName, Action<ILayer> action)
        {
            if (Layers == null) return;

            foreach (var layer in Layers)
            {
                try
                {
                    action(layer);
                }
                catch (Exception ex)
                {
                    Trace.error("Error while " + actionName + " layer '" + layer.Name + "': " + ex.Message);
                }
            }
        }

        private void PyxisView_Resize(object sender, EventArgs e)
        {
            ForEachLayer("resizing", layer => layer.HandleResize(Width, Height));
            Invalidate();
        }

        private void DisposeLayers()
        {
            ForEachLayer("disposing", layer => layer.HandleDispose());
            Layers.Clear();
        }

        private void OpenGLContext_Paint(object sender, PaintEventArgs e)
        {
            if (m_drawingRequest > 0)
            {
                m_drawingRequest--;
            }

            var reportMaker = new RenderingTimeReportMaker();
            reportMaker.Start();

            InvokePaintActions(m_beforePaintStartedActions);
            reportMaker.Tick("before actions");
            
            ForEachLayer("painting", layer =>
            {
                if (layer.Visible)
                {
                    layer.Paint();
                }
                if (layer is ILayerWithTiming)
                {
                    reportMaker.Tick(layer.Name,
                        (layer as ILayerWithTiming).GetLastFrameRenderingTimeReport().Details);
                }
                else
                {
                    reportMaker.Tick(layer.Name);
                }
            });

            InvokePaintActions(m_afterPaintCompletedActions);
            reportMaker.Tick("after actions");

            RenderingReport = reportMaker.Finish();
        }

        private void PyxisView_Paint(object sender, PaintEventArgs e)
        {
            //design time paint.
            const string message = "PyxisView [Design Time Display]";
            var size = e.Graphics.MeasureString(message,Font);
            e.Graphics.DrawString(message, Font, Brushes.Black, Width / 2f - size.Width / 2, Height / 2f - size.Height / 2);
        }

        private void frameTimer_Tick(object sender, EventArgs e)
        {
            RequestFrame();
        }

        private void RequestFrame()
        {
            if (m_openGlControl != null)
            {
                if (m_drawingRequest == 0)
                {
                    m_drawingRequest++;
                    m_openGlControl.Invalidate();
                }
            }
        }

        private readonly object m_actionsLock = new object();
        private readonly List<Action> m_afterPaintCompletedActions = new List<Action>();
        private readonly List<Action> m_beforePaintStartedActions = new List<Action>();

        private void InvokePaintActions(List<Action> actions)
        {
            List<Action> savedActions = new List<Action>();
            lock (m_actionsLock)
            {
                savedActions.AddRange(actions);
                actions.Clear();
            }

            savedActions.ForEach(action => action());
        }

        /// <summary>
        /// Invoked an action when paint has been completed.
        /// </summary>
        internal void InvokeAfterPaintCompleted(Action action)
        {
            lock (m_actionsLock)
            {
                m_afterPaintCompletedActions.Add(action);
            }
        }

        /// <summary>
        /// Invoked an action before paint started.
        /// </summary>
        internal void InvokeBeforePaintStarted(Action action)
        {
            lock (m_actionsLock)
            {
                m_beforePaintStartedActions.Add(action);
            }
        }

        /// <summary>
        /// Perform an animation for a specified duration.
        /// </summary>
        /// <param name="animation">The animation.</param>
        /// <param name="duration">The duration of the animation.</param>
        /// <returns>The animation.</returns>
        public Animation Animate(Action<double> animation, TimeSpan duration)
        {
            return new Animation(this, animation, duration);            
        }

        /// <summary>
        /// Save the current Pyxis.UI.PyxisView to a System.Drawing.Bitmap.
        /// </summary>
        /// <returns>The System.Drawing.Bitmap of the current Pyxis.UI.PyxisView.</returns>
        public Bitmap SaveToBitmap()
        {
            Bitmap bitmap = null;
            this.InvokeIfRequired(() =>
            {
                bitmap = new Bitmap(Width, Height);
                var bitmapData = bitmap.LockBits(new Rectangle(Point.Empty, bitmap.Size), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
                Gl.glReadPixels(0, 0, Width, Height, Gl.GL_BGRA, Gl.GL_UNSIGNED_BYTE, bitmapData.Scan0);
                Gl.glFinish();
                bitmap.UnlockBits(bitmapData);
                bitmap.RotateFlip(RotateFlipType.RotateNoneFlipY);
            });
            return bitmap;
        }


        private class RenderingTimeReportMaker
        {
            private RenderingTimeReport Report { get; set;}

            private Stopwatch m_watch;
            private long m_lastTick;
            private  double m_millisecondsPerTick;

            public void Start()
            {
                Report = new RenderingTimeReport()
                            {
                                Name = "Frame",
                                Details = new List<RenderingTimeReport>()
                            };

                m_watch = new Stopwatch();
                m_watch.Start();
                var ticksPerSeconds = Stopwatch.Frequency;
                m_millisecondsPerTick = 1000.0 / ticksPerSeconds;
            }

            public void Tick(string name, List<RenderingTimeReport> details = null)
            {
                var newTick = m_watch.ElapsedTicks;
                var timeInMilliseconds = (newTick - m_lastTick) * m_millisecondsPerTick;
                m_lastTick = newTick;                

                Report.Details.Add(
                    new RenderingTimeReport()
                    {
                        Name = name,
                        RenderTimeInMilliseconds = timeInMilliseconds,
                        Details = details
                    });
            }

            public RenderingTimeReport Finish()
            {
                Report.RenderTimeInMilliseconds = m_watch.ElapsedTicks * m_millisecondsPerTick;
                return Report;
            }
        }
    }
}
