using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Publishing;
using Pyxis.UI.Layers.Globe;
using Tao.Platform.Windows;

namespace Pyxis.UI.Layers
{
    /// <summary>
    /// A layer containing an interactive globe.
    /// </summary>
    public class GlobeLayer : BaseLayer, IDisposable, ILayerWithTiming
    {
        private readonly Engine m_engine;
        private readonly PyxisView m_pyxisView;

        private readonly IntPtr m_hglrc;

        internal View View { get; private set; }

        private bool m_needToUpdateView;

        private IProcess_SPtr m_viewPointProcess;
        internal IProcess_SPtr ViewPointProcess
        {
            get
            {
                return m_viewPointProcess;
            }
            private set
            {
                m_viewPointProcess = value;
                m_needToUpdateView = true;                
            }
        }

        /// <summary>
        /// Keep the performance of rendering of the last frame
        /// </summary>
        private RenderingTimeReport m_report;

        /// <summary>
        /// Gets the current Pyxis.UI.Layers.Globe.ViewState of the layer.
        /// </summary>
        public ViewState ViewState { get; private set; }

        /// <summary>
        /// Occurs when the Pyxis.UI.Layers.GlobeLayer is clicked by a mouse.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseClick;
        /// <summary>
        /// Occurs when the Pyxis.UI.Layers.GlobeLayer is double-clicked by a mouse.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseDoubleClick;
        /// <summary>
        /// Occurs when the mouse pointer moves while over the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseMove;
        /// <summary>
        /// Occurs when any mouse button is pressed while the pointer is over the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseDown;
        /// <summary>
        /// Occurs when any mouse button is released over the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseUp;
        /// <summary>
        /// Occurs when the user rotates the mouse wheel while the mouse pointer is over the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseWheel;

        /// <summary>
        /// Occurs when the mouse pointer moves to a new cell while over the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GeographicMouseEventArgs> GlobeMouseCellChanged;

        /// <summary>
        /// Occurs when the Pyxis.Contract.Publishing.Camera of the Pyxis.UI.Layers.GlobeLayer changes.
        /// </summary>
        public event EventHandler<CameraChangeEventArgs> CameraChanged;

        /// <summary>
        /// Occurs when the mouse pointer enters an annotation on the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GlobeAnnotationMouseEventArgs> AnnotationMouseEnter;
        /// <summary>
        /// Occurs when the mouse pointer leaves an annotation on the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GlobeAnnotationMouseEventArgs> AnnotationMouseLeave;
        /// <summary>
        /// Occurs when the mouse pointer moves while over an annotation on the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GlobeAnnotationMouseEventArgs> AnnotationMouseMove;
        /// <summary>
        /// Occurs when the mouse pointer clicks over an annotation on the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GlobeAnnotationMouseEventArgs> AnnotationMouseClick;
        /// <summary>
        /// Occurs when the mouse pointer double clicks over an annotation on the Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        public event EventHandler<GlobeAnnotationMouseEventArgs> AnnotationMouseDoubleClick;

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.GlobeLayer class.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to embed the globe in.</param>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        public GlobeLayer(PyxisView pyxisView, Engine engine)
            : base("Globe")
        {
            m_engine = engine;
            m_pyxisView = pyxisView;

            //register resource in assembly for view to load (demo_icons,pyxis_grid etc)
            ManagedBitmapServer.Instance.RegisterResourcesInAssembly(Assembly.GetAssembly(typeof(GlobeLayer)));

            m_hglrc = Wgl.wglGetCurrentContext();
            View = new View(ViewTheme.knViewEmbedded);
            View.reshape(pyxisView.Width, pyxisView.Height);

            ViewState = new ViewState(engine);

            ViewState.OnNewViewPointProcessReady += (s, e) =>
            {
                ViewPointProcess = ViewState.GetViewPointProcess();
            };

            ViewPointProcess = ViewState.GetViewPointProcess();

            View.getAnnotationMouseEnterNotifier().Event += ViewAnnotation_MouseEnterEvent;
            View.getAnnotationMouseLeaveNotifier().Event += ViewAnnotation_MouseLeaveEvent;
            View.getAnnotationMouseMoveNotifier().Event += ViewAnnotation_MouseMoveEvent;
            View.getAnnotationClickNotifier().Event += ViewAnnotation_MouseClickEvent;
            View.getAnnotationDoubleClickNotifier().Event += ViewAnnotation_MouseDoubleClickEvent;

            SetCamera(CameraExtensions.Default, TimeSpan.Zero);

            engine.BeforeStopping(SafeDisposeView);
        }

