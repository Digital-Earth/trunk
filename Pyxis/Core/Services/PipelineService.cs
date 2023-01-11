using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Services
{
    /// <summary>
    /// A core service for initializing PYXIS processes and caching initialized processes to avoid initializing more than once.
    /// 
    /// However, this class should be smart enough not to keep IProcess_SPtr that are no longer being used,
    /// at least by C# code.
    /// 
    /// In order to do, this class has a Generation Based Cache.
    /// Level 1: new processes that just been created
    /// Level 2: recent processes
    /// Level 3: old processes
    /// 
    /// Level 1 and Level 2 keep real (hard) reference to IProcess class, while Level 3 keep a WeakPointer reference.
    /// This means that while a process is in level1 or level2 - it will kept alive by PipelineService.
    /// And it will remain alive in level3 only if there other code in C# that have ref to it.
    /// 
    /// Every time a function GetProcess() is called, it will result that process to be add/moved into Level1 cache.
    /// </summary>
    internal class PipelineService : ServiceBase
    {
        private EngineConfig Config { get; set; }

        /// <summary>
        /// State lock for the Levels process cache.
        /// </summary>
        private readonly object m_lock = new object();

        /// <summary>
        /// State lock for accessing PipeManager
        /// </summary>
        private readonly object m_deepLock = new object();
        
        /// <summary>
        /// Level 1 Process cache
        /// </summary>
        private Dictionary<string, IProcess_SPtr> m_newProcesses;

        /// <summary>
        /// Level 2 Process cache
        /// </summary>
        private Dictionary<string, IProcess_SPtr> m_recentProcesses;

        /// <summary>
        /// Level 3 Process cache - Weak reference for long lived IProcess
        /// </summary>
        private Dictionary<string, WeakReference<IProcess_SPtr>> m_oldProcesses;

        private const int NEW_PROCESS_CACHE_LIMIT = 10;
        private const int RECENT_PROCESS_CACHE_LIMIT = 40;

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.Services.PipelineService.
        /// </summary>
        /// <param name="config"></param>
        public PipelineService(EngineConfig config)
        {
            Config = config;
        }
        
        /// <summary>
        /// Starts the Pyxis.Core.Services.PipelineService by initializing an empty process cache.
        /// </summary>
        protected override void StartService()
        {
            lock (m_lock)
            {
                //Level1
                m_newProcesses = new Dictionary<string, IProcess_SPtr>();

                //Level2
                m_recentProcesses = new Dictionary<string, IProcess_SPtr>();

                //Level3 (WeakRef)
                m_oldProcesses = new Dictionary<string, WeakReference<IProcess_SPtr>>();
            }
        }

        /// <summary>
        /// Stops the Pyxis.Core.Services.PipelineService by emptying the process cache.
        /// </summary>
        protected override void StopService()
        {
            lock (m_lock)
            {
                //force removing swig handles into c++ domain
                foreach (var process in m_newProcesses.Values)
                {
                    process.Dispose();
                }
                foreach (var process in m_recentProcesses.Values)
                {
                    process.Dispose();
                }
                foreach (var weakRef in m_oldProcesses.Values)
                {
                    IProcess_SPtr process;
                    if (weakRef.TryGetTarget(out process))
                    {
                        process.Dispose();
                    }
                }
                m_newProcesses.Clear();
                m_recentProcesses.Clear();
                m_oldProcesses.Clear(); 

                //allow the Garbage Collection to catch up
                GC.Collect();

                //notify PipeManager it can try to release memory if possible
                PipeManager.releaseMemoryIfPossible();
            }
        }

        internal string KeyFromResource(ResourceReference resource)
        {
            return JsonConvert.SerializeObject(resource);
        }

        private bool TryGetProcess(string key, out IProcess_SPtr process)
        {
            lock (m_lock)
            {
                //try fetch from level-1
                if (m_newProcesses.TryGetValue(key, out process))
                {
                    return true;
                }

                //try fetch from level-2
                if (m_recentProcesses.TryGetValue(key, out process))
                {
                    //upgrade to level-1
                    m_recentProcesses.Remove(key);
                    m_newProcesses.Add(key, process);
                    return true;
                }

                WeakReference<IProcess_SPtr> weakRef;

                //try fetch from level-3 (weak ref)
                if (m_oldProcesses.TryGetValue(key, out weakRef))
                {
                    if (weakRef.TryGetTarget(out process))
                    {
                        //upgrade to level-1
                        m_oldProcesses.Remove(key);
                        m_newProcesses.Add(key, process);
                        return true;
                    }
                    else
                    {
                        //process no longer been used
                        m_oldProcesses.Remove(key);
                    }
                }

                return false;
            } 
        }

        /// <summary>
        /// This function downgrade process from level 1 into level 2, level 2 into level 3, level 3 is get purged from all broken weak references.
        /// </summary>
        private void CycleGenerations()
        {
            PurgeOldProcessCache();

            //check to see if will pass the recent count threshold 
            if (m_recentProcesses.Count + m_newProcesses.Count < RECENT_PROCESS_CACHE_LIMIT)
            {
                //add all new into recent...
                foreach(var keyValue in m_newProcesses) 
                {
                    m_recentProcesses.Add(keyValue.Key,keyValue.Value);
                }                
            }
            else
            {
                //recent cache is full, move recent to old
                foreach (var keyValue in m_recentProcesses)
                {
                    m_oldProcesses.Add(keyValue.Key, new WeakReference<IProcess_SPtr>(keyValue.Value));
                }

                //new become recent
                m_recentProcesses = m_newProcesses;
            }

            //make a new empty new list
            m_newProcesses = new Dictionary<string, IProcess_SPtr>();
        }

        /// <summary>
        /// Purge broken weak references from OldProcessCache
        /// </summary>
        private void PurgeOldProcessCache()
        {
            GC.Collect();

            //trim old process
            var oldProcesses = new Dictionary<string, WeakReference<IProcess_SPtr>>();
            foreach (var weakRef in m_oldProcesses)
            {
                IProcess_SPtr stillAliveProcess;

                if (weakRef.Value.TryGetTarget(out stillAliveProcess))
                {
                    oldProcesses.Add(weakRef.Key, weakRef.Value);
                }
            }            
            m_oldProcesses = oldProcesses;
        }

        /// <summary>
        /// Try to retrieve a process from a Pyxis.Contract.Publishing.ResourceReference.
        /// In order for this function to work, a process must be initialized using
        /// the complete referenced resource.
        /// </summary>
        /// <param name="resource">Point to resource Id and version</param>
        /// <param name="process">Out parameter, the process if exists</param>
        /// <returns>true if process was found; otherwise, false</returns>
        public bool TryGetProcess(ResourceReference resource, out IProcess_SPtr process)
        {
            return TryGetProcess(KeyFromResource(resource), out process);   
        }

        /// <summary>
        /// Initialize the process using the Pipeline definition field.
        /// A cache of all active pipelines is kept to avoid
        /// initializing the same process twice.
        /// </summary>
        /// <param name="pipeline">The requested pipeline</param>
        /// <returns>Initialized process</returns>
        public IProcess_SPtr GetProcess(Pipeline pipeline)
        {
            var key = KeyFromResource(ResourceReference.FromResource(pipeline));
            IProcess_SPtr process;

            //do fast check to get already existing processes
            lock (m_lock)
            {
                //try fetch process from cache
                if (TryGetProcess(key, out process))
                {
                    return new IProcess_SPtr(process);
                }
            }

            //do a deep lock when accessing PipeManager...
            lock (m_deepLock)
            {
                //now that we are in a deepLock - try to see if we got the process already
                lock (m_lock)
                {
                    //try fetch process from cache
                    if (TryGetProcess(key, out process))
                    {
                        //validate result is not null
                        if (process != null && process.isNotNull())
                        {
                            return new IProcess_SPtr(process);
                        }
                    }
                }

                try
                {
                    //try fetch process from PipeManager cache using ProcRef
                    process = PipeManager.getProcess(pyxlib.strToProcRef(pipeline.ProcRef), true);
                    if (process != null && process.isNull())
                    {
                        process = null;
                    }
                }
                catch (Exception ex)
                {
                    Trace.debug("An exception was thrown when calling PipeManager.getProcess : " + ex.Message);
                    process = null;
                }

                //create new process from pipeline definition
                if (process == null)
                {
                    process = PipeManager.readPipelineFromString(pipeline.Definition);
                    PipeManager.import(process);

                    process.initProc(true);
                }

                //make sure the process was initialized correctly
                if (process.getInitState() != IProcess.eInitStatus.knInitialized)
                {
                    var errorProc = PipeUtils.findFirstError(process);
                    var error = errorProc.getInitError();

                    Trace.debug("Failed to initialize pipeline " + pipeline.Metadata.Name + " due to an error in process (" + new ProcRef(errorProc) + ") :" + error.getError());
                    return null;
                }

                //add into process cache
                lock (m_lock)
                {
                    //add process into cache
                    if (m_newProcesses.Count >= NEW_PROCESS_CACHE_LIMIT)
                    {
                        CycleGenerations();
                    }
                    m_newProcesses[key] = process;

                    return new IProcess_SPtr(process);
                }
            }           
        }
    }
}
