/******************************************************************************
Republisher.cs

begin      : 09/14/2009 
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Domain;
using Pyxis.Services.PipelineLibrary.Repositories;
#endif
using Pyxis.Utilities;

namespace PyxNet.Pyxis
{
    public class Republisher
    {
        public class PipelinePublishedEventArgs : EventArgs
        {
            public ProcRef ProcRef
            {
                get
                {
                    return m_procRef;
                }
            }
            private ProcRef m_procRef;

            public PipelinePublishedEventArgs(ProcRef procRef)
            {
                m_procRef = procRef;
            }
        }

        /// <summary>
        /// Event which is fired when some data has been recieved.
        /// </summary>
        public event EventHandler<PipelinePublishedEventArgs> OnPipelinePublished
        {
            add
            {
                m_onPipelinePublished.Add(value);
            }
            remove
            {
                m_onPipelinePublished.Remove(value);
            }
        }
        private EventHelper<PipelinePublishedEventArgs> m_onPipelinePublished = new EventHelper<PipelinePublishedEventArgs>();

        /// <summary>
        /// This is the delay to allow PyxNet to get initialized before we republish
        /// the features and coverages at application startup.  Currently 30 seconds.
        /// </summary>
        private static readonly TimeSpan WaitBeforeRepublishing = TimeSpan.FromSeconds(30);

        private Stack Stack { get; set; }

        private MultiPublisher m_publisher;

        public Republisher(MultiPublisher publisher)
        {
            m_publisher = publisher;
            Stack = StackSingleton.Stack;
        }

        /// <summary>
        /// Publish all processes that were published during a prior run.
        /// This will return quickly, and the actual publishing will
        /// be done on a background thread after a SecondsToWaitBeforeRepublishing
        /// second pause.
        /// </summary>
        public void PublishAllPipelines()
        {
            #if !NO_LIBRARY
#if USE_PYXNET_LS
            Console.WriteLine("Finding licenseServer over PyxNet");

            // Find a license server.
            PyxNet.Service.ServiceFinder finder = new PyxNet.Service.ServiceFinder(this.Stack);

            PyxNet.Service.ServiceInstance licenseServer = finder.FindService(
                Service.CertificateServer.CertificateAuthorityServiceId,
                TimeSpan.FromSeconds(15));
#else
            PyxNet.Service.ServiceInstance licenseServer = null;
#endif

            Console.WriteLine("Republishing all published pipelines");

            IList<Pipeline> pipelines = PipelineRepository.Instance.GetAllPublishedPipelines();

            int i = 0;
            foreach (Pipeline pipeline in pipelines)
            {
                Pipeline pipelineToPublish = pipeline;
                ProcRef procRef = pipelineToPublish.ProcRef;

                Console.Write((++i) + " of " + pipelines.Count + " \t " + pipeline.Name);
                var process = PipeManager.getProcess(procRef);

                if (process.isNull())
                {
                    Console.WriteLine("Pipeline with ProcRef=" + procRef + " was requested to be published but it can not be resolved.");
                    continue;
                }


                try
                {
                    Console.Write("\t Trying to publish...");
                    if (m_publisher.Publish(procRef, licenseServer))
                    {
                        Console.WriteLine("\tPublished!");
                        m_onPipelinePublished.Invoke(this, new PipelinePublishedEventArgs(procRef));

                        continue;
                    }
                    Console.WriteLine("\tPublish failed!");
                }
                catch (Exception ex)
                {
                    Console.WriteLine("\tError republishing pipeline {0}[{1}.  {2}.",
                        procRef.getProcID(), procRef.getProcVersion(), ex.ToString());
                }
            }
            #endif
        }

        public void StartPublish()
        {
            #if !NO_LIBRARY
            System.Threading.Thread thread = new System.Threading.Thread(
                delegate()
                {
                    // give Pyxnet a chance to initialize, settle down, and map NAT ports.
                    System.Threading.Thread.Sleep(WaitBeforeRepublishing);

                    // Find a license server.
                    PyxNet.Service.ServiceFinder finder = new PyxNet.Service.ServiceFinder(this.Stack);
                    PyxNet.Service.ServiceInstance licenseServer = finder.FindService(
                        Service.CertificateServer.CertificateAuthorityServiceId,
                        TimeSpan.FromSeconds(15));

                    IList<Pipeline> pipelines = PipelineRepository.Instance.GetAllPublishedPipelines();
                    foreach (Pipeline pipeline in pipelines)
                    {
                        System.Threading.Thread publishThread = new System.Threading.Thread(
                            delegate(object pipelineObject)
                            {
                                Pipeline pipelineToPublish = pipelineObject as Pipeline;
                                ProcRef procRef = pipelineToPublish.ProcRef;

                                Console.WriteLine("Calling get process for:" + pipeline.Name);
                                var process = PipeManager.getProcess(procRef);
                                Console.WriteLine("GetProcess finished for:" + process.getProcName());

                                if (process.isNull())
                                {
                                    Trace.error("Pipeline with ProcRef=" + procRef + " was requested to be published but it can not be resolved.");
                                    return;
                                }

                                try
                                {
                                    while (true)
                                    {
                                        Console.WriteLine("Trying to publish:" + pipeline.Name);
                                        if (m_publisher.Publish(procRef, licenseServer))
                                        {
                                            Console.WriteLine("Published :" + pipeline.Name);
                                            m_onPipelinePublished.Invoke(this, new PipelinePublishedEventArgs(procRef));

                                            return;
                                        }
                                        Console.WriteLine("Publish failed:" + pipeline.Name);
                                        // Wait for a moment before retrying.
                                        System.Threading.Thread.Sleep(TimeSpan.FromMinutes(1));

                                    }
                                }
                                catch (Exception ex)
                                {
                                    Stack.Tracer.WriteLine("Error re-publishing pipeline {0}[{1}.  {2}.",
                                        procRef.getProcID(), procRef.getProcVersion(), ex.ToString());
                                }
                            });

                        //we don't want to use the enumareted veriable - not safe.
                        var pipelineCopy = pipeline;

                        publishThread.IsBackground = true;
                        publishThread.Start(pipelineCopy);
                    }
                });
            thread.Name = "Republisher";
            thread.IsBackground = true;
            thread.Start();
            #endif
        }
    }
}