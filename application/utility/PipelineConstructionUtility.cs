/******************************************************************************
PipelineConstructionUtility.cs

begin      : May 14, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Repositories;
#endif

namespace ApplicationUtility
{
    /// <summary>
    /// Utility class to aid in Pipeline construction. Common pieces of automated pipeline generation are 
    /// found here. 
    /// </summary>
    public static class PipelineConstructionUtility
    {
        //todo: This struct + all the Import Prune Results are never used. the rest of the function shere can be moved out of Application Utility to Pyxis.IO
        /// <summary>
        /// Light weight object to store information returned from a call to import and prune.
        /// This structure holds onto the number of pipelines that were imported into the library
        /// as well as the root process of the pipeline, that is imported. The number of piplines
        /// imported is used for reporting during massive data import. The head of the pipeline
        /// imported is stored to provide access to further processing.
        /// </summary>
        public struct ImportPruneResult
        {
            /// <summary>
            /// The head process that was imported during an import and prune operation.
            /// </summary>
            IProcess_SPtr m_headProc;

            /// <summary>
            /// The number of pipelines that were imported during an import and prune operation.
            /// </summary>
            int m_procImportedCount;


            /// <summary>
            /// Construct a structure to store the head process and the number of piplines that 
            /// were imported.
            /// </summary>
            /// <param name="headProcess">A IProcess_SPtr pointing to the head process.</param>
            /// <param name="procImportedCount">The number of processes stored.</param>
            public ImportPruneResult(IProcess_SPtr headProcess, int procImportedCount)
            {
                m_headProc = headProcess;
                m_procImportedCount = procImportedCount;
            }

            /// <summary>
            /// Get the head process stored in this structure.
            /// </summary>
            public IProcess_SPtr HeadProcess
            {
                get
                {
                    return m_headProc;
                }
            }

            /// <summary>
            /// Get the number of processes imported, which are stored in this structure.
            /// </summary>
            public int NumberOfProcessesImported
            {
                get
                {
                    return m_procImportedCount;
                }
            }
        }

        /// <summary>
        /// Finds and instantiates all of the pipeline builders registered within Pyxis. The 
        /// pipeline builders are found via PYXCOM and instantiated by GUID. 
        /// </summary>
        /// <returns>A List of pipeline builders found within the system.</returns>
        public static List<IPipeBuilder_SPtr> GetPipelineBuilders()
        {
            List<IPipeBuilder_SPtr> builders = new List<IPipeBuilder_SPtr>();
            Vector_GUID guids = new Vector_GUID();

            //Find all the pipeline builders in the system.
            pyxlib.PYXCOMGetClassIDs(IPipeBuilder.iid, guids);

            //Create instances of those pipeline builders.
            foreach (GUID g in guids)
            {
                IUnknown_SPtr spUnknown = pyxlib.PYXCOMhelpCreate(g);
                IPipeBuilder_SPtr pipeBuilder = pyxlib.QueryInterface_IPipeBuilder(spUnknown);

                if ((pipeBuilder != null) && (pipeBuilder.get() != null))
                {
                    builders.Add(pipeBuilder);
                }
            }
            return builders;
        }

        /// <summary>
        /// Determines if a particular data source can be loaded. The algorithm will modify its
        /// behaviour based on the options specified.
        /// </summary>
        /// <param name="strPath">The potential data source.</param>
        /// <param name="options">The type of scan to perform.</param>
        /// <returns>true if the data source can be loaded, otherwise false.</returns>
        public static bool IsDataSourceSupported(String strPath, IPipeBuilder.eCheckOptions options)
        {
            foreach (IPipeBuilder_SPtr builder in GetPipelineBuilders())
            {

                if (builder.isDataSourceSupported(strPath, options))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Cycles through the list of pipeline builders, when a builder is found that 
        /// supports the specified path then it builds the catalog for the path.
        /// </summary>
        /// <param name="path">The path for which to build the catalog.</param>
        /// <returns>The catalog or null if no catalog was loaded.</returns>
        public static PYXCatalog_CSPtr BuildCatalog(string path)
        {
            foreach (IPipeBuilder_SPtr builder in GetPipelineBuilders())
            {
                try
                {
                    var spCatalog = builder.buildCatalog(path);
                    if (spCatalog != null && spCatalog.isNotNull())
                    {
                        return spCatalog;
                    }
                }
                catch (Exception ex)
                {
                    Trace.info("Failed to build a pipeline for: " + path);
                    Trace.info(ex.StackTrace);
                }
            }

            return new PYXCatalog_CSPtr();
        }

        /// <summary>
        /// Cycles through all the PipelineBuilders and tries to create a pipeline for the
        /// specified data set.
        /// </summary>
        /// <param name="pyxDataSet">Describes the data set to be opened.</param>
        /// <param name="config">Configuration settings for pipeline builders</param>
        /// <returns>IProcess_SPtr pointing to the head process of the pipeline that was built or null if no pipeline was built.</returns>
        public static IProcess_SPtr BuildPipeline(PYXDataSet_SPtr pyxDataSet, Dictionary<string,string> config = null)
        {
            foreach (IPipeBuilder_SPtr builder in GetPipelineBuilders())
            {
                try
                {
                    if (config != null)
                    {
                        foreach(var keyValue in config)
                        {
                            builder.setConfig(keyValue.Key, keyValue.Value);
                        }
                    }
                    var spProc = builder.buildPipeline(pyxDataSet);
                    if (spProc != null && spProc.isNotNull())
                    {
                        return spProc;
                    }
                }
                catch (Exception ex)
                {
                    Trace.info("Failed to build a pipeline for: " + pyxDataSet.getUri());
                    Trace.info(ex.StackTrace);
                }
            }

            return null;
        }

        /// <summary>
        /// Imports all the processes which have been created and instantiated for a particular pipeline
        /// into the library. This method achieves it's work by delegating to ProcessBuiltPipelines, and 
        /// defaults that the processes are to be marked as "temporary" to false which means all pipelines
        /// imported by calling this method will be persisted to the library.
        /// </summary>
        ///<seealso cref="ProcessBuiltPipelines"/>
        /// <param name="builtProcesses">A vector containing the processes which were built by the pipebuilder. </param>
        /// <returns>An ImportPruneResult storing information from the import/prune operation.</returns>
        public static ImportPruneResult PruneAndImportPipelines(Vector_IProcess builtProcesses)
        {
            return ProcessBuiltPipelines(builtProcesses, false);
        }

        /// <summary>
        /// Imports all the processes which have been created and instantiated for a paricular pipeline
        /// into the library. This method achieves it's work by delegating to ProcessBuiltPipelines. When calling
        /// this method the caller has a choice as to whether their pipelines are persisted or not.
        /// </summary>
        ///<seealso cref="ProcessBuiltPipelines"/>
        /// <param name="builtProcesses">A vector containing the processes which were built by the pipebuilder.</param>
        /// <param name="procTemp">Flag to indicate whether to persist the pipelines by marking them as not temp. 
        /// To persist the pipelines this flag should be false. For non persistant pipelines to be imported into the
        /// library then this flag should be true.</param>
        /// <returns>An ImportPruneResult storing information from the import/prune operation.</returns>
        public static ImportPruneResult PruneAndImportPipelines(Vector_IProcess builtProcesses, bool procTemp)
        {
            return ProcessBuiltPipelines(builtProcesses, procTemp);
        }

        public static ImportPruneResult PruneAndImportProcess(IProcess_SPtr process, bool procTemp)
        {
            Vector_IProcess vecProcs = new Vector_IProcess();
            vecProcs.Add(process);
            return ProcessBuiltPipelines(vecProcs, procTemp);
        }

        /// <summary>
        /// Imports all the processes which have been created and instantiated for a particular pipeline
        /// into the library. During this import process, the actual list of processes is pruned such 
        /// that only the root process in a pipeline is visible in the library list. 
        /// </summary>
        /// <param name="builtPipelines">A vector containing the processes which were built by the pipebuilder. </param>
        /// <param name="isTemporary">Whether to mark the pipelines as temporary.</param>
        /// <returns>An ImportPruneResult storing information from the import/prune operation.</returns>
        private static ImportPruneResult ProcessBuiltPipelines(Vector_IProcess builtPipelines, bool isTemporary)
        {
            IProcess_SPtr headProcess = null;
            int pipeCount = 0;

            foreach (IProcess_SPtr pipeline in builtPipelines)
            {
                Vector_IProcess vecImportedPipelines = PipeManager.import(pipeline);

#if !NO_LIBRARY
                PipelineRepository.Instance.SetIsTemporary(vecImportedPipelines, isTemporary);
#endif

                // after this statement only the root processes of the imported 
                // pipelines will be present in the vector vecImportedPipelines
                PipeUtils.pruneNonRoots(vecImportedPipelines);

#if !NO_LIBRARY
                foreach (IProcess_SPtr importedPipeline in vecImportedPipelines)
                {
                    PipelineRepository.Instance.SetIsHidden(importedPipeline, false);
                }
#endif

                //If we have more then 1 head process then we don't care what the head is.
                if (vecImportedPipelines.Count == 1)
                {
                    headProcess = vecImportedPipelines[0];
                }

                ++pipeCount;
            }

            return new ImportPruneResult(headProcess, pipeCount);
        }
    }
}
