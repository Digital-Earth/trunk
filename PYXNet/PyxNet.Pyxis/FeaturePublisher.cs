/******************************************************************************
FeaturePublisher.cs

begin      : 26/08/2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Linq;
using Pyxis.Utilities;

using PyxNet;
using PyxNet.DataHandling;

namespace PyxNet.Pyxis
{
    /// <summary>
    /// Summary description for FeaturePublisher
    /// </summary>
    public class FeaturePublisher
    {
        // A TraceTool that keeps a log of where we got all our pieces from.
        private NumberedTraceTool<FeaturePublisher> m_tracer
            = new NumberedTraceTool<FeaturePublisher>(TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// A local class to keep track of the things we are publishing.
        /// We keep a list of these objects, one per item we publish.
        /// </summary>
        public class PublishedFeatureInfo : Publishing.Publisher.IPublishedItemInfo
        {
            #region Fields And Properties

            private readonly IProcess_SPtr m_spProcess;

            /// <summary>
            /// The process we are publishing.
            /// </summary>
            public IProcess_SPtr Process
            {
                get { return m_spProcess; }
            }

            private readonly IFeature_SPtr m_spFeature;

            /// <summary>
            /// The process we are publishing as a feature.
            /// </summary>
            public IFeature_SPtr Feature
            {
                get { return m_spFeature; }
            }

            private string m_description;

            private string m_lowerCaseDescription;

            private List<string> m_keywords = null;

            /// <summary>
            /// Gets the keywords (a list of words that match this item).
            /// </summary>
            /// <value>The keywords.</value>
            private List<string> Keywords
            {
                get
                {
                    if (m_keywords == null)
                    {
                        // We could lock on this, but it doesn't matter that much.  
                        List<string> keywords = new List<string>();
                        keywords.AddRange(m_lowerCaseDescription.Split(null));
                        if (m_spProcess != null)
                        {
                            keywords.AddRange(m_spProcess.getProcName().ToLower().Split(null));
                        }
                        m_keywords = keywords;
                    }
                    return m_keywords;
                }
            }

            private string m_procGuidString;

            public string Description
            {
                get { return m_description; }
                set
                {
                    m_description = value;
                    m_lowerCaseDescription = m_description.ToLower();
                    m_keywords = null;
                }
            }

            private PyxNet.Service.Certificate m_certificate;

            public PyxNet.Service.Certificate Certificate
            {
                get { return m_certificate; }
                set { m_certificate = value; }
            }

            #endregion

            public ProcRef ProcRef
            {
                get
                {
                    return new ProcRef(m_spProcess);
                }
            }
            /// <summary>
            /// Constructor.
            /// </summary>
            /// <param name="spProc">A handle to the process.</param>
            public PublishedFeatureInfo(IProcess_SPtr spProc)
            {
                if (null == spProc)
                {
                    throw new ArgumentNullException("spProc");
                }

                m_spFeature = pyxlib.QueryInterface_IFeature(spProc.getOutput());
                if (m_spFeature.get() == null)
                {
                    // we did not get a coverage, so throw.
                    throw new ArgumentException("The process is not a feature.", "spProc");
                }

                m_spProcess = spProc;
                m_procGuidString = pyxlib.guidToStr(m_spProcess.getProcID());
                Description = m_spProcess.getProcDescription();
            }

            #region Equality

            public override bool Equals(object obj)
            {
                return Equals(obj as PublishedFeatureInfo);
            }

            public bool Equals(PublishedFeatureInfo info)
            {
                if (null == info)
                {
                    return false;
                }

                if (info.m_spProcess.getProcID() == m_spProcess.getProcID() &&
                    info.m_spProcess.getProcVersion() == m_spProcess.getProcVersion())
                {
                    return true;
                }

                return false;
            }

            public override int GetHashCode()
            {
                throw new MissingMethodException("Method not implemented.");
            }

            #endregion

            /// <summary>
            /// Cache the CoverageInfo for performance.
            /// </summary>
            private FeatureDefinitionMessage m_featureDef = null;

            /// <summary>
            /// Helper property to get a CoverageRequestMessage that is initialized to
            /// have all the information about the published coverage.
            /// </summary>
            /// <returns></returns>
            public FeatureDefinitionMessage FeatureInfo
            {
                get
                {
                    if (m_featureDef == null)
                    {
                        m_featureDef = new FeatureDefinitionMessage();
                        m_featureDef.ProcRef = new ProcRef(Process);
                        m_featureDef.PipelineDefinition = PipeManager.writePipelineToNewString(Process);
                        m_featureDef.Geometry = Feature.getGeometry();
                    }
                    return m_featureDef;
                }
            }

            /// <summary>
            /// Check to see if a query string matches the published coverage in any way.
            /// </summary>
            /// <param name="testAgainst">The string to test.</param>
            /// <returns>True if the string matches.</returns>
            public bool Matches(string testAgainst)
            {
                string lowerCaseTestAgainst = testAgainst.ToLower();
                string[] wordsToMatch = lowerCaseTestAgainst.Split(null);

                bool matched = true;
                foreach (string word in wordsToMatch)
                {
                    if (!Keywords.Contains(word))
                    {
                        matched = false;
                        break;
                    }
                }
                if (matched)
                {
                    return true;
                }

                // check to see if the Guid matches in string format.
                return m_procGuidString.Equals(testAgainst);
            }

            #region IPublishedItemInfo Members

            IEnumerable<string> PyxNet.Publishing.Publisher.IPublishedItemInfo.Keywords
            {
                get
                {
                    // add in the guid of the process.
                    yield return m_procGuidString;

                    // add in all the parts of the description
                    foreach (string keyword in this.Keywords)
                    {
                        yield return keyword;
                    }

                    // TODO: scan for meta data that could be used to search on
                    // and add it to the local query hash table.
                }
            }

            public QueryResult Matches(Query query, Stack stack)
            {
                if (Matches(query.Contents))
                {
                    // we found a match
                    NodeInfo connectedNodeInfo = null;
                    foreach (NodeInfo nodeInfo in stack.KnownHubList.ConnectedHubs)
                    {
                        connectedNodeInfo = nodeInfo;
                        break;
                    }
                    QueryResult queryResult =
                        new QueryResult(query.Guid, query.OriginNode, stack.NodeInfo, connectedNodeInfo);
                    queryResult.MatchingContents = Process.getProcName();
                    queryResult.MatchingDescription = Description;
                    queryResult.MatchingDataSetID.Guid = new Guid(pyxlib.guidToStr(Process.getProcID()));

                    // add in extra info here the feature definition.
                    queryResult.ExtraInfo = FeatureInfo.ToMessage();

                    return queryResult;
                }
                return null;
            }

            #endregion
        }

        #region Fields and Properties

        /// <summary>
        /// This list of features that we are publishing.
        /// </summary>
        private readonly DynamicList<PublishedFeatureInfo> m_publishedFeatures =
            new DynamicList<PublishedFeatureInfo>();

        /// <summary>
        /// The stack that we are using to publish/monitor.
        /// </summary>
        private readonly Stack m_stack;

        #endregion

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack that you want to publish on.</param>
        public FeaturePublisher(Stack stack)
        {
            m_stack = stack;
        }

        #region DisplayUri Event

        /// <summary> Event handler for DisplayUri. </summary>
        public event EventHandler<DisplayUriEventArgs> DisplayUri
        {
            add
            {
                m_DisplayUri.Add(value);
            }
            remove
            {
                m_DisplayUri.Remove(value);
            }
        }
        private EventHelper<DisplayUriEventArgs> m_DisplayUri = new EventHelper<DisplayUriEventArgs>();

        /// <summary>
        /// Raises the DisplayUri event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theUri"></param>
        public void OnDisplayUri(object sender, Uri theUri)
        {
            m_DisplayUri.Invoke(sender, new DisplayUriEventArgs(theUri));
        }
        #endregion DisplayUri Event

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
        /// Publish all the data files referenced by the pipeline through the
        /// stack's publishing system.
        /// </summary>
        /// <param name="pub"></param>
        /// <param name="licenseServer"></param>
        private void PublishFiles(PublishedFeatureInfo pub, PyxNet.Service.ServiceInstance licenseServer)
        {
            // Handle all the Path and Url procs.
            Vector_IUnknown vecProcs = new Vector_IUnknown();
            IProcess_SPtr spProc;
            IPath_SPtr spPath;
            IUrl_SPtr spUrl;
            PipeUtils.findProcsOfType(pub.Process, IUrl.iid, vecProcs);
            for (int index = 0; index < vecProcs.Count; ++index)
            {
                spProc = pyxlib.QueryInterface_IProcess(vecProcs[index]);
                spPath = pyxlib.QueryInterface_IPath(spProc.getOutput());
                if (spPath.get() != null)
                {
                    // it is a path proc so publish all the files that are needed.
                }
                else
                {
                    spUrl = pyxlib.QueryInterface_IUrl(spProc.getOutput());
                    if (spUrl.get() == null)
                    {
                        // we should never get here.
                        // TODO: what do we throw here?
                    }
                    try
                    {
                        System.Uri uri = new Uri(spUrl.getUrl());
                        if (uri.IsFile)
                        {
                            try
                            {
                                string filename = uri.LocalPath;
                            }
                            catch (System.InvalidOperationException)
                            {
                                // no local file
                            }
                        }
                    }
                    catch (System.UriFormatException)
                    {
                        // bad uri, just ignore it
                    }
                }
            }

        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="processHandle">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(ProcRef processHandle, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == processHandle)
            {
                return new IProcess_SPtr();
            }

            // Get the process through the process resolver.
            IProcess_SPtr spProcess = PipeManager.getProcess(processHandle);

            return Publish(spProcess, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="spProc">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(IProcess_SPtr spProc, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == spProc)
            {
                return new IProcess_SPtr();
            }

            PublishedFeatureInfo pub;
            try
            {
                pub = new PublishedFeatureInfo(spProc);
            }
            catch (ArgumentException)
            {
                // It's not a feature process.
                return new IProcess_SPtr();
            }

            return Publish(pub, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="pub">The info about the coverage to publish.</param>
        /// <param name="licenseServer">The licence server to use to negotiate a licence.
        /// Can be null if no negotiation is wanted.</param>
        /// <returns>The process smart pointer, which points to null if unsuccessful.</returns>
        private IProcess_SPtr Publish(
            PublishedFeatureInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            // Return if it's already published.
            if (m_publishedFeatures.Contains(pub))
            {
                return pub.Process;
            }

            // If we made it here, we are trying publishing a new coverage (to us).
            if (GetExistingPublicationCertificate(pub) ||
                (RequestPublicationCertificate(pub, licenseServer)))
            {
                // we found a local certificate to publish, so we are OK to publish.
                if (m_publishedFeatures.Add(pub, false))
                {
                    m_stack.Publisher.PublishItem(pub);
                    OnPublished(pub.Process);
                }

                // TODO: verify the return value here.
                return pub.Process;
            }

            return new IProcess_SPtr();
        }

        /// <summary>
        /// This function will look for the publication certificate, first in the local
        /// stack's repository, then search for it on PyxNet.  
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool GetExistingPublicationCertificate(
            PublishedFeatureInfo pub)
        {
#if LICENSE_SERVER
            Stack stack = m_stack;

            // Build the fact that we want to find.
            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.DataInfo.DataSetID.Guid);
            PyxNet.Service.ResourceDefinitionFact newFact =
                new PyxNet.Service.ResourceDefinitionFact(
                    pub.CoverageInfo.PipelineDefinition);
            newFact.ResourceId = coverageResourceId;

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ResourceDefinitionFact resourceDefinitionFact =
                finder.Find() as PyxNet.Service.ResourceDefinitionFact;

            // Return true if the certificate is not null.
            if (resourceDefinitionFact != null)
            {
                pub.Certificate = GetExistingResourcePermisson(resourceDefinitionFact.ResourceId);
            }
            return pub.Certificate != null;
#else
            return true;
#endif
        }

        private PyxNet.Service.Certificate GetExistingResourcePermisson(
            PyxNet.Service.ResourceId resourceId)
        {
            Stack stack = m_stack;

            // Build the fact that we want to find.
            PyxNet.Service.ResourcePermissionFact newFact =
                new PyxNet.Service.ResourcePermissionFact(resourceId,
                    stack.NodeInfo.NodeId);

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ICertifiableFact resourcePermissionFact =
                finder.Find();

            // Return it.
            if (resourcePermissionFact != null)
            {
                return resourcePermissionFact.Certificate;
            }
            return null;
        }

        /// <summary>
        /// This function will request the publication certificate from a server.
        /// It is a blocking call.
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool RequestPublicationCertificate(
            PublishedFeatureInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            // TODO: Deal with equivalency.

            Stack stack = m_stack;

            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.Process.getProcID());

            PyxNet.Service.ResourceDefinitionFact newDefinitionFact =
                new PyxNet.Service.ResourceDefinitionFact(pub.FeatureInfo.PipelineDefinition);
            newDefinitionFact.ResourceId = coverageResourceId;

            PyxNet.Service.ResourcePermissionFact newPermissionFact =
                new PyxNet.Service.ResourcePermissionFact(coverageResourceId,
                    stack.NodeInfo.NodeId);

            // Search for a certificate locally.
            foreach (PyxNet.Service.ResourceDefinitionFact fact in
                m_stack.CertificateRepository.GetMatchingFacts(
                    newDefinitionFact.UniqueKeyword, newDefinitionFact.GetType()))
            {
                System.Diagnostics.Debug.Assert(fact != null);
                pub.Certificate = GetExistingResourcePermisson(fact.ResourceId);
                return true;
            }

            SynchronizationEvent permissionGrantedTimer =
                new SynchronizationEvent();

            // If here, we don't have a certificate.  Request one.
            PyxNet.Service.CertificateRequester requester =
                new PyxNet.Service.CertificateRequester(stack,
                    newDefinitionFact, newPermissionFact);
            requester.DisplayUri +=
                delegate(object sender, PyxNet.DisplayUriEventArgs a)
                {
                    OnDisplayUri(this, a.Uri);
                };
            requester.CertificateReceived +=
                delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                {
                    // Get the certificate.
                    PyxNet.Service.Certificate certificate = c.Certificate;
                    System.Diagnostics.Debug.Assert(certificate != null);

                    // Add it to the stack's certificate repository.
                    stack.CertificateRepository.Add(certificate);

                    // Set it in the published coverage info.
                    pub.Certificate = certificate;
                };
            requester.PermissionGranted +=
                delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                {
                    permissionGrantedTimer.Pulse();
                };
            requester.Start(licenseServer, TimeSpan.FromSeconds(30));

            // Wait until we get one.
            permissionGrantedTimer.Wait();

            return true;
        }

        /// <summary>
        /// Stop publishing all coverages, and stop responding to messages.
        /// </summary>
        public void StopPublishing()
        {
            // TODO: how do we do this with the stacks publisher??
            m_publishedFeatures.Clear();
        }

        internal bool IsPublished(IProcess_SPtr spProc)
        {
            foreach (var item in m_publishedFeatures)
            {
                if (pyxlib.isEqualProcRef(item.ProcRef, new ProcRef(spProc)))
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Unpublish a feature from PyxNet making it no longer available to remote nodes.
        /// </summary>
        /// <param name="unPub">The info about the feature to unpublish.</param>

        public bool Unpublish(ProcRef procRef)
        {
            bool canUnpublish = false;
            PublishedFeatureInfo featureInfo = null;
            lock (m_publishedFeatures)
            {
                featureInfo = m_publishedFeatures.FirstOrDefault(x => x.ProcRef == procRef);
                if (featureInfo != null && m_publishedFeatures.Remove(featureInfo))
                {
                    canUnpublish = true;
                }
            }
            if (canUnpublish)
            {
                m_stack.Publisher.UnpublishItem(featureInfo);
            }
            return canUnpublish;
        }
    }
}

// see PyxNet.Pyxis.Text.cs for unit tests.