        private void SafeDisposeView()
        {
            if (m_pyxisView != null && !m_pyxisView.IsDisposed)
            {
                if (m_pyxisView.InvokeRequired)
                {
                    m_pyxisView.Invoke((MethodInvoker)DisposeView);
                }
                else
                {
                    DisposeView();
                }
            }
        }

        private void DisposeView()
        {
            if (View != null)
            {
                ViewState.Dispose();

                View.getAnnotationMouseEnterNotifier().Event -= ViewAnnotation_MouseEnterEvent;
                View.getAnnotationMouseLeaveNotifier().Event -= ViewAnnotation_MouseLeaveEvent;
                View.getAnnotationMouseMoveNotifier().Event -= ViewAnnotation_MouseMoveEvent;
                View.getAnnotationClickNotifier().Event -= ViewAnnotation_MouseClickEvent;
                View.getAnnotationDoubleClickNotifier().Event -= ViewAnnotation_MouseDoubleClickEvent;

                Wgl.wglMakeCurrent(Wgl.wglGetCurrentDC(), m_hglrc);
                View.dispose();
                View = null;
                View.closeAllResources();
                Wgl.wglMakeCurrent(Wgl.wglGetCurrentDC(), IntPtr.Zero);

                m_viewPointProcess = null;
            }
        }

        /// <summary>
        /// Called when layer is about to be disposed.
        /// </summary>
        public override void HandleDispose()
        {
            DisposeView();
            base.HandleDispose();
        }

        /// <summary>
        /// Dispose all the resources used by the layer.
        /// </summary>
        public void Dispose()
        {
            DisposeView();
        }

        /// <summary>
        /// Draw the layer - called every frame.
        /// </summary>
        public override void Paint()
        {
            if (View != null)
            {
                if (m_needToUpdateView)
                {
                    m_needToUpdateView = false;
                    View.setViewPointProcess(m_viewPointProcess);
                }

                View.display();
                InvokeLoadingCompletedIfNeeded();
                InvokeCameraChangeIfNeeded();
                InvokeMouseCellChangeIfNeeded();

                m_report = new RenderingTimeReport()
                {
                    Name = "Globe",
                    RenderTimeInMilliseconds = 0,
                    Details = new List<RenderingTimeReport>() {
                        GetRenderTimeDetails("setup-camera"),                        
                        GetRenderTimeDetails("setup-terrain-refine"),
                        GetRenderTimeDetails("setup-terrain-update"),
                        GetRenderTimeDetails("setup-processing"),
                        GetRenderTimeDetails("render-terrain"),
                        GetRenderTimeDetails("setup-icons"),                
                        GetRenderTimeDetails("render-icons"),                                                                                               
                        GetRenderTimeDetails("mouse-pick"),
                        GetRenderTimeDetails("render-end")
                    }
                };
            }
        }

        private RenderingTimeReport GetRenderTimeDetails(string name) {
            return new RenderingTimeReport()
            {
                Name = name,
                RenderTimeInMilliseconds = View.getRenderTime(name)
            };
        }

        /// <summary>
        /// Handle resize event.
        /// </summary>
        /// <param name="width">The new width.</param>
        /// <param name="height">The new height.</param>
        public override void HandleResize(int width, int height)
        {
            if (View != null)
            {
                View.reshape(width, height);
            }
        }

