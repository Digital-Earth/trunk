/******************************************************************************
PipelineRepository.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

using NHibernate;

namespace Pyxis.Services.PipelineLibrary.Repositories
{
    /// <summary>
    /// A class that contains all the methods to perform CRUD (create, read, 
    /// update, delete) operations on the PipelineLibrary database.
    /// </summary>
    /// <remarks>
    /// We use NHibernate to communicate with the database.  The 
    /// NHibernate reference manual is located at 
    /// http://nhforge.org/doc/nh/en/index.html.  Also, one of its authors 
    /// maintains a blog with useful NHibernate details at 
    /// http://ayende.com/Blog/Default.aspx.
    /// </remarks>
    public class PipelineRepository : Domain.IPipelineRepository
    {
        private static Type s_implementationType = typeof(PipelineRepository);

        public static void SetPipelineImplementationType<T>() where T : Domain.IPipelineRepository
        {
            s_implementationType = typeof(T);
        }


        /// <summary>
        /// The one and only instance of the PipelineRepository.
        /// </summary>
        public static Domain.IPipelineRepository Instance
        {
            get
            {
                if (PipelineRepository.s_instance == null)
                {
                    PipelineRepository.s_instance = Activator.CreateInstance(s_implementationType, true) as Domain.IPipelineRepository;
                }
                return PipelineRepository.s_instance;
            }
        }
        private static Domain.IPipelineRepository s_instance;

        #region Member Variables

        private NHibernateManager m_nHibernateManager = null;

        private ISession Session
        {
            get
            {
                return m_nHibernateManager.Session;
            }
        }

        private ITransaction m_transaction = null;

        /// <summary>
        /// The thread on which all database operations take place.
        /// </summary>
        private System.Threading.Thread m_databaseThread;

        /// <summary>
        /// A flag to determine when the database thread should complete 
        /// execution.
        /// </summary>
        private bool m_shouldExit = false;

        private bool m_needFlush = false;

        /// <summary>
        /// All database operations are "jobs", that are maintained by the JobList 
        /// and are executed FIFO by the database thread.
        /// </summary>
        private Pyxis.Utilities.JobList<DatabaseJob> m_jobList =
            new Pyxis.Utilities.JobList<DatabaseJob>();

        private string m_pathToGeometries = AppServices.getWorkingPath().ToString() +
            System.IO.Path.DirectorySeparatorChar +
            "PYXLibrary" + System.IO.Path.DirectorySeparatorChar +
            "Geometries";

        private Pyxis.Utilities.ThreadSafeDictionary<Pyxis.Services.PipelineLibrary.Domain.Pipeline, IProcess_SPtr> m_unstablePipelines
            = new Pyxis.Utilities.ThreadSafeDictionary<Pyxis.Services.PipelineLibrary.Domain.Pipeline, IProcess_SPtr>();

        #endregion Member Variables

        #region Jobs

        /// <summary>
        /// The base class for all database jobs.
        /// </summary>
        private abstract class DatabaseJob : Pyxis.Utilities.Job
        {
            public PipelineRepository Repository { get; set; }

            public override void Execute()
            {
                base.Execute();

                if (this.Exception != null)
                {
                    Trace.error("Database Job has failed with the following error: " + this.Exception.ToString());
                }
            }

            /// <summary>
            /// Used by repository clients to add a job to the job list and wait 
            /// on its completion.
            /// </summary>
            public void ExecuteOnRepository()
            {
                if (Repository == null)
                {
                    this.Execute();
                }
                else
                {
                    var syncEvent = new Pyxis.Utilities.SynchronizationEvent();

                    EventHandler<Pyxis.Utilities.ChangedEventArgs<bool>> handler =
                        (object sender, Pyxis.Utilities.ChangedEventArgs<bool> e) =>
                        {
                            syncEvent.Pulse();
                        };

                    this.Finished.Changed += handler;

                    Repository.m_jobList.Add(this);
                    syncEvent.Wait();

                    this.Finished.Changed -= handler;
                }

                if (this.Exception != null)
                {
                    throw this.Exception;
                }
            }
        }

        private abstract class NonintrusiveDatabaseJob : DatabaseJob
        {
        }

        /// <summary>
        /// Represents a Job that returns a result.
        /// </summary>
        /// <typeparam name="ResultType">The type of result.</typeparam>
        private class JobWithResult<ResultType> : NonintrusiveDatabaseJob
        {
            /// <summary>
            /// Gets or sets the result returned by the job on execution.
            /// </summary>
            /// <value>The result.</value>
            public ResultType Result
            {
                get
                {
                    if (m_result == null)
                    {
                        ExecuteOnRepository();
                    }
                    return m_result;
                }
                set
                {
                    m_result = value;
                }
            }
            private ResultType m_result;
        }

        #region Specific Jobs

        private class CheckPointJob : DatabaseJob
        {
            protected override void DoExecute()
            {
                try
                {
                    Repository.m_transaction.Commit();
                }
                catch (System.Exception ex)
                {
                    Trace.error(
                        string.Format(
                        "Error during database checkpoint, rolling back transaction: {0}",
                        ex.ToString()));

                    Repository.m_transaction.Rollback();
                    Repository.Session.Close();
                    Repository.Session.Dispose();
                }
                finally
                {
                    Repository.Session.Clear();
                    Repository.Session.Disconnect();
                    Repository.Session.Reconnect();
                    Repository.m_transaction = Repository.Session.BeginTransaction();
                }
            }
        }

        private class UninitializeJob : DatabaseJob
        {
            protected override void DoExecute()
            {
                if (Repository.m_transaction != null)
                {
                    try
                    {
                        Repository.m_transaction.Commit();
                    }
                    catch (System.Exception ex)
                    {
                        Trace.error(
                            string.Format(
                            "Error during database uninitialize, rolling back transaction: {0}",
                            ex.ToString()));

                        Repository.m_transaction.Rollback();
                    }
                }

                Repository.Session.Close();

                Repository.m_jobList.Stop();
                Repository.m_shouldExit = true;
            }
        }

        private class AddJob : DatabaseJob
        {
            public IProcess_SPtr Process { get; set; }

            protected override void DoExecute()
            {
                ProcRef procRef = new ProcRef(Process);
                GetByProcRefJob job = new GetByProcRefJob
                {
                    Session = Repository.Session,
                    Guid = procRef.getProcID(),
                    Version = procRef.getProcVersion()
                };
                Domain.Pipeline pipeline = job.Result;

                if (pipeline == null)
                {
                    pipeline = new Domain.Pipeline
                    {
                        Name = Process.getProcName(),
                        Description = Process.getProcDescription(),
                        PipelineGuid = Process.getProcID(),
                        Version = Process.getProcVersion(),
                        IsPublished = false,
                    };
                    pipeline.SetIdentity(Process);
                    pipeline.Definition = PipeManager.writeProcessToNewString(Process);
                    pipeline.AddOutputType(Process.getSpec().getOutputInterfaces());

                    IFeature_SPtr feature =
                        pyxlib.QueryInterface_IFeature(
                            pyxlib.QueryInterface_PYXCOM_IUnknown(Process));

                    if (feature.isNotNull())
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

                    Repository.DoAdd(pipeline);
                }
                else
                {
                    pipeline.Name = Process.getProcName();
                    pipeline.Description = Process.getProcDescription();
                    pipeline.SetIdentity(Process);
                    pipeline.Definition = PipeManager.writeProcessToNewString(Process);
                    pipeline.OutputTypes.Clear();
                    pipeline.AddOutputType(Process.getSpec().getOutputInterfaces());

                    Repository.Update(pipeline);
                }
            }
        }

        private class UpdateJob : DatabaseJob
        {
            public Domain.Pipeline Pipeline { get; set; }

            protected override void DoExecute()
            {
                if (Pipeline.Id == 0)
                {
                    return;
                }

                if (Pipeline.Identity == null)
                {
                    Repository.Session.Evict(Pipeline);
                    Repository.Session.Update(Pipeline);
                }
                else
                {
                    Repository.AddOrUpdateIdentity(Pipeline);
                }
            }
        }

        private class GetByOutputTypeJob : JobWithResult<IList<Domain.Pipeline>>
        {
            public Guid OutputType;

            // TODO[kabiraman]: Can we make this more performant by writing a 
            // NHibernate critiera query?
            protected override void DoExecute()
            {
                var matchingPipelines = new List<Domain.Pipeline>();

                GetAllPipelinesJob job = new GetAllPipelinesJob
                {
                    Session = Repository.Session
                };
                IList<Domain.Pipeline> pipelines = job.Result;

                foreach (Domain.Pipeline pipeline in pipelines)
                {
                    foreach (Domain.PipelineOutputType pipelineOutputType in pipeline.OutputTypes)
                    {
                        if (pipelineOutputType.OutputType == OutputType)
                        {
                            matchingPipelines.Add(pipeline);
                            break;
                        }
                    }
                }

                Result = matchingPipelines;
            }
        }

        private class GetByProcRefJob : JobWithResult<Domain.Pipeline>
        {
            public Guid Guid;

            public int Version;

            public ISession Session;

            protected override void DoExecute()
            {
                ISession session = null;

                if (Repository == null)
                {
                    session = Session;
                }
                else
                {
                    session = Repository.Session;
                }

                Result = session
                    .CreateCriteria(typeof(Domain.Pipeline))
                        .Add(NHibernate.Criterion.Expression.Eq(
                            "PipelineGuid", Guid))
                        .Add(NHibernate.Criterion.Expression.Eq(
                            "Version", Version))
                    .SetMaxResults(1)
                    .SetFlushMode(FlushMode.Never)
                    .UniqueResult<Domain.Pipeline>();
            }
        }

        private class GetByIdentityJob : JobWithResult<IList<Domain.Pipeline>>
        {
            public string Identity;

            protected override void DoExecute()
            {
                Domain.PipelineIdentity identity = Repository.Session
                    .CreateCriteria(typeof(Domain.PipelineIdentity))
                        .Add(NHibernate.Criterion.Expression.Eq("XmlIdentity", Identity))
                    .SetFlushMode(FlushMode.Never)
                    .UniqueResult<Domain.PipelineIdentity>();

                if (identity == null)
                {
                    Result = new List<Domain.Pipeline>();
                }
                else
                {
                    Result = new List<Domain.Pipeline>(identity.Pipelines);
                }
            }
        }

        private class GetAllPipelinesWithUnstableIdentityJob : JobWithResult<IList<Domain.Pipeline>>
        {
            protected override void DoExecute()
            {
                var matchingPipelines = new List<Domain.Pipeline>();
                var pipelines = Repository.Session
                    .CreateCriteria(typeof(Domain.Pipeline))
                        .Add(NHibernate.Criterion.Expression.Eq("IsHidden", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                        .AddOrder(NHibernate.Criterion.Order.Desc("Id"))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.Pipeline>();

                foreach (Domain.Pipeline pipeline in pipelines)
                {
                    if (!pipeline.Identity.IsStable)
                    {
                        matchingPipelines.Add(pipeline);
                    }
                }

                Result = matchingPipelines;
            }
        }

        private class GetAllPipelinesJob :
            JobWithResult<IList<Domain.Pipeline>>
        {
            public ISession Session;

            protected override void DoExecute()
            {
                ISession session = null;

                if (Repository == null)
                {
                    session = Session;
                }
                else
                {
                    session = Repository.Session;
                }

                Result = session
                    .CreateCriteria(typeof(Domain.Pipeline))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.Pipeline>();
            }
        }

        private class GetAllParentPipelinesJob :
            JobWithResult<IList<Domain.Pipeline>>
        {
            protected override void DoExecute()
            {
                Result = Repository.Session
                    .CreateSQLQuery(
                        "SELECT * FROM Pipeline WHERE PipelineId NOT IN " +
                        "(SELECT DISTINCT ChildPipelineId FROM PipelineRelationship)").AddEntity(
                        typeof(Domain.Pipeline))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.Pipeline>();
            }
        }

        private class GetAllPublishedPipelinesJob :
            JobWithResult<IList<Domain.Pipeline>>
        {
            protected override void DoExecute()
            {
                // order by Id DESC so that parent pipelines are available
                // first to the Library UI
                Result = Repository.Session
                    .CreateCriteria(typeof(Domain.Pipeline))
                        .Add(NHibernate.Criterion.Expression.Eq("IsPublished", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsHidden", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                        .AddOrder(NHibernate.Criterion.Order.Desc("Id"))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.Pipeline>();
            }
        }

        private class GetAllDisplayablePipelinesJob :
            JobWithResult<IList<Domain.Pipeline>>
        {
            protected override void DoExecute()
            {
                // order by Id DESC so that parent pipelines are available 
                // first to the Library UI
                Result = Repository.Session
                    .CreateCriteria(typeof(Domain.Pipeline))
                        .Add(NHibernate.Criterion.Expression.Eq("IsHidden", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                        .AddOrder(NHibernate.Criterion.Order.Desc("Id"))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.Pipeline>();
            }
        }

        private class GetAllDisplayablePipelinesWithWhereClauseJob :
            JobWithResult<IList<Domain.Pipeline>>
        {
            public string WhereClause;

            protected override void DoExecute()
            {
                // ORDER BY Id DESC, so that parent pipelines are available 
                // first to the Library UI
                Result = Repository.Session
                    .CreateSQLQuery(string.Format(
                    "SELECT * FROM Pipeline {0} {1}",
                    WhereClause,
                    "AND IsHidden = 0 AND IsTemporary = 0 ORDER BY PipelineId DESC")).AddEntity(typeof(Domain.Pipeline))
                .SetFlushMode(FlushMode.Never)
                .List<Domain.Pipeline>();
            }
        }

        private class ClearDatabaseJob : DatabaseJob
        {
            protected override void DoExecute()
            {
                IList<Domain.PipelineIdentity> identities = Repository.Session
                    .CreateCriteria(typeof(Domain.PipelineIdentity))
                    .List<Domain.PipelineIdentity>();

                foreach (Domain.PipelineIdentity identity in identities)
                {
                    Repository.Session.Delete(identity);
                }
            }
        }

        #endregion Specific Jobs

        #endregion Jobs

        /// <summary>
        /// Default constructor; not to be made available except for unit test, 
        /// use static instance instead.
        /// </summary>
        internal PipelineRepository()
        {
        }

        #region IPipelineRepository Members

        public void Initialize(
            Dictionary<string, string> databaseConfiguration)
        {
            m_nHibernateManager = NHibernateManager.Instance;
            DoInitialize(databaseConfiguration);
        }

        internal void Initialize(NHibernateManager manager)
        {
            m_nHibernateManager = manager;
            DoInitialize(null);
        }

        private void DoInitialize(
            Dictionary<string, string> databaseConfiguration)
        {
            m_databaseThread = new System.Threading.Thread(
                DatabaseWorkerThread_DoWork);
            m_databaseThread.IsBackground = false;
            m_databaseThread.Start(databaseConfiguration);
        }

        public void CheckPoint()
        {
            DatabaseJob newJob = new CheckPointJob
            {
                Repository = this
            };
            newJob.ExecuteOnRepository();
        }

        public void Uninitialize()
        {
            DatabaseJob newJob = new UninitializeJob
            {
                Repository = this
            };
            newJob.ExecuteOnRepository();
        }

        public void Add(IProcess_SPtr process)
        {
            try
            {
                DatabaseJob newJob = new AddJob
                {
                    Process = process,
                    Repository = this
                };
                newJob.ExecuteOnRepository();
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during Add: {0}",
                    ex.ToString()));
            }
        }

        public IList<Domain.Pipeline> GetByOutputType(
            Guid outputType)
        {
            try
            {
                GetByOutputTypeJob newJob = new GetByOutputTypeJob
                {
                    OutputType = outputType,
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetByOutputType: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public Domain.Pipeline GetByProcRef(ProcRef procRef)
        {
            return GetByProcRef(procRef.getProcID(), procRef.getProcVersion());
        }

        public Domain.Pipeline GetByProcRef(Guid guid, int version)
        {
            try
            {
                GetByProcRefJob newJob = new GetByProcRefJob
                {
                    Guid = guid,
                    Version = version,
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetByProcRef: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetByIdentity(string identity)
        {
            try
            {
                GetByIdentityJob newJob = new GetByIdentityJob
                {
                    Repository = this,
                    Identity = identity
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetByIdentity: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllPipelines()
        {
            try
            {
                GetAllPipelinesJob newJob = new GetAllPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllPipelines: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllPublishedPipelines()
        {
            try
            {
                GetAllPublishedPipelinesJob newJob =
                    new GetAllPublishedPipelinesJob
                    {
                        Repository = this
                    };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllPublishedPipelines: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllParentPipelines()
        {
            try
            {
                GetAllParentPipelinesJob newJob =
                    new GetAllParentPipelinesJob
                    {
                        Repository = this
                    };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllParentPipelines: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllDisplayablePipelines()
        {
            try
            {
                GetAllDisplayablePipelinesJob newJob =
                    new GetAllDisplayablePipelinesJob
                    {
                        Repository = this
                    };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllDisplayablePipelines: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllDisplayablePipelines(
            string whereClause)
        {
            try
            {
                GetAllDisplayablePipelinesWithWhereClauseJob newJob =
                     new GetAllDisplayablePipelinesWithWhereClauseJob
                     {
                         WhereClause = whereClause,
                         Repository = this
                     };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format(
                    "Error during GetAllDisplayablePipelines with WHERE clause: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public IList<Domain.Pipeline> GetAllPipelinesWithUnstableIdentity()
        {
            try
            {
                GetAllPipelinesWithUnstableIdentityJob newJob =
                    new GetAllPipelinesWithUnstableIdentityJob
                    {
                        Repository = this
                    };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllPipelinesWithUnstableIdentity: {0}",
                    ex.ToString()));
                return null;
            }
        }

        public bool Exists(ProcRef procRef)
        {
            return ((GetByProcRef(procRef) == null) ? false : true);
        }

        public bool Exists(IProcess_SPtr process)
        {
            return Exists(new ProcRef(process));
        }

        public bool Exists(string identity)
        {
            return ((GetByIdentity(identity).Count == 0) ? false : true);
        }

        public void UpdatePipelineIdentities()
        {
            Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to GetAllPipelinesWithUnstableIdentity()");

            var pipelines = GetAllPipelinesWithUnstableIdentity();

            Trace.info("PipelineRepository.UpdatePipelineIdentities(): After call to GetAllPipelinesWithUnstableIdentity()");

            Trace.info(string.Format("Found {0} unstable pipelines.", pipelines.Count.ToString()));

            foreach (Domain.Pipeline pipeline in pipelines)
            {
                Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to PipeManager.getProcess()");

                IProcess_SPtr process = PipeManager.getProcess(pipeline.ProcRef);

                Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to process.isNotNull()");

                if (process.isNotNull())
                {
                    Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to process.initProc()");

                    // a lot of effort
                    process.initProc();

                    Trace.info(string.Format("Testing pipeline {0} - {1} for stability...",
                        pipeline.Name, pyxlib.procRefToStr(pipeline.ProcRef)));

                    Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to PipeUtils.isPipelineIdentityStable()");

                    if (PipeUtils.isPipelineIdentityStable(process))
                    {
                        Trace.info(string.Format("Pipeline {0} - {1} has become stable.",
                            pipeline.Name, pyxlib.procRefToStr(pipeline.ProcRef)));

                        pipeline.Identity.XmlIdentity = process.getIdentity();
                        pipeline.Identity.IsStable = true;
                        Trace.info("PipelineRepository.UpdatePipelineIdentities(): Before call to Update()");

                        Update(pipeline);
                    }
                    else
                    {
                        Trace.info(string.Format("PipeUtils reported unstable for pipeline {0} - {1}.",
                            pipeline.Name, pyxlib.procRefToStr(pipeline.ProcRef)));
                    }
                }
            }
        }

        private object m_geometryMutex = new object();

        public PYXGeometry_SPtr TryGetGeometry(ProcRef procRef)
        {
            PYXGeometry_SPtr geometry = null;

            lock (m_geometryMutex)
            {
                string pathToGeometryFile = m_pathToGeometries +
                    System.IO.Path.DirectorySeparatorChar +
                    pyxlib.procRefToStr(procRef);

                if (System.IO.File.Exists(pathToGeometryFile))
                {
                    using (System.IO.StreamReader sr =
                        new System.IO.StreamReader(pathToGeometryFile, Encoding.UTF8))
                    {
                        try
                        {
                            geometry =
                                PYXGeometrySerializer.deserialize(sr.ReadToEnd());
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

        public PYXGeometry_SPtr GetGeometry(ProcRef procRef)
        {
            PYXGeometry_SPtr geometry = TryGetGeometry(procRef);

            if (geometry == null)
            {
                Domain.Pipeline pipeline = GetByProcRef(procRef);
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

        public void PersistGeometry(ProcRef procRef)
        {
            string pathToGeometryFile = m_pathToGeometries +
                System.IO.Path.DirectorySeparatorChar +
                pyxlib.procRefToStr(procRef);

            lock (m_geometryMutex)
            {
                Domain.Pipeline pipeline = GetByProcRef(procRef);
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

        public void SetIsPublished(ProcRef procRef, bool value)
        {
            Domain.Pipeline pipeline = GetByProcRef(procRef);

            if (pipeline != null)
            {
                pipeline.IsPublished = value;
                Update(pipeline);
            }
            else
            {
                // throw an exception here?
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Warning!  Unable to set {0}[{1}] as published.  Process not found.",
                    procRef.getProcID(), procRef.getProcVersion()));
            }
        }

        public void SetIsPublished(IProcess_SPtr process, bool value)
        {
            SetIsPublished(new ProcRef(process), value);
        }

        public void SetIsHidden(ProcRef procRef, bool value)
        {
            Domain.Pipeline pipeline = null;

            pipeline = GetByProcRef(procRef);

            if (pipeline != null)
            {
                pipeline.IsHidden = value;
                Update(pipeline);
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

        public void SetIsTemporary(ProcRef procRef, bool value)
        {
            Domain.Pipeline pipeline = GetByProcRef(procRef);

            if (pipeline != null)
            {
                pipeline.IsTemporary = value;
                Update(pipeline);
            }
            else
            {
                // throw an exception here?
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Warning!  Unable to set {0}[{1}] as temporary.  Process not found.",
                    procRef.getProcID(), procRef.getProcVersion()));
            }
        }

        public void SetIsTemporary(IProcess_SPtr process, bool value)
        {
            SetIsTemporary(new ProcRef(process), value);
        }

        public void SetIsTemporary(Vector_IProcess processes, bool value)
        {
            var pipelinesToUpdate = new List<Domain.Pipeline>();

            foreach (var process in processes)
            {
                var procRef = new ProcRef(process);
                Domain.Pipeline pipeline = GetByProcRef(procRef);

                if (pipeline == null)
                {
                    // throw an exception here?
                    System.Diagnostics.Trace.WriteLine(string.Format(
                        "Warning!  Unable to set {0}[{1}] as temporary.  Process not found.",
                        procRef.getProcID(), procRef.getProcVersion()));

                    continue;
                }

                if (pipeline.IsTemporary != value)
                {
                    pipelinesToUpdate.Add(pipeline);
                }
            }

            //set temporary value
            pipelinesToUpdate.ForEach(p => p.IsTemporary = value);

            //update all in bulk
            pipelinesToUpdate.ForEach(p => Update(p));
        }


        #region Events

        public event EventHandler<Domain.PipelineEventArgs> PipelineAdded
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
        private Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs> m_PipelineAdded = new Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs>();

        private void OnPipelineAdded(Domain.Pipeline pipeline)
        {
            m_PipelineAdded.Invoke(this,
                new Domain.PipelineEventArgs { Pipeline = pipeline });
        }

        public event EventHandler<Domain.PipelineEventArgs> PipelineRemoved
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
        private Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs> m_PipelineRemoved = new Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs>();

        private void OnPipelineRemoved(Domain.Pipeline pipeline)
        {
            m_PipelineRemoved.Invoke(this,
                new Domain.PipelineEventArgs { Pipeline = pipeline });
        }

        public event EventHandler<Domain.PipelineEventArgs> PipelineHidden
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
        private Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs> m_PipelineHidden = new Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs>();

        private void OnPipelineHidden(Domain.Pipeline pipeline)
        {
            m_PipelineHidden.Invoke(this,
                new Domain.PipelineEventArgs { Pipeline = pipeline });
        }

        public event EventHandler<Domain.PipelineEventArgs> PipelineUnhidden
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
        private Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs> m_PipelineUnhidden =
            new Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs>();

        private void OnPipelineUnhidden(Domain.Pipeline pipeline)
        {
            m_PipelineUnhidden.Invoke(this,
                new Domain.PipelineEventArgs { Pipeline = pipeline });
        }

        public event EventHandler<Domain.PipelineEventArgs> PipelineGeometryResolved
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
        private Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs> m_PipelineGeometryResolved =
            new Pyxis.Utilities.EventHelper<Domain.PipelineEventArgs>();

        internal void OnPipelineGeometryResolved(object sender, Domain.PipelineEventArgs e)
        {
            m_PipelineGeometryResolved.Invoke(this, e);
        }

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
        private Pyxis.Utilities.EventHelper<UnhandledExceptionEventArgs> m_onFatalError =
            new Pyxis.Utilities.EventHelper<UnhandledExceptionEventArgs>();

        internal void InvokeFatalError(Exception ex)
        {
            m_onFatalError.Invoke(this, new UnhandledExceptionEventArgs(ex, true));
        }

        #endregion Events

        #endregion IPipelineRepository Members

        #region Private Methods

        #region Job-Related

        /// <summary>
        /// The database thread runs here.
        /// </summary>
        /// <param name="databaseConfiguration">The database configuration.</param>
        private void DatabaseWorkerThread_DoWork(object databaseConfiguration)
        {
            try
            {
                if (databaseConfiguration == null)
                {
                    m_nHibernateManager.Initialize();
                }
                else
                {
                    m_nHibernateManager.Initialize(
                        databaseConfiguration as Dictionary<string, string>);
                }

                m_transaction = Session.BeginTransaction();

                RemoveTemporaryPipelines();
                ChangeUndefinedIdentityStabilityToFalse();

                while (!m_shouldExit)
                {
                    DatabaseJob j = m_jobList.Get();

                    if (j != null)
                    {
                        if ((j is NonintrusiveDatabaseJob))
                        {
                            if (m_needFlush)
                            {
                                Session.Flush();
                                m_needFlush = false;
                            }
                        }
                        else
                        {
                            m_needFlush = true;
                        }

                        j.Execute();
                        j.Finished.Value = true;
                    }
                }
            }
            catch (System.Exception ex)
            {
                Trace.error("Database failed : " + ex.ToString());
                InvokeFatalError(ex);
            }
        }

        #endregion Job-Related

        /// <summary>
        /// Removes all temporary pipelines from the database.
        /// </summary>
        private void RemoveTemporaryPipelines()
        {
            try
            {
                IList<Domain.Pipeline> pipelines = Session
                    .CreateCriteria(typeof(Domain.Pipeline))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", true))
                        .List<Domain.Pipeline>();

                IList<Domain.GeoWebStreamServerPipelineStatus> pipelinesStatus = Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .List<Domain.GeoWebStreamServerPipelineStatus>();

                foreach (Domain.Pipeline pipeline in pipelines)
                {
                    Session.Delete(pipeline);

                    var status = pipelinesStatus.FirstOrDefault(x => x.Id == pipeline.Id);

                    if (status != null)
                    {
                        Session.Delete(status);
                    }
                }

                if (pipelines.Count > 0)
                {
                    Session.Transaction.Commit();
                    Session.Clear();
                    Session.Disconnect();
                    Session.Reconnect();
                    m_transaction = Session.BeginTransaction();
                }
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during RemoveTemporaryPipelines: {0}",
                    ex.ToString()));
            }
        }

        /// <summary>
        /// For a database that was created before the addition of the IsStable column, existing rows in the 
        /// PipelineIdentity table will have an undefined value for IsStable.  This method sets all undefined 
        /// values to false and is to be called at start up.
        /// </summary>
        private void ChangeUndefinedIdentityStabilityToFalse()
        {
            try
            {
                PipelineIdentityRepository repository = new PipelineIdentityRepository();
                IList<Domain.PipelineIdentity> identites = Session
                    .CreateCriteria(typeof(Domain.PipelineIdentity))
                        .List<Domain.PipelineIdentity>();

                Trace.info(string.Format("Found {0} total identities.", identites.Count.ToString()));

                foreach (Domain.PipelineIdentity identity in identites)
                {
                    if (!identity.IsStable)
                    {
                        Session.Evict(identity);
                        repository.Update(identity, Session);
                    }
                }
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during ChangeNullIdentityStabilityToFalse: {0}",
                    ex.ToString()));
            }
        }

        /// <summary>
        /// Add the pipeline to the database.
        /// </summary>
        /// <param name="pipeline">The pipeline.</param>
        private void DoAdd(Domain.Pipeline pipeline)
        {
            if (pipeline.Identity == null)
            {
                try
                {
                    Session.Save(pipeline);
                }
                catch (System.Exception ex)
                {
                    Trace.error(
                        string.Format("Error during DoAdd: {0}",
                        ex.ToString()));
                }
            }
            else
            {
                AddOrUpdateIdentity(pipeline);
            }

            OnPipelineAdded(pipeline);
        }

        /// <summary>
        /// Updates the specified pipeline in the database.  
        /// If the provided pipeline's identity is not null, we check to see 
        /// if the identity already exists in the database.  If it does, then 
        /// the pipeline is associated to that identity.  If the identity does 
        /// not exist in the database, the identity is added to the database 
        /// and then the pipeline is updated.
        /// </summary>
        /// <param name="pipeline">The pipeline.</param>
        private void Update(Domain.Pipeline pipeline)
        {
            try
            {
                DatabaseJob newJob = new UpdateJob
                {
                    Pipeline = pipeline,
                    Repository = this
                };
                newJob.ExecuteOnRepository();
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during database update: {0}",
                    ex.ToString()));
            }
        }

        /// <summary>
        /// 1. If the identity of the provided pipeline doesn't already 
        /// exist in the database, adds the 
        /// identity and adds/updates the pipeline to the database.
        /// 2. If the identity already exists in the database, assigns the 
        /// identity to the pipeline.
        /// </summary>
        /// <param name="pipeline">The pipeline.</param>
        private void AddOrUpdateIdentity(Domain.Pipeline pipeline)
        {
            var respository = new PipelineIdentityRepository();
            string identityString = string.Empty;

            try
            {
                identityString = pipeline.Identity.XmlIdentity;

                Domain.PipelineIdentity identity =
                    respository.GetByIdentity(identityString, Session);

                if (identity == null)
                {
                    respository.Add(pipeline.Identity, Session);
                }
                else
                {
                    pipeline.SetIdentity(identity);
                    Session.Evict(identity);
                    respository.Update(identity, Session);
                }
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during AddOrUpdateIdentity: {0}",
                    ex.ToString()));
            }
        }



        #endregion Private Methods

        #region Methods Exclusive to Unit Testing

        internal void ClearDatabase()
        {
            try
            {
                DatabaseJob newJob = new ClearDatabaseJob
                {
                    Repository = this
                };
                newJob.ExecuteOnRepository();
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during ClearDatabase: {0}",
                    ex.ToString()));
            }
        }

        #endregion Methods Exclusive to Unit Testing

        #region PipelineStatus Methods

        public void AddPipelineStatus(string pipelineDefinition)
        {
            try
            {
                DatabaseJob newJob = new AddPipelineStatusJob
                {
                    PipelineDefinition = pipelineDefinition,
                    Repository = this
                };
                newJob.ExecuteOnRepository();
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during AddPipelineStatus: {0}",
                    ex.ToString()));
            }
        }

        public void SetPipelineId(string pipelineDefinition, ProcRef procRef)
        {
            Domain.GeoWebStreamServerPipelineStatus status = GetByDefinition(pipelineDefinition);
            Domain.Pipeline pipeline = GetByProcRef(procRef);
            status.PipelineId = pipeline.Id;

            UpdatePipelineStatusJob job = new UpdatePipelineStatusJob
            {
                Status = status,
                Repository = this
            };
            job.ExecuteOnRepository();
        }

        public void SetIsImported(ProcRef procRef, bool isImported)
        {
            Domain.GeoWebStreamServerPipelineStatus status = GetPipelineStatusByProcRef(procRef);
            status.IsImported = isImported;

            UpdatePipelineStatusJob job = new UpdatePipelineStatusJob
            {
                Status = status,
                Repository = this
            };
            job.ExecuteOnRepository();
        }

        public void SetIsDownloaded(ProcRef procRef, bool isDownloaded)
        {
            Domain.Pipeline pipeline = GetByProcRef(procRef);
            Domain.GeoWebStreamServerPipelineStatus status = GetByPipelineId(pipeline.Id);
            status.IsDownloaded = isDownloaded;

            UpdatePipelineStatusJob job = new UpdatePipelineStatusJob
            {
                Status = status,
                Repository = this
            };
            job.ExecuteOnRepository();
        }

        public void SetIsProcessed(ProcRef procRef, bool isProcessed)
        {
            Domain.Pipeline pipeline = GetByProcRef(procRef);
            Domain.GeoWebStreamServerPipelineStatus status = GetByPipelineId(pipeline.Id);
            status.IsProcessed = isProcessed;

            UpdatePipelineStatusJob job = new UpdatePipelineStatusJob
            {
                Status = status,
                Repository = this
            };
            job.ExecuteOnRepository();
        }

        public IList<string> GetNotImportedPipelines()
        {
            try
            {
                GetNotImportedPipelinesJob newJob = new GetNotImportedPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetNotImportedPipelines: {0}",
                    ex.ToString()));
                return new List<string>();
            }
        }

        public IList<ProcRef> GetNotDownloadedPipelines()
        {
            try
            {
                GetNotDownloadedPipelinesJob newJob = new GetNotDownloadedPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetNotDownloadedPipelines: {0}",
                    ex.ToString()));
                return new List<ProcRef>();
            }
        }

        public IList<ProcRef> GetNotPublishedPipelines()
        {
            try
            {
                GetNotPublishedPipelinesJob newJob = new GetNotPublishedPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetNotPublishedPipelines: {0}",
                    ex.ToString()));
                return new List<ProcRef>();
            }
        }

        public IList<ProcRef> GetNotProcessedPipelines()
        {
            try
            {
                GetNotProcessedPipelinesJob newJob = new GetNotProcessedPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetNotProcessedPipelines: {0}",
                    ex.ToString()));
                return new List<ProcRef>();
            }
        }

        public IList<Domain.GeoWebStreamServerPipelineStatus> GetAllGeoWebStreamServerPipelines()
        {
            try
            {
                GetAllGeoWebStreamServerPipelinesJob newJob = new GetAllGeoWebStreamServerPipelinesJob
                {
                    Repository = this
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetAllGeoWebStreamServerPipelines: {0}",
                    ex.ToString()));
                return new List<Domain.GeoWebStreamServerPipelineStatus>();
            }
        }

        public Domain.GeoWebStreamServerPipelineStatus GetPipelineStatusByProcRef(ProcRef procRef)
        {
            try
            {
                GetPipelineStatusByProcRefJob newJob = new GetPipelineStatusByProcRefJob
                {
                    Repository = this,
                    ProcRef = procRef
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetPipelineStatusByProcRef: {0}",
                    ex.ToString()));
                return null;
            }
        }

        #endregion PipelineStatus Methods

        #region PipelineStatus Jobs

        private class AddPipelineStatusJob : DatabaseJob
        {
            public string PipelineDefinition { get; set; }

            protected override void DoExecute()
            {
                try
                {

                    IList<Domain.GeoWebStreamServerPipelineStatus> allStatuses = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                    .List<Domain.GeoWebStreamServerPipelineStatus>();

                    foreach (var status in allStatuses)
                    {
                        // don't allow duplicates
                        if (status.PipelineDefinition == PipelineDefinition)
                        {
                            return;
                        }
                    }

                    var newStatus = new Domain.GeoWebStreamServerPipelineStatus();
                    newStatus.PipelineDefinition = PipelineDefinition;
                    newStatus.IsImported = newStatus.IsDownloaded = newStatus.IsPublished =
                        newStatus.IsProcessed = false;

                    Repository.Session.Save(newStatus);
                }
                catch (System.Exception ex)
                {
                    Trace.error(
                        string.Format("Error during AddPiplineStatusJob: {0}",
                        ex.ToString()));
                }
            }
        }

        private class UpdatePipelineStatusJob : DatabaseJob
        {
            public Domain.GeoWebStreamServerPipelineStatus Status { get; set; }

            protected override void DoExecute()
            {
                try
                {

                    Repository.Session.Evict(Status);
                    Repository.Session.Update(Status);
                }
                catch (System.Exception ex)
                {
                    Trace.error(
                        string.Format("Error during UpdatePiplineStatusJob: {0}",
                        ex.ToString()));
                }
            }
        }

        private class GetByDefinitionJob : JobWithResult<Domain.GeoWebStreamServerPipelineStatus>
        {
            public string PipelineDefinition { get; set; }

            protected override void DoExecute()
            {
                Domain.GeoWebStreamServerPipelineStatus status = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("PipelineDefinition", PipelineDefinition))
                    .SetFlushMode(FlushMode.Never)
                    .UniqueResult<Domain.GeoWebStreamServerPipelineStatus>();

                Result = status;
            }
        }

        private class GetByPipelineIdJob : JobWithResult<Domain.GeoWebStreamServerPipelineStatus>
        {
            public int PipelineId { get; set; }

            protected override void DoExecute()
            {
                Domain.GeoWebStreamServerPipelineStatus status = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("PipelineId", PipelineId))
                    .SetFlushMode(FlushMode.Never)
                    .UniqueResult<Domain.GeoWebStreamServerPipelineStatus>();

                Result = status;
            }
        }


        /// <summary>
        /// Helper class for convert GeoWebStreamServerPipelineStatus into valid ProcRef
        /// </summary>
        private class GwssPipelineStatusToProcRefListJob : JobWithResult<IList<ProcRef>>
        {
            protected List<ProcRef> ConvertToValidProcRef(IEnumerable<Domain.GeoWebStreamServerPipelineStatus> statuses)
            {
                var procRefs = new List<ProcRef>();
                foreach (var status in statuses)
                {
                    var pipeline = Repository.Session
                        .CreateCriteria(typeof(Domain.Pipeline))
                            .Add(NHibernate.Criterion.Expression.Eq("Id", status.PipelineId))
                            .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                        .SetFlushMode(FlushMode.Never)
                        .UniqueResult<Domain.Pipeline>();

                    if (pipeline != null)
                    {
                        procRefs.Add(pipeline.ProcRef);
                    }
                }

                return procRefs;
            }
        }

        private class GetNotImportedPipelinesJob : JobWithResult<IList<string>>
        {
            protected override void DoExecute()
            {
                IList<Domain.GeoWebStreamServerPipelineStatus> statuses = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("IsImported", false))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.GeoWebStreamServerPipelineStatus>();

                var definitions = new List<string>();

                foreach (Domain.GeoWebStreamServerPipelineStatus status in statuses)
                {
                    definitions.Add(status.PipelineDefinition);
                }

                Result = definitions;
            }
        }

        private class GetNotDownloadedPipelinesJob : GwssPipelineStatusToProcRefListJob
        {
            protected override void DoExecute()
            {
                IList<Domain.GeoWebStreamServerPipelineStatus> statuses = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("IsImported", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsDownloaded", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.GeoWebStreamServerPipelineStatus>();

                Result = ConvertToValidProcRef(statuses);
            }
        }

        private class GetNotPublishedPipelinesJob : GwssPipelineStatusToProcRefListJob
        {
            protected override void DoExecute()
            {
                IList<Domain.GeoWebStreamServerPipelineStatus> statuses = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("IsImported", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsDownloaded", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsPublished", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.GeoWebStreamServerPipelineStatus>();

                Result = ConvertToValidProcRef(statuses);
            }
        }

        private class GetNotProcessedPipelinesJob : GwssPipelineStatusToProcRefListJob
        {
            protected override void DoExecute()
            {
                IList<Domain.GeoWebStreamServerPipelineStatus> statuses = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("IsImported", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsDownloaded", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsPublished", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsProcessed", false))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.GeoWebStreamServerPipelineStatus>();

                Result = ConvertToValidProcRef(statuses);
            }
        }

        private class GetAllGeoWebStreamServerPipelinesJob : JobWithResult<IList<Domain.GeoWebStreamServerPipelineStatus>>
        {
            protected override void DoExecute()
            {
                Result = Repository.Session
                    .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                        .Add(NHibernate.Criterion.Expression.Eq("IsImported", true))
                        .Add(NHibernate.Criterion.Expression.Eq("IsTemporary", false))
                    .SetFlushMode(FlushMode.Never)
                    .List<Domain.GeoWebStreamServerPipelineStatus>();
            }
        }

        private class GetPipelineStatusByProcRefJob : JobWithResult<Domain.GeoWebStreamServerPipelineStatus>
        {
            public ProcRef ProcRef { get; set; }

            protected override void DoExecute()
            {
                GetByProcRefJob job = new GetByProcRefJob
                {
                    Session = Repository.Session,
                    Guid = ProcRef.getProcID(),
                    Version = ProcRef.getProcVersion()
                };
                Domain.Pipeline pipeline = job.Result;

                if (pipeline == null)
                {
                    Result = null;
                }
                else
                {
                    Domain.GeoWebStreamServerPipelineStatus status = Repository.Session
                        .CreateCriteria(typeof(Domain.GeoWebStreamServerPipelineStatus))
                            .Add(NHibernate.Criterion.Expression.Eq("PipelineId", pipeline.Id))
                        .SetFlushMode(FlushMode.Never)
                        .UniqueResult<Domain.GeoWebStreamServerPipelineStatus>();

                    Result = status;
                }
            }
        }

        #endregion PipelineStatus Jobs

        private Domain.GeoWebStreamServerPipelineStatus GetByDefinition(string pipelineDefinition)
        {
            try
            {
                GetByDefinitionJob newJob = new GetByDefinitionJob
                {
                    Repository = this,
                    PipelineDefinition = pipelineDefinition
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetByDefinition: {0}",
                    ex.ToString()));
                return null;
            }
        }

        private Domain.GeoWebStreamServerPipelineStatus GetByPipelineId(int pipelineId)
        {
            try
            {
                GetByPipelineIdJob newJob = new GetByPipelineIdJob
                {
                    Repository = this,
                    PipelineId = pipelineId
                };
                return newJob.Result;
            }
            catch (System.Exception ex)
            {
                Trace.error(
                    string.Format("Error during GetByPipelineId: {0}",
                    ex.ToString()));
                return null;
            }
        }


        public bool TryRemovePipeline(ProcRef procRef)
        {
            SetIsTemporary(procRef, true);
            return true;
        }

        public void SetProcessedResolution(ProcRef procRef, int resolution)
        {
            Domain.Pipeline pipeline = GetByProcRef(procRef);
            Domain.GeoWebStreamServerPipelineStatus status = GetByPipelineId(pipeline.Id);
            status.MaxProcessedResolution = resolution;

            UpdatePipelineStatusJob job = new UpdatePipelineStatusJob
            {
                Status = status,
                Repository = this
            };
            job.ExecuteOnRepository();
        }
    }
}
