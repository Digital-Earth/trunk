using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Encapsulates functionality related to the appearance of the globe what GeoSources are displayed on the globe and the styling of the GeoSources.
    /// The view enforces that only one request is processed at any instant before it is ready to process another request.
    /// </summary>
    /// <seealso cref="Pyxis.UI.Layers.Globe.StyledGeoSource"/>
    public class ViewState : IDisposable
    {
        private readonly List<StyledGeoSource> m_styledGeoSources = new List<StyledGeoSource>();

        private readonly object m_viewPointProcesslockObject = new object();
        private IProcess_SPtr m_currentViewPointProcess;
        private IProcess_SPtr m_newCandidateViewPointProcess;
        
        private readonly Engine m_engine;

        private readonly object m_lockObject = new object();

        /// <summary>
        /// Occurs when a new view of the globe is ready.  
        /// This includes when new data is added/removed or when new styles are applied.
        /// </summary>
        public event EventHandler<EventArgs> OnNewViewPointProcessReady;
        private readonly TaskScheduler m_originalScheduler;


        private bool m_isReady;
        /// <summary>
        /// Determine if the view is ready to accept new requests.
        /// </summary>
        public bool IsReady { get { return m_isReady; } }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Globe.ViewState class.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use for processing.</param>
        public ViewState(Engine engine)
        {
            m_engine = engine;
            
            m_originalScheduler = TaskScheduler.FromCurrentSynchronizationContext();

            m_currentViewPointProcess = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.ViewPointProcess);
            m_currentViewPointProcess.initProc();

            m_isReady = true;
        }

        /// <summary>
        /// Show a new Pyxis.UI.Layers.Globe.StyledGeoSource on the globe corresponding to a Pyxis.Contract.Publishing.GeoSource.
        /// </summary>
        /// <param name="geoSource">Pyxis.Contract.Publishing.GeoSource to be shown.</param>
        /// <returns>The identifier of <paramref name="geoSource"/>.</returns>
        public Guid Show(GeoSource geoSource) {
            lock (m_lockObject)
            {
                using (new IsReadyKeeper(this))
                {
                    var styledGeoSource = StyledGeoSource.Create(m_engine, geoSource);
                    m_styledGeoSources.Add(styledGeoSource);
                    GenerateNewViewPoint();
                    return styledGeoSource.Id;
                }
            }
        }

        /// <summary>
        /// Show a new Pyxis.UI.Layers.Globe.StyledGeoSource on the globe corresponding to a Pyxis.Contract.Publishing.GeoSource.
        /// </summary>
        /// <param name="geoSource">Pyxis.Contract.Publishing.GeoSource to be shown.</param>
        /// <param name="style">StyleInformation to style the GeoSource</param>
        /// <returns>The identifier of <paramref name="geoSource"/>.</returns>
        public Guid Show(GeoSource geoSource,Style style)
        {
            lock (m_lockObject)
            {
                using (new IsReadyKeeper(this))
                {
                    var styledGeoSource = StyledGeoSource.Create(m_engine, geoSource, style);
                    m_styledGeoSources.Add(styledGeoSource);
                    GenerateNewViewPoint();
                    return styledGeoSource.Id;
                }
            }
        }

        /// <summary>
        /// Create a default style for a GeoSource.
        /// </summary>
        /// <param name="geoSource">Pyxis.Contract.Publishing.GeoSource to be styled.</param>
        /// <param name="style">StyleInformation that should guide default style of the GeoSource</param>
        /// <returns>new Style that can be used to visualize the GeoSource</returns>
        public Style CreateDefaultStyle(GeoSource geoSource, Style style)
        {
            var styledGeoSource = StyledGeoSource.Create(m_engine, geoSource);
            return styledGeoSource.CreateDefaultStyle(style);              
        }
        
        /// <summary>
        /// Get the identifier of a Pyxis.UI.Layers.Globe.StyledGeoSource given a Pyxis.Contract.Publishing.GeoSource.
        /// </summary>
        /// <param name="geoSource">The Pyxis.Contract.Publishing.GeoSource to find the styled version of.</param>
        /// <returns>The identifier of the Pyxis.UI.Layers.Globe.StyledGeoSource corresponding to <paramref name="geoSource"/> if one exists; otherwise null.</returns>
        public Guid? GetStyledGeoSource(GeoSource geoSource)
        {
            lock (m_lockObject)
            {
                var styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.GeoSource.Id == geoSource.Id);
                if (styledGeoSource != null)
                {
                    return styledGeoSource.Id;
                }
                return null;
            }
        }

        /// <summary>
        /// Get all the identifiers of all the Pyxis.UI.Layers.Globe.StyledGeoSource in the view.
        /// </summary>
        /// <returns>The identifiers of all the Pyxis.UI.Layers.Globe.StyledGeoSource in the view</returns>
        public List<Guid> GetStyledGeoSourcesIds()
        {
            lock (m_lockObject)
            {
                return m_styledGeoSources.Select(x => x.Id).ToList();
            }
        }

        /// <summary>
        /// Get all the identifiers of all the Pyxis.Contract.Publishing.GeoSource in the view.
        /// </summary>
        /// <returns>The identifiers of all the Pyxis.Contract.Publishing.GeoSource in the view</returns>
        public List<GeoSource> GetGeoSources()
        {
            lock (m_lockObject)
            {
                return m_styledGeoSources.Select(x => x.GeoSource).ToList();
            }
        }

        /// <summary>
        /// Hide a Pyxis.UI.Layers.Globe.StyledGeoSource from the view.
        /// </summary>
        /// <param name="id">The identifier of the Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        public void Hide(Guid id)
        {
            lock (m_lockObject)
            {
                using (new IsReadyKeeper(this))
                {
                    var styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);
                    if (styledGeoSource != null)
                    {
                        m_styledGeoSources.Remove(styledGeoSource);
                        GenerateNewViewPoint();
                    }
                }
            }
        }

        /// <summary>
        /// Get Pyxis.UI.Layers.Globe.StyleInformation about a Pyxis.UI.Layers.Globe.StyledGeoSource in the view.
        /// </summary>
        /// <param name="id">The identifier of the Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <returns>Pyxis.UI.Layers.Globe.StyleInformation about the Pyxis.UI.Layers.Globe.StyledGeoSource identified by <paramref name="id"/> if it is in the view; otherwise null.</returns>
        public Style GetStyle(Guid id)
        {
            lock (m_lockObject)
            {
                var styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);
                if (styledGeoSource != null)
                {
                    return styledGeoSource.Style;
                }
                return null;
            }
        }

        /// <summary>
        /// Get the ProcRef for a Pyxis.UI.Layers.Globe.StyledGeoSource in the view.
        /// Note: this method expose PYXLIB class ProcRef, therefore it have to be internal.
        /// </summary>
        /// <param name="id">The identifier of the Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <returns>ProcRef about the Pyxis.UI.Layers.Globe.StyledGeoSource identified by <paramref name="id"/> if it is in the view; otherwise null.</returns>
        internal ProcRef GetProcRef(Guid id)
        {
            lock (m_lockObject)
            {
                var styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);
                if (styledGeoSource != null)
                {
                    return styledGeoSource.ProcRef;
                }
                return null;
            }
        }

        /// <summary>
        /// Get the Pyxis.UI.Layers.Globe.StyledGeoSource for a process defined by a ProcRef in the view.
        /// Note: this method expose PYXLIB class ProcRef, therefore it have to be internal.
        /// </summary>
        /// <param name="procRef">The ProcRef to identify the Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <returns>Pyxis.UI.Layers.Globe.StyledGeoSource identified by <paramref name="procRef"/> if it is in the view; otherwise null.</returns>
        internal StyledGeoSource GetByProcRef(ProcRef procRef)
        {
            var procRefString = pyxlib.procRefToStr(procRef);
            lock (m_lockObject)
            {
                return m_styledGeoSources.FirstOrDefault(styled => styled.StyledPipeline != null && styled.StyledPipeline.ProcRef == procRefString 
                    || styled.GeoSource != null && styled.GeoSource.ProcRef == procRefString);
            }
        }

        /// <summary>
        /// Apply a style to a Pyxis.UI.Layers.Globe.StyledGeoSource in the view.
        /// </summary>
        /// <param name="id">Identifier of a Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <param name="style">The desired styling of the Pyxis.UI.Layers.Globe.StyledGeoSource identified by <paramref name="id"/>.</param>
        public void ApplyStyle(Guid id, Style style)
        {
            lock (m_lockObject)
            {
                using (new IsReadyKeeper(this))
                {
                    var styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);
                    if (styledGeoSource != null)
                    {
                        styledGeoSource.ApplyStyle(style);
                        GenerateNewViewPoint();
                    }
                }
            }
        }

        /// <summary>
        /// Set the style of a Pyxis.UI.Layers.Globe.StyledGeoSource using one of its fields.
        /// </summary>
        /// <param name="id">Identifier of a Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <param name="fieldName">The field to use for styling.</param>
        /// <returns>Pyxis.UI.Layers.Globe.StyledInformation about the generated styling.</returns>
        public Style SetStyleByField(Guid id, string fieldName)
        {
            return SetStyleByField(id, fieldName, PaletteExtensions.Default);
        }

        /// <summary>
        /// Set the style of a Pyxis.UI.Layers.Globe.StyledGeoSource using one of its fields and a Pyxis.UI.Layers.Globe.Palette.
        /// </summary>
        /// <param name="id">Identifier of a Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <param name="fieldName">The field to use for styling.</param>
        /// <param name="palette">The Pyxis.UI.Layers.Globe.Palette to use for the styling.</param>
        /// <returns>
        /// Pyxis.UI.Layers.Globe.StyledInformation about the generated styling.
        /// Returns null if no StyledGeoSource found with a matching id or failed to create Style with given parameters.
        /// </returns>
        public Style SetStyleByField(Guid id, string fieldName, Palette palette)
        {
            using (new IsReadyKeeper(this))
            {
                StyledGeoSource styledGeoSource;

                lock (m_lockObject)
                {

                    styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);
                    if (styledGeoSource == null)
                    {
                        return null;
                    }
                }

                var newStyle = styledGeoSource.CreateStyleByField(fieldName, palette);

                if (newStyle == null)
                {
                    return null;
                }

                lock (m_lockObject)
                {
                    styledGeoSource.ApplyStyle(newStyle);
                    GenerateNewViewPoint();
                    return newStyle;
                }
            }            
        }

        /// <summary>
        /// Set the style of a Pyxis.UI.Layers.Globe.StyledGeoSource using one of its fields.
        /// The style created based on a given Pyxis.UI.Layers.Globe.Palette and a given IGeometry.
        /// </summary>
        /// <param name="id">Identifier of a Pyxis.UI.Layers.Globe.StyledGeoSource.</param>
        /// <param name="fieldName">The field to use for styling.</param>
        /// <param name="palette">The Pyxis.UI.Layers.Globe.Palette to use for the styling.</param>
        /// <param name="geometry">The Geometry to be used to sample values of the GeoSource.</param>
        /// <returns>
        /// Pyxis.UI.Layers.Globe.StyledInformation about the generated styling.
        /// Returns null if no StyledGeoSource found with a matching id or failed to create Style with given parameters.
        /// </returns>
        public Style SetStyleByFieldBasedOnGeometry(Guid id, string fieldName, Palette palette, IGeometry geometry)
        {
            using (new IsReadyKeeper(this))
            {
                StyledGeoSource styledGeoSource;

                lock (m_lockObject)
                {
                    styledGeoSource = m_styledGeoSources.FirstOrDefault(styled => styled.Id == id);

                    if (styledGeoSource == null)
                    {
                        return null;
                    }
                }

                var newStyle = styledGeoSource.CreateStyleByField(fieldName, palette, geometry);
                if (newStyle == null)
                {
                    return null;
                }

                lock (m_lockObject)
                {
                    styledGeoSource.ApplyStyle(newStyle);
                    GenerateNewViewPoint();
                    return newStyle;
                }
            }
        }


        /// <summary>
        /// Get a PYXIS process pointer to the viewpoint process underlying the view.
        /// </summary>
        /// <returns>A PYXIS process pointer to the underlying viewpoint process.</returns>
        public IProcess_SPtr GetViewPointProcess()
        {
            lock (m_viewPointProcesslockObject)
            {
                return m_currentViewPointProcess;
            }
        }

        private void GenerateNewViewPoint()
        {
            bool generateGeometry = false;
            IProcess_SPtr newViewPointProcess;

            lock (m_lockObject)
            {
                newViewPointProcess = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.ViewPointProcess);

                foreach (var styledGeoSource in m_styledGeoSources)
                {
                    styledGeoSource.AddToViewPoint(newViewPointProcess);
                }

                //if view state have more 2 or more pipelines styled as elevation, it will requie some processing
                //on the viewPointProcess to calcualte the union (area) of the those pipelines.
                //That union will require some processing time on the main thread (ui-freeze) if not pre-calculated.
                if (m_styledGeoSources.Where((styledGeoSource) => styledGeoSource.Style.ShowAsElevation == true).Count() > 1)
                {
                    generateGeometry = true;
                }

                lock (m_viewPointProcesslockObject)
                {
                    m_newCandidateViewPointProcess = newViewPointProcess;
                }
            }

            Task.Factory.StartNew(() => InitializeNewViewPointProcess(newViewPointProcess,generateGeometry));            
        }

        private void InitializeNewViewPointProcess(IProcess_SPtr newViewPointProcess,bool generateGeometry)
        {
            if (newViewPointProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                newViewPointProcess.Dispose();
                return;
            }

            if (generateGeometry)
            {
                //do it here in background thread, seting the new view point process will be on the main thread and freeze the UI
                var outputProcess = pyxlib.QueryInterface_IProcess(newViewPointProcess.getOutput());
                var blender = outputProcess.getParameter(0).getValue(1);
                var feature = pyxlib.QueryInterface_IFeature(blender.getOutput());
                var geometry = feature.getGeometry();
            }

            lock (m_lockObject)
            {
                lock (m_viewPointProcesslockObject)
                {
                    //we got a new view point process while we did this
                    if (m_newCandidateViewPointProcess != newViewPointProcess)
                    {
                        //we have nothing else to do
                        return;
                    }

                    m_currentViewPointProcess = newViewPointProcess;
                }
            }            

            //invoke event in the original thread
            Task.Factory.StartNew(() =>
            {
                var handler = OnNewViewPointProcessReady;
                if (handler != null)
                {
                    handler.Invoke(this, new EventArgs());
                }
            }, CancellationToken.None, TaskCreationOptions.None, m_originalScheduler);
        }

        private bool m_disposed = false;

        /// <summary>
        /// Dispose of all the resources used by the view.
        /// </summary>
        public void Dispose()
        {
            if (!m_disposed)
            {
                foreach (var styledGeoSource in m_styledGeoSources)
                {
                    styledGeoSource.Dispose();
                }
                m_styledGeoSources.Clear();

                if (m_newCandidateViewPointProcess != null)
                {
                    m_newCandidateViewPointProcess.Dispose();
                }
                m_newCandidateViewPointProcess = null;

                if (m_currentViewPointProcess != null)
                {
                    m_currentViewPointProcess.Dispose();
                }
                m_currentViewPointProcess = null;
            }
            m_disposed = true;
        }

        //helper class to keep track of m_isReady value
        private class IsReadyKeeper : IDisposable
        {
            private readonly ViewState m_state;

            public IsReadyKeeper(ViewState state)
            {
                m_state = state;
                m_state.m_isReady = false;
            }

            public void Dispose()
            {
                m_state.m_isReady = true;
            }
        }
    }
}