        /// <summary>
        /// Invoke the handler of a mouse event over the globe.
        /// </summary>
        /// <param name="handler">The globe mouse event handler.</param>
        /// <param name="eventArgs">A MouseEventArgs that contains the event data.</param>
        protected void InvokeGlobeMouseEvent(EventHandler<GeographicMouseEventArgs> handler, MouseEventArgs eventArgs)
        {
            if (handler != null)
            {
                var index = View.getPointerIndex(View.getViewDataResolution());
                if (!index.isNull())
                {
                    handler.Invoke(this, new GeographicMouseEventArgs(eventArgs, index.toString()));
                }
            }
        }

        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseDown(object sender, MouseEventArgs e)
        {
            if (View != null)
            {
                View.onMouseDown(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                      , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseDown, e);
            }
            return true;
        }

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseMove(object sender, MouseEventArgs e)
        {
            m_lastMouseMoveState = e;
            if (View != null)
            {
                View.onMouseMove(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                  , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseMove, e);
            }
            return true;
        }

        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseUp(object sender, MouseEventArgs e)
        {
            if (View != null)
            {
                View.onMouseUp(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                  , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseUp, e);
            }
            return true;
        }

        /// <summary>
        /// Represents the method that will handle the MouseWheel event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseWheel(object sender, MouseEventArgs e)
        {
            if (View != null)
            {
                View.onMouseWheel(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                  , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseWheel, e);
            }
            return true;
        }

        /// <summary>
        /// Represents the method that will handle the MouseClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseClick(object sender, MouseEventArgs e)
        {
            if (View != null)
            {
                View.onMouseClick(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                  , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseClick, e);
            }
            return true;
        }

        /// <summary>
        /// Represents the method that will handle the MouseDoubleClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (View != null)
            {
                View.onMouseDoubleClick(e.X, e.Y, e.Delta, e.Button == MouseButtons.Left, e.Button == MouseButtons.Right, e.Button == MouseButtons.Middle
                                  , (Control.ModifierKeys & Keys.Alt) != 0, (Control.ModifierKeys & Keys.Shift) != 0, (Control.ModifierKeys & Keys.Control) != 0);

                InvokeGlobeMouseEvent(GlobeMouseDoubleClick, e);
            }
            return true;
        }

        /// <summary>
        /// Get a Pyxis.Contract.Publishing.Camera representing the current view of the globe.
        /// </summary>
        /// <returns>Pyxis.Contract.Publishing.Camera of the current globe view if the layer has not been disposed; otherwise null.</returns>
        public Contract.Publishing.Camera GetCamera()
        {
            if (View != null)
            {
                return View.getCamera().ToCamera();
            }
            return null;
        }

        /// <summary>
        /// Transition the current view of the globe to the specified camera position.
        /// </summary>
        /// <param name="camera">The desired camera position to view the globe from.</param>
        /// <param name="duration">The duration of the transition between the current viewing position and the desired position.</param>
        public void SetCamera(Contract.Publishing.Camera camera, TimeSpan duration)
        {
            if (View != null)
            {
                View.goToCamera(camera.ToPYXCamera(), (int)duration.TotalMilliseconds);
            }
        }

        /// <summary>
        /// Transition the current view of the globe to the specified Pyxis.Core.IO.GeoJson.GeographicPosition.
        /// </summary>
        /// <param name="position">The desired Pyxis.Core.IO.GeoJson.GeographicPosition to view on the globe.</param>
        /// <param name="duration">The duration of the transition between the current viewing position and the desired position.</param>
        public void GotoPoint(GeographicPosition position, TimeSpan duration)
        {
            if (View != null)
            {
                View.goToLatlon(position.ToPointLocation().asGeocentric(), (int)duration.TotalMilliseconds);
            }
        }

        /// <summary>
        /// Transition the current view of the globe to the specified PYXIS geometry.
        /// </summary>
        /// <param name="geometry">The desired PYXIS geometry to view on the globe.</param>
        /// <param name="duration">The duration of the transition between the current viewing position and the desired position.</param>
        public void GotoGeometry(IGeometry geometry, TimeSpan duration)
        {
            if (View != null)
            {
                View.goToGeometry(m_engine.ToPyxGeometry(geometry), (int)duration.TotalMilliseconds);
            }
        }

