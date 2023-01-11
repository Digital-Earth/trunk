/******************************************************************************
DefaultDataManager.cs

begin      : June 22, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Domain;
using Pyxis.Services.PipelineLibrary.Repositories;
#endif

namespace ApplicationUtility
{
    /// <summary>
    /// Is responsible for the loading of Default Data.
    /// </summary>
    public static class DefaultDataManager
    {
        static DefaultDataManager()
        {
            InitializeDefaultData();
        }

        static public void InitializeDefaultData()
        {
            DefaultDataCollection = new Pyxis.Utilities.DynamicList<IProcess_SPtr>();

            System.Threading.ThreadPool.QueueUserWorkItem(
                delegate(Object unused)
                {
                    string pipelinePath =
                        AppServices.getApplicationPath() +
                        System.IO.Path.DirectorySeparatorChar + "default_data.ppl";
                    Trace.info(
                        string.Format("The path to default data ppl file is '{0}'.", pipelinePath));

                    try
                    {
                        //first - import the pipeline into the library... then start to play with it...
                        PipeManager.import(pipelinePath);

                        // parse the file and insert into the library
                        IProcess_SPtr pipeline =
                            PipeManager.readPipelineFromFile(pipelinePath);

                        // verify a pipeline was read from the file
                        if (pipeline.get() == null)
                        {
                            Trace.error("Unable to find any valid pipelines in the default data file: " +
                                pipelinePath);
                        }
                        else
                        {
                            // got a pipeline so initialize it
                            pipeline.initProc(true);

                            if (pipeline.getInitState() != IProcess.eInitStatus.knInitialized)
                            {
                                // TODO: should probably do something better here like 
                                // add all processes that did initialize.
                                Trace.error(
                                    "Default data process collection is not " +
                                    "initialized, no default data loaded:\n" +
                                    PipeUtils.pipeToTree(pipeline));
                            }
                            else
                            {
#if DEBUG
                        // TODO: this trace call takes 1.279 seconds on a fast machine -- find out why
                        Trace.info("Initializing default data with the following pipeline: \n" +
                            PipeUtils.pipeToTree(pipeline));
#endif


                                Trace.info("Importing default data pipeline into the library...");

                                Vector_IProcess vecProcesses = PipeManager.import(pipeline);
                                #if !NO_LIBRARY
                                foreach (IProcess_SPtr proc in vecProcesses)
                                {
                                    // Import as remote and not temporary but leave hidden
                                    PipelineRepository.Instance.SetIsPublished(proc, true);
                                    PipelineRepository.Instance.SetIsTemporary(proc, false);
                                }
                                #endif

                                // create a new process list to return
                                IProcessCollection_SPtr procCollection =
                                    pyxlib.QueryInterface_IProcessCollection(pipeline.getOutput());
                                if (procCollection.get() == null)
                                {
                                    Trace.error("Defined default data set is not a 'Process Collection' as expected!");
                                }
                                else if (procCollection.getProcessList().get() == null)
                                {
                                    Trace.error("Default data process collection has a null process list!");
                                }
                                else
                                {
                                    // unhide the individual data sources in the collection
                                    Vector_IProcess procVector = new Vector_IProcess();
                                    procCollection.getProcessList().getProcesses(procVector);
                                    foreach (IProcess_SPtr rootProc in procVector)
                                    {
                                        #if !NO_LIBRARY
                                        PipelineRepository.Instance.SetIsHidden(rootProc, false);
                                        #endif
                                        DefaultDataCollection.Add(new IProcess_SPtr(rootProc));
                                    }
                                }
                            }
                        }
                    }
                    catch
                    {
                        Trace.error("An unexpected error occurred while initializing default data sets!");
                    }

                    // This is a bit of a hack to allow the app to initialize synchronously.  The UI jumps a lot with asynch init.
                    Initialized = true;
                });
        }

        /// <summary>
        /// Gets or sets a value indicating whether this <see cref="DefaultDataManager"/> is initialized.
        /// </summary>
        /// <value><c>true</c> if initialized; otherwise, <c>false</c>.</value>
        static public bool Initialized { get; set; }

        /// <summary>
        /// Gets or sets the default data collection.  Note that this is a dynamic collection, so you
        /// can watch it for changes.
        /// </summary>
        /// <value>The default data collection.</value>
        static public Pyxis.Utilities.DynamicList<IProcess_SPtr> DefaultDataCollection
        {
            get;
            set;
        }
    }
}
