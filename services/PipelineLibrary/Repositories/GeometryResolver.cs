/******************************************************************************
GeometryResolver.cs

begin		: October 20, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Services.PipelineLibrary.Domain;
using System;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Repositories
{
    /// <summary>
    /// Resolves geometries for pipelines added to the database.
    /// </summary>
    internal class GeometryResolver
    {
        private Domain.Pipeline m_pipelineToResolve;

        private System.Threading.Thread m_resolverThread = null;

        public GeometryResolver(Domain.Pipeline pipeline)
        {
            m_pipelineToResolve = pipeline;
        }

        /// <summary>
        /// Start the resolution of geometry in a background thread.
        /// </summary>
        /// <param name="repository">The repository.</param>
        /// <param name="pathToGeometries">The path to the geometries folder.</param>
        public void Start(
            IPipelineRepository repository,
            string pathToGeometries)
        {
            // TODO[kabiraman]: This used to be done in SerializeGeometryToFile() in the
            // background thread and we would later on crash when an IProcess's
            // destructor was called.  Specifically, in an IProcess' finalize(),
            // the detachObserversFromInputs() call would crash because the process'
            // input is garbage.  I was unable to find the reason for this crash.
            // Moving the line below to this thread resolves the crash,
            // but this is something to be looked at later on.
            IProcess_SPtr process = PipeManager.getProcess(
                m_pipelineToResolve.ProcRef, true);

            System.Threading.ParameterizedThreadStart resolverStarter =
                delegate
                {
                    this.Resolve(repository, pathToGeometries, process);
                };
            m_resolverThread = new System.Threading.Thread(resolverStarter);
            m_resolverThread.Name = "Geometry Resolver Thread";
            m_resolverThread.IsBackground = true;
            m_resolverThread.Start();
        }

        public void Resolve(
            IPipelineRepository repository,
            string pathToGeometries,
            IProcess_SPtr process)
        {
            try
            {
                if (!repository.Exists(m_pipelineToResolve.ProcRef))
                {
                    return;
                }

                PYXGeometry_SPtr geometry = null;
                string pathToGeometryFile = pathToGeometries +
                    System.IO.Path.DirectorySeparatorChar +
                    pyxlib.procRefToStr(m_pipelineToResolve.ProcRef);

                if (System.IO.Directory.Exists(pathToGeometries))
                {
                    if (System.IO.File.Exists(pathToGeometryFile))
                    {
                        geometry = repository.TryGetGeometry(m_pipelineToResolve.ProcRef);
                        if (geometry == null || geometry.get() == null)
                        {
                            geometry = SerializeGeometryToFile(pathToGeometryFile, process);
                        }
                    }
                    else
                    {
                        geometry =
                            SerializeGeometryToFile(pathToGeometryFile, process);
                    }
                }
                else
                {
                    System.IO.Directory.CreateDirectory(pathToGeometries);
                    geometry = SerializeGeometryToFile(pathToGeometryFile, process);
                }

                if (geometry != null)
                {
                    OnPipelineGeometryResolved(m_pipelineToResolve);
                }
            }
            catch (Exception ex)
            {
                Trace.error(string.Format(
                    "GeometryResolver failed with the following error: {0}",
                    ex.ToString()));
            }
        }

        public event EventHandler<Domain.PipelineEventArgs> PipelineGeometryResolved
        {
            add
            {
                m_pipelineGeometryResolved.Add(value);
            }
            remove
            {
                m_pipelineGeometryResolved.Remove(value);
            }
        }

        private Pyxis.Utilities.EventHelper<PipelineEventArgs> m_pipelineGeometryResolved = new Pyxis.Utilities.EventHelper<PipelineEventArgs>();

        private void OnPipelineGeometryResolved(Pipeline pipelineToResolve)
        {
            m_pipelineGeometryResolved.Invoke(this,
              new PipelineEventArgs { Pipeline = pipelineToResolve });
        }

        public PYXGeometry_SPtr SerializeGeometryToFile(string pathToGeometryFile, IProcess_SPtr process)
        {
            if (process != null && process.get() != null)
            {
                if (m_pipelineToResolve.ProvidesOutputType(IFeature.iid) &&
                    process.getInitState() == IProcess.eInitStatus.knInitialized)
                {
                    IFeature_SPtr feature = pyxlib.QueryInterface_IFeature(
                        process.getOutput());

                    if (feature != null && feature.get() != null)
                    {
                        PYXGeometry_SPtr geometry = feature.getGeometry();

                        if (geometry == null || geometry.get() == null)
                        {
                            return null;
                        }

                        string serializedGeometry = PYXGeometrySerializer.serialize(
                            geometry.get());

                        using (System.IO.StreamWriter sw = new System.IO.StreamWriter(
                            pathToGeometryFile, false, Encoding.UTF8))
                        {
                            sw.Write(serializedGeometry);
                        }

                        return geometry;
                    }
                }
            }

            return null;
        }
    }
}