        /// <summary>
        /// Transition the current view of the globe to a view of the specified Pyxis.Contract.Publishing.Pipeline.
        /// </summary>
        /// <param name="pipeline">The desired Pyxis.Contract.Publishing.Pipeline to view on the globe.</param>
        /// <param name="duration">The duration of the transition between the current viewing position and the desired position.</param>
        public void GotoPipeline(Pipeline pipeline, TimeSpan duration)
        {
            if (View != null)
            {
                var process = m_engine.GetProcess(pipeline);
                var feature = pyxlib.QueryInterface_IFeature(process.getOutput());
                View.goToGeometry(feature.getGeometry(), (int)duration.TotalMilliseconds);
            }
        }

        /// <summary>
        /// Get Pyxis.Core.IO.GeoJson.GeographicPosition of points in the screen space of the layer.
        /// If the screen point is not over the globe, null is returned.
        /// </summary>
        /// <param name="points">List of two-dimensional points visible on the screen over the layer.</param>
        /// <returns>The Pyxis.Core.IO.GeoJson.GeographicPosition of the <paramref name="points"/> on the globe if the layer has not been disposed; otherwise null.</returns>
        public List<GeographicPosition> ScreenToGeographicPosition(List<int[]> points)
        {
            VerifyCallingThread();

            if (View == null)
            {
                return null;
            }

            //run the projection on the UI thread...
            return points.Select(x =>
                    {
                        var coord3D = View.projectFromScreenSpace(x[0], x[1]);
                        if (coord3D.length() > 0.1)
                        {
                            coord3D.normalize();
                            return new GeographicPosition(PointLocation.fromXYZ(coord3D));
                        }
                        return null;
                    }).ToList();            
        }
        
        private float[] SafeGeographicPositionToScreen(GeographicPosition point)
        {
            var screen = View.projectToScreenSpace(point.ToPointLocation().asXYZ());
            return new[] { (float)screen.x(), (float)screen.y() };
        }

        /// <summary>
        /// Get the two-dimensional point on the screen of the Pyxis.Core.IO.GeoJson.GeographicPosition on the globe.
        /// The effects of elevation are ignored, i.e. the globe surface is treated like the surface of a sphere.
        /// </summary>
        /// <param name="point">The Pyxis.Core.IO.GeoJson.GeographicPosition on the globe.</param>
        /// <returns>The two-dimensional point on the screen of the <paramref name="point"/> on the globe if the layer has not been disposed; otherwise null.</returns>
        public float[] GeographicPositionToScreen(GeographicPosition point)
        {
            VerifyCallingThread();

            if (View == null)
            {
                return null;
            }

            return SafeGeographicPositionToScreen(point);
        }

        /// <summary>
        /// Get the two-dimensional points on the screen of the Pyxis.Core.IO.GeoJson.GeographicPosition on the globe.
        /// </summary>
        /// <param name="points">The Pyxis.Core.IO.GeoJson.GeographicPosition on the globe.</param>
        /// <returns>The two-dimensional point on the screen of the <paramref name="points"/> on the globe if the layer has not been disposed; otherwise null.</returns>
        public List<float[]> GeographicPositionToScreen(List<GeographicPosition> points)
        {
            VerifyCallingThread();

            if (View == null)
            {
                return null;
            }

            return points.Select(SafeGeographicPositionToScreen).ToList();
        }

        /// <summary>
        /// Get the PYXIS index of where the cursor is pointing on the globe.
        /// </summary>
        /// <returns>PYXIS index of where the cursor is pointing to on the globe if the layer has not been disposed; otherwise null.</returns>
        public PYXIcosIndex GetCursorIndex()
        {
            if (View == null)
            {
                return null;
            }
            return View.getPointerIndex(View.getViewDataResolution());
        }

