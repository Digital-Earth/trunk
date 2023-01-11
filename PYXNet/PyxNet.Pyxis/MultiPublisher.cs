/******************************************************************************
MultiPublisher.cs

begin      : 09/14/2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using ApplicationUtility;
#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Exceptions;
using Pyxis.Services.PipelineLibrary.Repositories;
#endif
using Pyxis.Utilities;
using PyxNet.Publishing.Files;
using System;
using System.Collections.Generic;
using System.Linq;

namespace PyxNet.Pyxis
{
    /// <summary> EventArgs for a Published event. </summary>
    public class PublishedEventArgs : EventArgs
    {
        private IProcess_SPtr m_Process;

        /// <summary>The Process.</summary>
        public IProcess_SPtr Process
        {
            get { return m_Process; }
            set { m_Process = value; }
        }

        internal PublishedEventArgs(IProcess_SPtr theProcess)
        {
            m_Process = theProcess;
        }
    }

    /// <summary>
    /// A collection of publishers that can use each other and simplifiy publishing.
    ///
    /// Can publish Coverages, Features, and Files.
    /// </summary>
    public class MultiPublisher
    {
        /// <summary>
        /// The underlying coverage publisher.
        /// </summary>
        private CoveragePublisher m_coveragePublisher;

        /// <summary>
        /// The underlying feature publisher.
        /// </summary>
        private FeaturePublisher m_featurePublisher;

        /// <summary>
        /// The underlying feature publisher.
        /// </summary>
        private ProcessChannelPublisher m_processChannelPublisher;

        public ProcessChannelPublisher ProcessChannelPublisher
        {
            get { return m_processChannelPublisher; }
        }

        /// <summary>
        /// The underlying file publisher.
        /// </summary>
        private FilePublisher m_filePublisher;

        /// <summary>
        /// stack passed in with constructor.
        /// </summary>
        internal PyxNet.Stack Stack
        {
            get;
            private set;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MultiPublisher"/> class.
        /// </summary>
        /// <param name="inStack">The in stack.</param>
        /// <remarks>All other constructors filter through this one.</remarks>
        public MultiPublisher(Stack inStack)
        {
            Stack = inStack;

            m_coveragePublisher = new CoveragePublisher(Stack);
            m_coveragePublisher.Published +=
                delegate(object sender, PublishedEventArgs args)
                {
                    OnPublished(sender, args.Process);
                };

            m_featurePublisher = new FeaturePublisher(Stack);
            m_featurePublisher.Published +=
                delegate(object sender, PublishedEventArgs args)
                {
                    OnPublished(sender, args.Process);
                };

            m_processChannelPublisher = new ProcessChannelPublisher(Stack);
            m_processChannelPublisher.Published +=
                delegate(object sender, PublishedEventArgs args)
                {
                    OnPublished(sender, args.Process);
                };

            m_filePublisher = Stack.FilePublisher;

        }

        /// <summary> Event handler for DisplayUri. </summary>
        public event EventHandler<DisplayUriEventArgs> DisplayUri
        {
            add
            {
                m_coveragePublisher.DisplayUri += value;
                m_featurePublisher.DisplayUri += value;
                m_processChannelPublisher.DisplayUri += value;
            }
            remove
            {
                m_coveragePublisher.DisplayUri -= value;
                m_featurePublisher.DisplayUri -= value;
                m_processChannelPublisher.DisplayUri -= value;
            }
        }

        #region Published Event

        private EventHelper<PublishedEventArgs> m_Published =
            new EventHelper<PublishedEventArgs>();

        /// <summary> Event handler for Published. </summary>
        public event EventHandler<PublishedEventArgs> Published
        {
            add
            {
                m_Published.Add(value);
            }
            remove
            {
                m_Published.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Published event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theProcess"></param>
        public void OnPublished(object sender, IProcess_SPtr theProcess)
        {
            m_Published.Invoke(sender, new PublishedEventArgs(theProcess));
        }

        /// <summary>
        /// Raises the Published event, using the default sender (this).
        /// </summary>
        /// <param name="theProcess"></param>
        public void OnPublished(IProcess_SPtr theProcess)
        {
            OnPublished(this, theProcess);
        }

        #endregion Published Event

        /// <summary>
        /// Publish a coverage or feature over PyxNet making it available to remote nodes.  This will add the
        /// process to the library if it is not already in the library, and marked as published.
        /// This may block for a long time.
        /// </summary>
        /// <param name="procRef">The process which implements the coverage or feature to publish.</param>
        public bool Publish(ProcRef procRef)
        {
            return Publish(procRef, null);
        }

        // TODO: This logic is present in PipelineUtility
        public static IEnumerable<PyxNet.FileTransfer.DownloadContext.DownloadedManifest>
            ExtractDownloadedManifests(IProcess_SPtr process)
        {
            foreach (var child in process.WalkPipelines())
            {
                IUrl_SPtr url = pyxlib.QueryInterface_IUrl(child);
                if ((url != null) && (url.get() != null))
                {
                    PyxNet.FileTransfer.DownloadContext.DownloadedManifest result = null;
                    try
                    {
                        string manifestText = url.getManifest();
                        Manifest manifest = XmlTool.FromXml<Manifest>(manifestText);
                        result = new PyxNet.FileTransfer.DownloadContext.DownloadedManifest(
                            BuildDownloadContext(child), manifest);
                    }
                    catch
                    {
                    }

                    if (result != null)
                    {
                        yield return result;
                    }
                }
            }
        }

        private static PyxNet.FileTransfer.DownloadContext BuildDownloadContext(IProcess_SPtr process)
        {
            string baseDirectory = "";

            IPath_SPtr path = pyxlib.QueryInterface_IPath(process);

            if ((path != null) && (path.get() != null))
            {
                string firstFile = path.getPath();
                baseDirectory = new System.IO.FileInfo(firstFile).DirectoryName;
            }
            return new PyxNet.FileTransfer.DownloadContext(baseDirectory);
        }

        /// <summary>
        /// Helper function to test if the given pipeline has downloadable content.
        /// </summary>
        /// <param name="proc">The proc.</param>
        /// <returns></returns>
        public static bool PipelineHasDownloadableContent(IProcess_SPtr proc)
        {
            return ExtractDownloadedManifests(proc).Any(downloadedManifest => !downloadedManifest.Valid);
        }

        /// <summary>
        /// Publish a coverage or feature over PyxNet making it available to remote nodes.  This will add the
        /// process to the library if it is not already in the library, and mark it as published.
        /// This may block for a long time.
        /// </summary>
        /// <param name="process">The process which implements the coverage or feature to publish.</param>
        /// <param name="licenseServer">The license server to use for this publishing operation.</param>
        private bool Republish(IProcess_SPtr process, Service.ServiceInstance licenseServer)
        {
            var procRef = new ProcRef(process);
            //--
            //-- verify publication certificate
            //--
            if (licenseServer != null && //until we don't have a licenseServer - skip this step as it take 30 sec of timeout
                CheckPublicationCertificate(procRef, licenseServer) == false)
            {
                //--
                //-- TODO:
                //-- if publication certificate does not check out,
                //-- then we to do something here, like NOT publish
                //--
            }

            // Publish it.
            IProcess_SPtr spPublishedProc = m_coveragePublisher.Publish(procRef, licenseServer);
            if (spPublishedProc.get() == null)
            {
                spPublishedProc = m_featurePublisher.Publish(procRef, licenseServer);
            }

            // Check for any PathProc files in the entire pipeline, and publish them.
            int publishedFiles = 0;

            foreach (var fileinfo in spPublishedProc.SupportingFiles())
            {
                m_filePublisher.Publish(fileinfo);
                ++publishedFiles;
            }

            if ((spPublishedProc.get() != null) || (publishedFiles > 0))
            {
                Trace.info(string.Format("Published {0}={1} (including {2} supporting files.)",
                    (spPublishedProc.get() != null) ? spPublishedProc.getProcName() : "unknown",
                    pyxlib.procRefToStr(procRef), publishedFiles));

                return true;
            }

            return false;
        }

        /// <summary>
        /// Unpublish a coverage or feature over PyxNet making it no longer available to remote nodes.  This will not remove the
        /// process from the library, only mark it as unpublished.
        /// This may block for a long time.
        /// </summary>
        /// <param name="procRef">The process which implements the coverage or feature to publish.</param>
        private bool Unpublish(IProcess_SPtr process)
        {
            var procRef = new ProcRef(process);
            bool unPublishedSuccessfully = false;
            if (process.ProvidesOutputType(ICoverage.iid))
            {
                unPublishedSuccessfully = m_coveragePublisher.Unpublish(procRef);
            }
            else if (process.ProvidesOutputType(IFeature.iid))
            {
                unPublishedSuccessfully = m_featurePublisher.Unpublish(procRef);
            }

            // Check for any PathProc files in the entire pipeline, and unpublish them.
            int unpublishedFiles = 0;
            foreach (var fileinfo in process.SupportingFiles())
            {
                unPublishedSuccessfully &= m_filePublisher.Unpublish(fileinfo);
                ++unpublishedFiles;
            }
            if (unPublishedSuccessfully)
            {
#if !NO_LIBRARY
                PipelineRepository.Instance.SetIsPublished(process, false);
#endif
                Trace.info(string.Format("Unpublished {0}={1} (including {2} supporting files.)",
                    (process.get() != null) ? process.getProcName() : "unknown",
                    process.getProcName(), unpublishedFiles));
                StackSingleton.Stack.ForceQueryHashTableUpdate();
                return true;
            }
#if !NO_LIBRARY
            PipelineRepository.Instance.SetIsPublished(process, false);
#endif
            Trace.info(string.Format("Failed to unpublished {0}={1} (including {2} supporting files.)",
                (process.get() != null) ? process.getProcName() : "unknown",
                process.getProcName(), unpublishedFiles));
            StackSingleton.Stack.ForceQueryHashTableUpdate();
            return false;
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.  The process will not
        /// be added to the libary, but it will be marked as "Published" in the database.
        /// This returns immediately.
        /// </summary>
        /// <param name="spProc">The process which implements the coverage to publish.</param>
        public void StartCoveragePublish(IProcess_SPtr spProc)
        {
            // TODO:  Review the download init logic.  We should automatically publish every downloaded dataset _once_.
            if (!IsPublished(spProc))
            {
                System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate(object ignored)
                    {
                        try
                        {
                            Republish(spProc, null);
                        }
                        catch (Exception ex)
                        {
                            System.Diagnostics.Trace.WriteLine(string.Format(
                                "Ignoring exception during Republish.  {0}.",
                                ex.ToString()));
                        }
                    });
            }
        }

        /// <summary>
        /// Determines whether the specified process is published.
        /// </summary>
        /// <param name="spProc">The proc.</param>
        /// <returns>
        /// 	<c>true</c> if the specified process is published; otherwise, <c>false</c>.
        /// </returns>
        private bool IsPublished(IProcess_SPtr spProc)
        {
            return m_coveragePublisher.IsPublished(spProc) || m_featurePublisher.IsPublished(spProc) || m_processChannelPublisher.IsPublished(spProc);
        }

        protected bool DoAllLocalFilesHaveManifest(IProcess_SPtr proc)
        {
            // We need to find all processes that refer to local files
            // and make sure that the manifest is available for them
            // if any have not been calculated then we can not publish.
            Vector_IUnknown vecProcs = new Vector_IUnknown();
            IProcess_SPtr spProc;
            IUrl_SPtr spUrl;

            PipeUtils.findProcsOfType(proc, IUrl.iid, vecProcs);

            for (int index = 0; index < vecProcs.Count; ++index)
            {
                spProc = pyxlib.QueryInterface_IProcess(vecProcs[index]);
                spUrl = pyxlib.QueryInterface_IUrl(spProc.getOutput());
                if (spUrl.isLocalFile() && spUrl.getManifest().Length == 0)
                {
                    // we found a local file process with no manifest yet.
                    return false;
                }
            }

            //second, let find all procs that have embedded resource and check if we can publish their resource also
            Vector_IUnknown vecEmbeddedProcs = new Vector_IUnknown();
            PipeUtils.findProcsOfType(proc, IEmbeddedResourceHolder.iid, vecEmbeddedProcs);
            for (int index = 0; index < vecEmbeddedProcs.Count; ++index)
            {
                IEmbeddedResourceHolder_SPtr spEmbeddedResources = pyxlib.QueryInterface_IEmbeddedResourceHolder(vecEmbeddedProcs[index]);

                for (int i = 0; i < spEmbeddedResources.getEmbeddedResourceCount(); i++)
                {
                    if (!DoAllLocalFilesHaveManifest(spEmbeddedResources.getEmbeddedResource(i)))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// Determines if a given process is able to be published.
        /// </summary>
        /// <param name="procRef">The process to be verified publishable.</param>
        /// <returns>True if the process is able to be published and is not already published.</returns>
        public bool CanPublish(ProcRef procRef)
        {
            try
            {
                IProcess_SPtr proc = PipeManager.getProcess(procRef);

                if (proc == null || proc.isNull())
                {
                    return false;
                }

                // check if all local files have a manifest and that the geometry is valid
                if (
#if !NO_LIBRARY
(PipelineRepository.Instance.TryGetGeometry(procRef) == null) ||
#endif
 (!DoAllLocalFilesHaveManifest(proc)))
                {
                    return false;
                }

                // we can publish IFeature or ICoverage
                return (proc.getSpec().providesOutputType(ICoverage.iid) ||
                        proc.getSpec().providesOutputType(IFeature.iid));
            }
#if !NO_LIBRARY
            catch (ErrorDeserializingGeometryException)
            {
                Trace.error("CanPublish() is returning false because the pipeline's geometry could not be deserialized.");
                return false;
            }
#endif
            catch (System.Exception ex)
            {
                Trace.error("CanPublish() had a critical error. " + ex.ToString());

                // Something is wrong with this pipeline, so we can't publish.
                return false;
            }
        }

        private System.Collections.Concurrent.ConcurrentDictionary<ProcRef, int> m_publishCount = new System.Collections.Concurrent.ConcurrentDictionary<ProcRef, int>();

        public bool Publish(ProcRef procRef, PyxNet.Service.ServiceInstance licenseServer)
        {
            bool result = true;
            var process = PipeManager.getProcess(procRef, false);

            foreach (var geoPacketSource in process.ImmediateGeoPacketSources())
            {
                int publishedCount = m_publishCount.AddOrUpdate(new ProcRef(geoPacketSource), 1, (key, oldValue) => oldValue + 1);
                if (publishedCount == 1)
                {
                    result &= Republish(geoPacketSource, licenseServer);
                }
            }

#if !NO_LIBRARY
            if (result && !PipelineRepository.Instance.GetByProcRef(procRef).IsPublished)
            {
                PipelineRepository.Instance.SetIsPublished(procRef, true);
                PipelineRepository.Instance.CheckPoint();
            }
#endif
            return result;
        }

        public bool Unpublish(ProcRef procRef)
        {
            bool result = true;
            var process = PipeManager.getProcess(procRef, false);

            foreach (var geoPacketSource in process.ImmediateGeoPacketSources())
            {
                int publishCount = m_publishCount.AddOrUpdate(new ProcRef(geoPacketSource), 0, (key, oldValue) => oldValue - 1);
                if (publishCount == 0)
                {
                    result &= Unpublish(geoPacketSource);
                }
            }
#if !NO_LIBRARY
            if (result && PipelineRepository.Instance.GetByProcRef(procRef).IsPublished)
            {
                PipelineRepository.Instance.SetIsPublished(procRef, false);
                PipelineRepository.Instance.CheckPoint();
            }
#endif
            return result;
        }

        //--
        //-- check certificate for publication
        //-- verifies certificate, and possibly extents an expired one.
        //--
        private bool CheckPublicationCertificate(ProcRef procRef, PyxNet.Service.ServiceInstance licenseServer)
        {
            //--
            //-- Build published pipeline fact for the pipeline item data set
            //--
            IProcess_SPtr processPtr = PipeManager.getProcess(procRef);

            if (processPtr.isNull())
            {
                throw new NullReferenceException("procRef " + procRef + "could not be resolved into a process");
            }

            PyxNet.Service.PublishedPipelineFact pipelineFact =
                new PyxNet.Service.PublishedPipelineFact();
            pipelineFact.Id = processPtr.getProcID();
            pipelineFact.Name = processPtr.getProcName();
            pipelineFact.Description = processPtr.getProcDescription();
            //pipelineFact.Metadata = this.m_xmlMetadata.ToString();
            pipelineFact.Certificate = null;
            pipelineFact.PipelineIdentityXML = processPtr.getIdentity();

            //--
            //-- read pipeline definition
            //--
            pipelineFact.PipelineDefinitionPPL = PipeManager.writePipelineToNewString(processPtr);

            //--
            //-- Find certificate, search stack
            //--
            PyxNet.Service.CertificateFinder finder = new PyxNet.Service.CertificateFinder(Stack, pipelineFact);
            PyxNet.Service.ICertifiableFact pipelinePublicationFact = finder.Find();

            PyxNet.Service.Certificate certificate = null;
            if (pipelinePublicationFact == null)
            {
                //--
                //-- no cerficate, request a new one
                //--

                //--
                //-- must have a license server
                //--
                if (licenseServer != null)
                {
                    SynchronizationEvent permissionGrantedTimer = new SynchronizationEvent(TimeSpan.FromSeconds(30));

                    //--
                    //-- build certificate request, with permission fact
                    //--
                    PyxNet.Service.CertificateRequester requester =
                        new PyxNet.Service.CertificateRequester(Stack, pipelineFact);

                    requester.DisplayUri +=
                        delegate(object sender, PyxNet.DisplayUriEventArgs a)
                        {
                            //Log.Error("UsageReports:FindLicenseServer: DisplayUri - should never happen");
                            Trace.info(string.Format("DisplayUri: {0}", a.Uri));
                        };

                    requester.CertificateReceived +=
                        delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                        {
                            //--
                            //-- Get the certificate.
                            //--
                            certificate = c.Certificate;
                            System.Diagnostics.Debug.Assert(certificate != null);

                            //--
                            //-- Add it to the stack's certificate repository.
                            //--
                            Stack.CertificateRepository.Add(certificate);
                        };

                    requester.PermissionGranted +=
                        delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                        {
                            //Log.Info("UsageReports:FindLicenseServer: PermissionGranted");
                            permissionGrantedTimer.Pulse();
                        };

                    //--
                    //-- send request to license server
                    //--
                    requester.Start(licenseServer, TimeSpan.FromSeconds(15));

                    //--
                    //-- Wait until we get one.
                    //--
                    permissionGrantedTimer.Wait();
                }
            }
            else
            {
                //--
                //-- certificate already exists!
                //--
                certificate = pipelinePublicationFact.Certificate;
            }

            //--
            //-- success if we have a certificate
            //--
            return certificate != null;
        }

        public void ImportRemoteProcess(Guid dataSetId, int version, string xmlPipelineDefinition)
        {
            // get the .ppl from the coverageRequest and import the .ppl into the library.
            Vector_IProcess vecProcesses = PipeManager.importStr(xmlPipelineDefinition);
#if !NO_LIBRARY
            // set all the processes as not temporary
            PipelineRepository.Instance.SetIsTemporary(vecProcesses, false);

            // set the top of the process chain as NOT hidden
            ProcRef procRef = new ProcRef(pyxlib.strToGuid(dataSetId.ToString()), version);

            PipelineRepository.Instance.SetIsHidden(procRef, false);
            PipelineRepository.Instance.SetIsPublished(procRef, true);
#endif
        }

        public void StopPublishing()
        {
            m_coveragePublisher.StopPublishing();
            m_featurePublisher.StopPublishing();
            m_processChannelPublisher.StopPublishing();
            m_filePublisher.StopPublishing();
        }
    }
}