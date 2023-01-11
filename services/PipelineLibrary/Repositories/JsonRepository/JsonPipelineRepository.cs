using Newtonsoft.Json;
using Pyxis.Services.PipelineLibrary.Domain;
using Pyxis.Utilities;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Repositories.JsonRepository
{
    public class JsonPipelineRepository : IPipelineRepository
    {
        private object m_geometryMutex = new object();

        private EventHelper<UnhandledExceptionEventArgs> m_onFatalError =
            new EventHelper<UnhandledExceptionEventArgs>();

        private string m_pathToGeometries = AppServices.getWorkingPath().ToString() +
             Path.DirectorySeparatorChar +
            "PYXLibrary" + Path.DirectorySeparatorChar +
            "Geometries";

        private string m_pathToPipelineLibrary = AppServices.getWorkingPath().ToString() +
             Path.DirectorySeparatorChar +
            "PYXLibrary" + Path.DirectorySeparatorChar + "PipelineLibrary.json";

        private EventHelper<PipelineEventArgs> m_PipelineAdded = new EventHelper<PipelineEventArgs>();

        private EventHelper<PipelineEventArgs> m_PipelineGeometryResolved =
            new EventHelper<PipelineEventArgs>();

        private EventHelper<PipelineEventArgs> m_PipelineHidden = new EventHelper<PipelineEventArgs>();
        private EventHelper<PipelineEventArgs> m_PipelineRemoved = new EventHelper<PipelineEventArgs>();
        private PipelineRepositoryDTO m_pipelines = new PipelineRepositoryDTO();

        private EventHelper<PipelineEventArgs> m_PipelineUnhidden =
            new EventHelper<PipelineEventArgs>();

        private object m_lockObject = new object();

        public event EventHandler<UnhandledExceptionEventArgs> OnFatalError
        {
            add
            {
                m_onFatalError.Add(value);
            }
            remove
            {
                m_onFatalError.Remove(value);
            }
        }

        public event EventHandler<PipelineEventArgs> PipelineAdded
        {
            add
            {
                m_PipelineAdded.Add(value);
            }
            remove
            {
                m_PipelineAdded.Remove(value);
            }
        }

        public event EventHandler<PipelineEventArgs> PipelineGeometryResolved
        {
            add
            {
                m_PipelineGeometryResolved.Add(value);
            }
            remove
            {
                m_PipelineGeometryResolved.Remove(value);
            }
        }

        public event EventHandler<PipelineEventArgs> PipelineHidden
        {
            add
            {
                m_PipelineHidden.Add(value);
            }
            remove
            {
                m_PipelineHidden.Remove(value);
            }
        }

        public event EventHandler<PipelineEventArgs> PipelineRemoved
        {
            add
            {
                m_PipelineRemoved.Add(value);
            }
            remove
            {
                m_PipelineRemoved.Remove(value);
            }
        }

        public event EventHandler<PipelineEventArgs> PipelineUnhidden
        {
            add
            {
                m_PipelineUnhidden.Add(value);
            }
            remove
            {
                m_PipelineUnhidden.Remove(value);
            }
        }

        public void Add(IProcess_SPtr process)
        {
            lock (m_lockObject)
            {
                var procRef = new ProcRef(process);

                JsonPipeline pipeline = m_pipelines.Get(procRef.ToString()) as JsonPipeline;

                if (pipeline == null)
                {
                    pipeline = new JsonPipeline
                    {
                        Name = process.getProcName(),
                        Description = process.getProcDescription(),
                        PipelineGuid = process.getProcID(),
                        Version = process.getProcVersion(),
                        IsPublished = false,

                    };

                    pipeline.SetIdentity(process);
                    pipeline.Definition = PipeManager.writeProcessToNewString(process);
                    pipeline.AddOutputType(process.getSpec().getOutputInterfaces());

                    m_pipelines.Add(procRef.ToString(), pipeline);

                    IFeature_SPtr feature = null;
                    if (process.getInitState() == IProcess.eInitStatus.knInitialized)
                    {
                        feature = pyxlib.QueryInterface_IFeature(pyxlib.QueryInterface_PYXCOM_IUnknown(process));
                    }

                    if (feature != null && feature.isNotNull())
                    {
                        var defition = feature.getDefinition();
                        if (defition.isNotNull())
                        {
                            int count = defition.getFieldCount();
                            for (int i = 0; i < count; i++)
                            {
                                pipeline.AddMetadata(defition.getFieldDefinition(i).getName(), feature.getFieldValue(i).getString());
                            }
                        }
                    }
                }
                else
                {
                    pipeline.Name = process.getProcName();
                    pipeline.Description = process.getProcDescription();
                    pipeline.SetIdentity(process);
                    pipeline.Definition = PipeManager.writeProcessToNewString(process);
                    pipeline.OutputTypes.Clear();
                    pipeline.AddOutputType(process.getSpec().getOutputInterfaces());
                }

                // Add children
                pipeline.Children = new List<string>();
                foreach (var parameter in process.getParameters())
                {
                    for (var valueIndex = 0; valueIndex < parameter.getValueCount(); ++valueIndex)
                    {
                        var childProcess = parameter.getValue(valueIndex);
                        if (childProcess.isNotNull())
                        {
                            var childProcRef = new ProcRef(childProcess);
                            var childPipeline = GetByProcRef(childProcRef);
                            if (childPipeline != null)
                            {
                                pipeline.Children.Add(childProcRef.ToString());
                            }
                            else
                            {
                                throw new Exception("Json pipeline repository: Pipeline's child does not exist !");
                            }
                        }
                    }
                }
            }
        }

        public void Initialize(Dictionary<string, string> configuration)
        {
            lock (m_lockObject)
            {
                string path;
                if (configuration.TryGetValue("JsonFilePath", out path))
                {
                    m_pathToPipelineLibrary = path;
                }

                if (File.Exists(m_pathToPipelineLibrary))
                {
                    m_pipelines = JsonConvert.DeserializeObject<PipelineRepositoryDTO>(File.ReadAllText(m_pathToPipelineLibrary));
                }
            }
        }

        public void CheckPoint()
        {
            lock (m_lockObject)
            {
                try
                {
                    File.WriteAllText(m_pathToPipelineLibrary + ".new", JsonConvert.SerializeObject(m_pipelines));
                    if (File.Exists(m_pathToPipelineLibrary + ".old"))
                    {
                        File.Delete(m_pathToPipelineLibrary + ".old");
                    }
                    if (File.Exists(m_pathToPipelineLibrary))
                    {
                        File.Move(m_pathToPipelineLibrary, m_pathToPipelineLibrary + ".old");
                    }
                    File.Move(m_pathToPipelineLibrary + ".new", m_pathToPipelineLibrary);
                }
                catch (Exception e)
                {
                    InvokeFatalError(e);
                    throw e;
                }
            }
        }

        public bool Exists(ProcRef procRef)
        {
            lock (m_lockObject)
            {
                return m_pipelines.ContainsProcRef(procRef.ToString());
            }
        }

        public bool Exists(IProcess_SPtr process)
        {
            lock (m_lockObject)
            {
                var procRef = new ProcRef(process);
                return Exists(procRef);
            }
        }

        public bool Exists(string identity)
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Any(x => x.Identity.XmlIdentity == identity);
                return result;
            }
        }

        public IList<Pipeline> GetAllDisplayablePipelines()
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Where(x => !x.IsHidden && !x.IsTemporary).ToList();
                return result;
            }
        }

        public IList<Pipeline> GetAllDisplayablePipelines(string whereClause)
        {
            throw new NotImplementedException();
        }

        public IList<Pipeline> GetAllParentPipelines()
        {
            throw new NotImplementedException();
        }

        public IList<Pipeline> GetAllPipelines()
        {
            lock (m_lockObject)
            {
                return m_pipelines.Pipelines().ToList();
            }
        }

        public IList<Pipeline> GetAllPipelinesWithUnstableIdentity()
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Where(x => !x.IsHidden && !x.IsTemporary && !x.Identity.IsStable).ToList();
                return result;
            }
        }

        public IList<Pipeline> GetAllPublishedPipelines()
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Where(x => x.IsPublished && !x.IsTemporary).ToList();
                return result;
            }
        }

        public IList<Pipeline> GetByIdentity(string identity)
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Where(x => x.Identity.XmlIdentity == identity).ToList();
                return result;
            }
        }

        public IList<Pipeline> GetByOutputType(Guid outputType)
        {
            lock (m_lockObject)
            {
                var result = m_pipelines.Pipelines().Where(x => x.OutputTypes.Any(y => y.OutputType == outputType)).ToList();
                return result;
            }
        }

        public Pipeline GetByProcRef(ProcRef procRef)
        {
            lock (m_lockObject)
            {
                return m_pipelines.Get(procRef.ToString());
            }
        }

        public Pipeline GetByProcRef(Guid guid, int version)
        {
            return GetByProcRef(new ProcRef(pyxlib.strToGuid(guid.ToString()), version));
        }

        public PYXGeometry_SPtr GetGeometry(ProcRef procRef)
        {
            PYXGeometry_SPtr geometry = TryGetGeometry(procRef);

            if (geometry == null)
            {
                Pipeline pipeline = GetByProcRef(procRef);
                if (pipeline == null)
                {
                    return null;
                }

                IProcess_SPtr process = PipeManager.getProcess(
                    pipeline.ProcRef, true);
                if (process == null || process.get() == null)
                {
                    return null;
                }

                var resolver = new GeometryResolver(pipeline);
                resolver.PipelineGeometryResolved += OnPipelineGeometryResolved;
                resolver.Resolve(this, m_pathToGeometries, process);

                geometry = TryGetGeometry(procRef);
            }

            return geometry;
        }

        public IList<ProcRef> GetNotDownloadedPipelines()
        {
            lock (m_lockObject)
            {
                var definitions = new List<ProcRef>();
                definitions.AddRange(m_pipelines.Pipelines().Where(x => x.IsImported && !x.IsDownloaded && !x.IsTemporary).Select(x => x.ProcRef));
                return definitions;
            }
        }

        public IList<string> GetNotImportedPipelines()
        {
            lock (m_lockObject)
            {
                var definitions = new List<string>();
                definitions.AddRange(m_pipelines.Pipelines().Where(x => !x.IsImported && !x.IsTemporary).Select(x => x.Definition));
                return definitions;
            }
        }

        public IList<ProcRef> GetNotProcessedPipelines()
        {
            lock (m_lockObject)
            {
                return m_pipelines.Pipelines().Where(x => x.IsImported && x.IsDownloaded && x.IsPublished && !x.IsProcessed && !x.IsTemporary).Select(x => x.ProcRef).ToList();
            }
        }

        public IList<ProcRef> GetNotPublishedPipelines()
        {
            lock (m_lockObject)
            {
                return m_pipelines.Pipelines().Where(x => x.IsImported && x.IsDownloaded && !x.IsPublished && !x.IsTemporary).Select(x => x.ProcRef).ToList();
            }
        }

        public void PersistGeometry(ProcRef procRef)
        {
            string pathToGeometryFile = m_pathToGeometries +
                Path.DirectorySeparatorChar +
               pyxlib.procRefToStr(procRef);

            lock (m_geometryMutex)
            {
                Pipeline pipeline = GetByProcRef(procRef);
                if (pipeline == null)
                {
                    // TODO[kabiraman]: We should probably throw an exception here.
                    return;
                }

                IProcess_SPtr process = PipeManager.getProcess(pipeline.ProcRef, true);
                if (process == null || process.get() == null)
                {
                    // TODO[kabiraman]: We should probably throw an exception here.
                    return;
                }

                PYXGeometry_SPtr geometry = new GeometryResolver(pipeline).SerializeGeometryToFile(pathToGeometryFile, process);
                if (geometry == null || geometry.get() == null)
                {
                    // TODO[kabiraman]: We should probably throw an exception here.
                }
            }
        }

        public void SetIsDownloaded(ProcRef procRef, bool isDownloaded)
        {
            SetPipelineProperty(procRef, "IsDownloaded", isDownloaded);
        }

        public void SetIsHidden(ProcRef procRef, bool value)
        {
            Pipeline pipeline = null;
            pipeline = GetByProcRef(procRef);

            if (pipeline != null)
            {
                pipeline.IsHidden = value;
            }
            else
            {
                // throw an exception here?
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Warning!  Unable to set {0}[{1}] as hidden.  Process not found.",
                    procRef.getProcID(), procRef.getProcVersion()));
            }

            if (pipeline != null)
            {
                if (value)
                {
                    OnPipelineHidden(pipeline);
                }
                else
                {
                    OnPipelineUnhidden(pipeline);

                    // TODO[kabiraman]: There needs to be another way to decide which
                    // pipeline to create the geometry for; IsHidden is planned to
                    // be removed from the table schema and we want to be
                    // careful not to generate geometries for non-root
                    // processes in a pipeline.
                    var resolver = new GeometryResolver(pipeline);
                    resolver.PipelineGeometryResolved += OnPipelineGeometryResolved;
                    resolver.Start(this, m_pathToGeometries);
                }
            }
        }

        public void SetIsHidden(IProcess_SPtr process, bool value)
        {
            SetIsHidden(new ProcRef(process), value);
        }

        public void SetIsImported(ProcRef procRef, bool isImported)
        {
            SetPipelineProperty(procRef, "IsImported", isImported);
        }

        public void SetIsProcessed(ProcRef procRef, bool isProcessed)
        {
            SetPipelineProperty(procRef, "IsProcessed", isProcessed);
        }

        public void SetProcessedResolution(ProcRef procRef, int MaxProcessedResolution)
        {
            SetPipelineProperty(procRef, "MaxProcessedResolution", MaxProcessedResolution);
        }

        public void SetIsPublished(IProcess_SPtr process, bool value)
        {
            SetIsPublished(new ProcRef(process), value);
        }

        public void SetIsPublished(ProcRef procRef, bool isPublished)
        {
            SetPipelineProperty(procRef, "IsPublished", isPublished);
        }

        public bool TryRemovePipeline(ProcRef procRef)
        {
            lock (m_lockObject)
            {
                var pipeline = GetByProcRef(procRef) as JsonPipeline;

                var isReferenced = m_pipelines.Pipelines().Any(x => ((JsonPipeline)x).Children.Contains(procRef.ToString()));

                if (pipeline != null && !isReferenced)
                {
                    m_pipelines.Remove(pipeline.ProcRef.ToString());

                    foreach (var child in pipeline.Children)
                    {
                        TryRemovePipeline(pyxlib.strToProcRef(child));
                    }
                    return true;
                }
                return false;
            }
        }

        public void SetIsTemporary(ProcRef procRef, bool isTemporary)
        {
            SetPipelineProperty(procRef, "IsTemporary", isTemporary);
        }

        private void SetPipelineProperty(ProcRef procRef, string propertyName, object value)
        {
            lock (m_lockObject)
            {
                var pipeline = GetByProcRef(procRef);

                if (pipeline != null)
                {
                    var propertyInfo = pipeline.GetType().GetProperty(propertyName);
                    if (propertyInfo != null)
                    {
                        propertyInfo.SetValue(pipeline, value, null);
                    }
                    else
                    {
                        System.Diagnostics.Trace.WriteLine("Unable to set " + propertyName + " no such propery found");
                    }
                }
                else
                {
                    System.Diagnostics.Trace.WriteLine("Unable to find " + procRef.ToString());
                }
            }
        }

        public void SetIsTemporary(IProcess_SPtr process, bool value)
        {
            SetIsTemporary(new ProcRef(process), value);
        }

        public void SetIsTemporary(Vector_IProcess processes, bool value)
        {
            lock (m_lockObject)
            {
                foreach (var process in processes)
                {
                    SetPipelineProperty(new ProcRef(process), "IsTemporary", value);
                }
            }
        }

        public PYXGeometry_SPtr TryGetGeometry(ProcRef procRef)
        {
            PYXGeometry_SPtr geometry = null;

            lock (m_geometryMutex)
            {
                string pathToGeometryFile = m_pathToGeometries +
                    Path.DirectorySeparatorChar +
                    pyxlib.procRefToStr(procRef);

                if (File.Exists(pathToGeometryFile))
                {
                    using (var sr = new StreamReader(pathToGeometryFile, Encoding.UTF8))
                    {
                        try
                        {
                            geometry = PYXGeometrySerializer.deserialize(sr.ReadToEnd());
                        }
                        catch (System.Exception ex)
                        {
                            string errorMessage = string.Format(
                                "Unable to deserialize geometry because:\n{0}", ex.ToString());
                            Trace.error(errorMessage);
                            throw new PipelineLibrary.Exceptions.ErrorDeserializingGeometryException(
                                errorMessage);
                        }

                        if (geometry != null && geometry.get() == null)
                        {
                            geometry = null;
                        }
                    }
                }

                return geometry;
            }
        }

        public void Uninitialize()
        {
            // do nothing
        }

        internal void InvokeFatalError(Exception ex)
        {
            m_onFatalError.Invoke(this, new UnhandledExceptionEventArgs(ex, true));
        }

        internal void OnPipelineGeometryResolved(object sender, PipelineEventArgs e)
        {
            m_PipelineGeometryResolved.Invoke(this, e);
        }

        private void OnPipelineAdded(Pipeline pipeline)
        {
            m_PipelineAdded.Invoke(this,
                new PipelineEventArgs { Pipeline = pipeline });
        }

        private void OnPipelineHidden(Pipeline pipeline)
        {
            m_PipelineHidden.Invoke(this,
                new PipelineEventArgs { Pipeline = pipeline });
        }

        private void OnPipelineRemoved(Pipeline pipeline)
        {
            m_PipelineRemoved.Invoke(this,
                new PipelineEventArgs { Pipeline = pipeline });
        }

        private void OnPipelineUnhidden(Pipeline pipeline)
        {
            m_PipelineUnhidden.Invoke(this,
                new PipelineEventArgs { Pipeline = pipeline });
        }

        private class PipelineRepositoryDTO
        {
            //this is a private member but has to be declared public for deserializing 
            public ConcurrentDictionary<string, JsonPipeline> m_pipelines = new ConcurrentDictionary<string, JsonPipeline>();

            public Pipeline Get(string procRef)
            {
                JsonPipeline pipeline;
                m_pipelines.TryGetValue(procRef, out pipeline);
                return pipeline;
            }

            public IEnumerable<Pipeline> Pipelines()
            {
                {
                    return m_pipelines.Values.ToList();
                }
            }

            public bool ContainsProcRef(string procRef)
            {
                return m_pipelines.ContainsKey(procRef);
            }
            public void Add(string procRef, JsonPipeline pipeline)
            {
                if (pipeline.Children == null)
                {
                    throw new Exception("children is null!");
                }
                m_pipelines.TryAdd(procRef.ToString(), pipeline);
            }
            public bool Remove(string procref)
            {
                JsonPipeline removed;
                return m_pipelines.TryRemove(procref, out removed);
            }
        }
    }

    public class JsonPipeline : Pipeline
    {
        public System.Collections.Generic.List<string> Children { get; set; }
        public JsonPipeline()
            : base()
        {
            this.Children = new System.Collections.Generic.List<string>();
        }
    }
}