        /// <summary>
        /// Get the Pyxis.Core.IO.GeoJson.Specialized.CellGeometry of where the cursor is pointing on the globe.
        /// </summary>
        /// <returns>Pyxis.Core.IO.GeoJson.Specialized.CellGeometry of where the cursor is pointing on the globe if the layer has not been disposed; otherwise null.</returns>
        public CellGeometry GetCursorCell()
        {
            if (View == null)
            {
                return null;
            }
            var index = View.getPointerIndex(View.getViewDataResolution());
            if (index.isNull())
            {
                return null;
            }
            return new CellGeometry(index);
        }

        /// <summary>
        /// Create a PYXIS geometry representing the watershed flowing into a location specified by a PYXIS index.
        /// </summary>
        /// <param name="from">The PYXIS index representing the location of the watershed sink.</param>
        /// <returns>The PYXIS geometry representing the watershed flowing into <paramref name="from"/>if the layer has not been disposed; otherwise null.</returns>
        public PYXGeometry_SPtr CalculateWatershed(PYXIcosIndex from)
        {
            if (View == null)
            {
                return null;
            }
            return View.calculateWatershed(from);
        }

        /// <summary>
        /// Return a geometry on the globe that is visible
        /// </summary>
        /// <returns>Viewport Geometry on the globe</returns>
        public IGeometry GetScreenGeometry()
        {
            if (View == null)
            {
                return null;
            }
            return Geometry.FromPYXGeometry(View.getScreenGeometry());
        }

        /// <summary>
        /// throw an exception if the calling thread is not the ui thread.
        /// </summary>
        private void VerifyCallingThread()
        {
            if (m_pyxisView.InvokeRequired)
            {
                throw new InvalidOperationException("Control accessed from a thread other than the thread it was created on.");
            }
        }

        /// <summary>
        /// Gets if the view of the globe is loading.
        /// </summary>
        public bool IsLoading
        {
            get
            {
                if (View == null)
                {
                    return false;
                }
                return (!ViewState.IsReady) ||
                       (View.getStreamingProgress() != 100);
            }
        }

        /// <summary>
        /// Gets if a StyledGeoSource is loading
        /// </summary>
        /// <param name="styleGeoSourceId">Id of the StyleGeoSource</param>
        /// <returns>true if the Globe is loading this GeoSource</returns>
        public bool IsLoadingByVisibleId(Guid styleGeoSourceId)
        {
            //try to extract procRef from StyledGeoSource.Id
            var procRef = ViewState.GetProcRef(styleGeoSourceId);

            //if we failed to get procRef or View doesn't exists, it not loading
            if (procRef == null || View == null)
            {
                return false;
            }

            //try to get loading progress (-1 mean no progress available)
            var streamingProcess = View.getStreamingProgress(procRef);
            if (streamingProcess < 0)
            {
                return false;
            }

            //we are loading if the progress is not 100%
            return streamingProcess < 100;
        }

        private readonly object m_onLoadingCompletedLock = new object();

        private List<Action> m_onLoadingCompleted;

        /// <summary>
        /// Execute System.Action only once the view has completed loading.
        /// Actions are stored if the current globe view has completed loading and will execute after the next view is loaded.
        /// </summary>
        /// <param name="action">The System.Action to perform when loading is complete.</param>
        public void WhenLoadingCompleted(Action action)
        {
            lock (m_onLoadingCompletedLock)
            {
                if (m_onLoadingCompleted == null)
                {
                    m_onLoadingCompleted = new List<Action>();
                }
                m_onLoadingCompleted.Add(action);
            }
        }

        private void InvokeLoadingCompletedIfNeeded()
        {
            List<Action> callbacks = null;

            lock (m_onLoadingCompletedLock)
            {
                if (m_onLoadingCompleted != null && !IsLoading)
                {
                    callbacks = m_onLoadingCompleted;
                    m_onLoadingCompleted = null;
                }
            }

            if (callbacks != null)
            {
                m_pyxisView.InvokeAfterPaintCompleted(() =>
                {
                    callbacks.ForEach(callback => callback());
                });
            }
        }

        private Contract.Publishing.Camera m_oldCamera;

        private void InvokeCameraChangeIfNeeded()
        {
            var handler = CameraChanged;

            if (handler != null)
            {
                var newCamera = GetCamera();
                if (m_oldCamera != newCamera)
                {
                    handler.Invoke(this, new CameraChangeEventArgs(newCamera));
                }
                m_oldCamera = newCamera;
            }
        }

        private MouseEventArgs m_lastMouseMoveState;
        private string m_oldCell;

        private void InvokeMouseCellChangeIfNeeded()
        {
            var handler = GlobeMouseCellChanged;

            if (handler != null)
            {
                var newIndex = GetCursorIndex();

                var newCell = newIndex.isNull() ? null : newIndex.toString();

                if (m_oldCell != newCell && m_lastMouseMoveState != null)
                {
                    handler.Invoke(this, new GeographicMouseEventArgs(m_lastMouseMoveState, newCell));
                }
                m_oldCell = newCell;
            }
        }


        private GlobeAnnotationMouseEventArgs GenerateAnnotationMouseEventArgs(NotifierEvent_SPtr notifierEvent)
        {
            // Manually dereference smart pointer proxy and manually 
            // downcast underlying object.
            AnnotationMouseEvent eventData = AnnotationMouseEvent.dynamic_cast(notifierEvent.__deref__());

            var annotation = eventData.getAnnotation();
            var styledGeoSource = ViewState.GetByProcRef(annotation.getProcRef());

            if (styledGeoSource == null)
            {
                return null;
            }

            var feature = annotation.getFeature();
            var group = pyxlib.QueryInterface_IFeatureGroup(feature);

            IFeaturesIdsProvider featuresIdsProvider;

            if (group.isNull())
            {
                featuresIdsProvider = new ListFeaturesIdsProvider(new List<string> { feature.getID() });
            }
            else
            {
                featuresIdsProvider = new FeaturesGroupFeaturesIdProvider(group);
            }

            var mouseEvent = new MouseEventArgs(
                    (eventData.isLeftButtonDown() ? MouseButtons.Left : MouseButtons.None) |
                    (eventData.isRightButtonDown() ? MouseButtons.Right : MouseButtons.None) |
                    (eventData.isMiddleButtonDown() ? MouseButtons.Middle : MouseButtons.None),
                    0,
                    (int)eventData.getMouseX(), (int)eventData.getMouseY(),
                    eventData.getWheelDelta());

            return new GlobeAnnotationMouseEventArgs(mouseEvent, styledGeoSource, featuresIdsProvider);
        }

        private void InvokeAnnotationMouseEvent(NotifierEvent_SPtr notifierEvent, EventHandler<GlobeAnnotationMouseEventArgs> handler)
        {
            if (handler != null)
            {
                var eventArgs = GenerateAnnotationMouseEventArgs(notifierEvent);

                if (eventArgs != null)
                {
                    handler.Invoke(this, eventArgs);
                }
            }
        }

        private void ViewAnnotation_MouseMoveEvent(NotifierEvent_SPtr notifierEvent)
        {
            InvokeAnnotationMouseEvent(notifierEvent, AnnotationMouseMove);
        }

        private void ViewAnnotation_MouseLeaveEvent(NotifierEvent_SPtr notifierEvent)
        {
            InvokeAnnotationMouseEvent(notifierEvent, AnnotationMouseLeave);
        }

        private void ViewAnnotation_MouseEnterEvent(NotifierEvent_SPtr notifierEvent)
        {
            InvokeAnnotationMouseEvent(notifierEvent, AnnotationMouseEnter);
        }

        private void ViewAnnotation_MouseClickEvent(NotifierEvent_SPtr notifierEvent)
        {
            InvokeAnnotationMouseEvent(notifierEvent, AnnotationMouseClick);
        }

        private void ViewAnnotation_MouseDoubleClickEvent(NotifierEvent_SPtr notifierEvent)
        {
            InvokeAnnotationMouseEvent(notifierEvent, AnnotationMouseDoubleClick);
        }

        /// <summary>
        /// Get the rendering performance of the last frame.
        /// </summary>
        /// <returns>A RenderingTimeReport</returns>
        public RenderingTimeReport GetLastFrameRenderingTimeReport()
        {
            return m_report;
        }
    }
}
