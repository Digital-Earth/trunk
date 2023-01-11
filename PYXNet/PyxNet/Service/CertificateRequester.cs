using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace PyxNet
{
    /// <summary> EventArgs for a DisplayUri event. </summary>    
    public class DisplayUriEventArgs : EventArgs
    {
        private Uri m_Uri;

        /// <summary>The Uri.</summary>
        public Uri Uri
        {
            get { return m_Uri; }
            set { m_Uri = value; }
        }

        public DisplayUriEventArgs(Uri theUri)
        {
            m_Uri = theUri;
        }
    }

    namespace Service
    {
        /// <summary>
        /// </summary>    
        public class CertificateRequester : IDisposable
        {
            #region Fields and Properties
            private Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

            [System.Xml.Serialization.XmlRoot(Namespace = "urn:PyxNet.Service.CertificateRequester")]
            public enum State
            {
                Initializing,
                SearchingForCertificateServer,
                FoundCertificateServer,
                SentLicenseRequest,
                SentLicenseRequestToSelf,
                SentLicenseRequestIndirectly,
                ReceivedFalseResponse,
                ReceivedCertificate
            }
            public Pyxis.Utilities.ObservableObject<State> Status =
                new Pyxis.Utilities.ObservableObject<State>(State.Initializing);

            /// <summary>
            /// The local stack to use.
            /// </summary>
            private readonly Stack m_stack;

            /// <summary>
            /// Gets the stack.
            /// </summary>
            public Stack Stack
            {
                get { return m_stack; }
            }

            private ServiceInstance m_licenseServer;

            public ServiceInstance LicenseServer
            {
                get { return m_licenseServer; }
                set { m_licenseServer = value; }
            }

            private NodeInfo m_certificateServerNodeInfo;

            public NodeInfo CertificateServerNodeInfo
            {
                get { return m_certificateServerNodeInfo; }
                set { m_certificateServerNodeInfo = value; }
            }

            private StackConnection m_certificateConnection;

            public StackConnection CertificateConnection
            {
                get { return m_certificateConnection; }
                set { m_certificateConnection = value; }
            }

            private FactList m_factList = new FactList();

            public FactList FactList
            {
                get { return m_factList; }
            }

            /// <summary>
            /// The identity of the request.  Used to identify the request in
            /// asynchronous responses.
            /// </summary>
            private Guid m_id = Guid.NewGuid();

            /// <summary>
            /// Gets the identity of this request.  Used to identify the request in
            /// asynchronous responses.
            /// </summary>
            public Guid Id
            {
                get { return m_id; }
                set { m_id = value; }
            }
            #endregion

            #region Constructors

            /// <summary>
            /// Construct an instance.
            /// </summary>
            /// <param name="stack">The stack to use.</param>
            /// <param name="requestedServiceId">The requested service id.</param>
            public CertificateRequester(
                Stack stack,
                params ICertifiableFact[] facts)
            {
                m_stack = stack;

                // Handle CertificateRequestResponse message arriving at the stack.
                this.Stack.SecureStreamRegisterHandler(
                    CertificateRequestResponse.MessageID, HandleCertificateRequestResponse);

                foreach (ICertifiableFact fact in facts)
                {
                    m_factList.Add(fact);
                }
            }

            #endregion

            #region Dispose

            /// <summary>
            /// Track whether Dispose has been called.
            /// </summary>
            private bool m_disposed = false;

            /// <summary>
            /// Implement IDisposable.
            /// </summary>
            /// <remarks>
            /// Do not make this method virtual.
            /// A derived class should not be able to override this method. 
            /// </remarks>
            public void Dispose()
            {
                Dispose(true);

                // This object will be cleaned up by the Dispose method.
                // Therefore, you should call GC.SupressFinalize to
                // take this object off the finalization queue 
                // and prevent finalization code for this object
                // from executing a second time.
                GC.SuppressFinalize(this);
            }

            /// <summary>
            /// Dispose(bool disposing) executes in two distinct scenarios, 
            /// indicated by the "disposing" argument.
            /// </summary>
            /// <param name="disposing">
            /// If disposing equals true, the method has been called directly
            /// or indirectly by a user's code. Managed and unmanaged resources
            /// can be disposed.
            /// If disposing equals false, the method has been called by the 
            /// runtime from inside the finalizer and you should not reference 
            /// other objects. Only unmanaged resources can be disposed.
            /// </param>
            private void Dispose(bool disposing)
            {
                // Check to see if Dispose has already been called.
                if (!m_disposed)
                {
                    // If disposing equals true, dispose all managed 
                    // and unmanaged resources.
                    if (disposing)
                    {
                        Close();
                    }

                    // Call the appropriate methods to clean up 
                    // unmanaged resources here.
                    // If disposing is false, 
                    // only the following code is executed.
                }
                m_disposed = true;
            }

            /// <summary>
            /// Closes this instance.  
            /// </summary>
            /// <remarks>Also called by the Dispose routine, or the destructor.</remarks>
            public void Close()
            {
                this.Stack.SecureStreamUnregisterHandler(
                    CertificateRequestResponse.MessageID, HandleCertificateRequestResponse);
                OnRequestClosed(RequestClosedEventArgs.Empty);
            }

            /// <summary>
            /// The finalization code.
            /// </summary>
            /// <remarks>
            /// Use C# destructor syntax for finalization code.
            /// This destructor will run only if the Dispose method 
            /// does not get called.
            /// It gives your base class the opportunity to finalize.
            /// Do not provide destructors in types derived from this class.
            /// </remarks>
            ~CertificateRequester()
            {
                // Do not re-create Dispose clean-up code here.
                // Calling Dispose(false) is optimal in terms of
                // readability and maintainability.
                Dispose(false);
            }

            #endregion

            #region Events

            #region ResponseReceived Event

            /// <summary>
            /// Class which wraps a CertificateRequestResponse object, and will be passed as the second argument to a response handler.
            /// </summary>
            public class ResponseReceivedEventArgs : EventArgs
            {
                private readonly CertificateRequestResponse m_response;

                public CertificateRequestResponse Response
                {
                    get
                    {
                        return m_response;
                    }
                }

                public ResponseReceivedEventArgs(CertificateRequestResponse response)
                {
                    m_response = response;
                }
            }

            /// <summary>
            /// Event which is fired when a response is received.
            /// </summary>
            public event EventHandler<ResponseReceivedEventArgs> ResponseReceived
            {
                add
                {
                    m_ResponseReceived.Add(value);
                }
                remove
                {
                    m_ResponseReceived.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs> m_ResponseReceived = new Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs>();

            /// <summary>
            /// Method to safely raise the event.
            /// </summary>
            private void OnResponseReceived(CertificateRequestResponse response)
            {
                m_ResponseReceived.Invoke(this, new ResponseReceivedEventArgs(response));
            }

            #endregion

            #region PermissionGranted Event

            /// <summary>
            /// Event which is fired when permission is granted.
            /// </summary>
            public event EventHandler<ResponseReceivedEventArgs> PermissionGranted
            {
                add
                {
                    m_PermissionGranted.Add(value);
                }
                remove
                {
                    m_PermissionGranted.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs> m_PermissionGranted = new Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs>();

            /// <summary>
            /// Method to safely raise the PermissionGranted event.
            /// </summary>
            private void OnPermissionGranted(CertificateRequestResponse response)
            {
                m_PermissionGranted.Invoke(this, new ResponseReceivedEventArgs(response));
            }

            #endregion PermissionGranted Event

            #region PermissionDenied Event

            /// <summary>
            /// Event which is fired when permission is denied.
            /// </summary>
            public event EventHandler<ResponseReceivedEventArgs> PermissionDenied
            {
                add
                {
                    m_PermissionDenied.Add(value);
                }
                remove
                {
                    m_PermissionDenied.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs> m_PermissionDenied = new Pyxis.Utilities.EventHelper<ResponseReceivedEventArgs>();

            /// <summary>
            /// Method to safely raise the PermissionDenied event.
            /// </summary>
            private void OnPermissionDenied(CertificateRequestResponse response)
            {
                m_PermissionDenied.Invoke(this, new ResponseReceivedEventArgs(response));
            }

            #endregion PermissionDenied Event

            #region CertificateReceived Event

            /// <summary> EventArgs for a CertificateReceived event. </summary>    
            public class CertificateReceivedEventArgs : EventArgs
            {
                private readonly Certificate m_certificate;

                /// <summary>The Certificate.</summary>
                public Certificate Certificate
                {
                    get { return m_certificate; }
                }

                internal CertificateReceivedEventArgs(Certificate theCertificate)
                {
                    m_certificate = theCertificate;
                }
            }

            /// <summary> Event handler for CertificateReceived. </summary>
            public event EventHandler<CertificateReceivedEventArgs> CertificateReceived
            {
                add
                {
                    m_CertificateReceived.Add(value);
                }
                remove
                {
                    m_CertificateReceived.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<CertificateReceivedEventArgs> m_CertificateReceived = new Pyxis.Utilities.EventHelper<CertificateReceivedEventArgs>();

            /// <summary>
            /// Raises the CertificateReceived event.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="theCertificate "></param>
            public void OnCertificateReceived(object sender, Certificate theCertificate)
            {
                this.Stack.CertificateRepository.Add(theCertificate);

                m_CertificateReceived.Invoke(sender, new CertificateReceivedEventArgs(theCertificate));
            }

            #endregion CertificateReceived Event

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
            private Pyxis.Utilities.EventHelper<DisplayUriEventArgs> m_DisplayUri = new Pyxis.Utilities.EventHelper<DisplayUriEventArgs>();

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

            #region RemapPipeline Event

            /// <summary> EventArgs for a RemapPipeline event. </summary>    
            public class RemapPipelineEventArgs : EventArgs
            {
                private readonly Guid m_oldGuid;

                /// <summary>The Guid.</summary>
                public Guid OldGuid
                {
                    get { return m_oldGuid; }
                }

                private readonly byte[] m_replacementXml;

                public byte[] ReplacementXml
                {
                    get { return m_replacementXml; }
                }

                internal RemapPipelineEventArgs(Guid theGuid, byte[] replacementXml)
                {
                    m_oldGuid = theGuid;
                    m_replacementXml = replacementXml;
                }
            }

            /// <summary> Event handler for RemapPipeline. </summary>
            public event EventHandler<RemapPipelineEventArgs> RemapPipeline
            {
                add
                {
                    m_RemapPipeline.Add(value);
                }
                remove
                {
                    m_RemapPipeline.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<RemapPipelineEventArgs> m_RemapPipeline = new Pyxis.Utilities.EventHelper<RemapPipelineEventArgs>();

            /// <summary>
            /// Raises the RemapPipeline event.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="theGuid"></param>
            public void OnRemapPipeline(object sender, Guid theGuid, byte[] replacementXml)
            {
                m_RemapPipeline.Invoke(sender, new RemapPipelineEventArgs(theGuid, replacementXml));
            }

            #endregion RemapPipeline Event

            #region RequestClosed

            [System.Xml.Serialization.XmlRoot(Namespace = "urn:PyxNet.Service.CertificateRequester")]
            public class RequestClosedEventArgs : EventArgs
            {
                public static readonly new RequestClosedEventArgs Empty = new RequestClosedEventArgs();
            }

            /// <summary>
            /// The request has closed, been cancelled, or has been disposed.  
            /// </summary>
            public event EventHandler<RequestClosedEventArgs> RequestClosed
            {
                add
                {
                    m_RequestClosed.Add(value);
                }
                remove
                {
                    m_RequestClosed.Remove(value);
                }
            }
            private Pyxis.Utilities.EventHelper<RequestClosedEventArgs> m_RequestClosed = new Pyxis.Utilities.EventHelper<RequestClosedEventArgs>();

            /// <summary>
            /// Raises the <see cref="E:RequestClosed"/> event.
            /// </summary>
            /// <param name="e">The <see cref="PyxNet.CertificateRequester.RequestClosedEventArgs"/> instance containing the event data.</param>
            protected virtual void OnRequestClosed(RequestClosedEventArgs e)
            {
                m_RequestClosed.Invoke(this, e);
            }

            #endregion RequestClosed

            #endregion Events

            #region Handlers

            /// <summary>
            /// Handle the CertificateRequestResponse message arriving at our stack.
            /// </summary>
            /// <param name="sender">The sender.</param>
            /// <param name="args">The arguments.</param>
            private void HandleCertificateRequestResponse(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
            {
                CertificateRequestResponse response = new CertificateRequestResponse(new MessageReader(args.Message));

                // Check to see if it is the right one.
                if (response.Id.Equals(this.Id))
                {
                    // Handle any remappings....
                    foreach (Guid from in response.Remappings.Keys)
                    {
                        OnRemapPipeline(this, from, response.Remappings[from]);
                    }

                    // And any incoming certificates....
                    foreach (PyxNet.Service.Certificate c in response.CertificateList)
                    {
                        OnCertificateReceived(this, c);
                    }

                    // Notify that a response was received (general case).
                    OnResponseReceived(response);

                    // If there's a uri, then notify for that...
                    if (response.Url.Length > 0)
                    {
                        OnDisplayUri(this, new Uri(response.Url));
                    }

                    // And also notify for the specific case.
                    if (response.PermissionGranted)
                    {
                        Status.Value = State.ReceivedCertificate;
                        OnPermissionGranted(response);
                    }
                    else
                    {
                        Status.Value = State.ReceivedFalseResponse;
                        OnPermissionDenied(response);
                    }
                }
            }

            bool HandleQueryResult(object sender, Querier.ResultEventArgs args)
            {
                if (args.QueryResult.QueryGuid == this.Id)
                {
                    try
                    {
                        // Check to see if it is the right one.
                        Certificate response = new Certificate(
                            new MessageReader(args.QueryResult.ExtraInfo));
                        OnCertificateReceived(this, response);
                        return true;
                    }
                    catch (ArgumentException ex)
                    {
                        Trace.WriteLine(string.Format(
                            "Error parsing query result (ignoring): {0}", ex.ToString()));
                    }
                    catch (IndexOutOfRangeException ex)
                    {
                        Trace.WriteLine(string.Format(
                            "Error parsing query result (ignoring): {0}", ex.ToString()));
                    }
                }
                return false;
            }

            #endregion Handlers

            #region Start

            /// <summary>
            /// Blocks for only a short time.  Sends CertificateRequest to the license server.
            /// </summary>
            /// <param name="timeout">The timeout for finding a license server and/or its node.</param>
            public void Start(TimeSpan timeout)
            {
                m_certificateServerNodeInfo = null;

                try
                {
                    Status.Value = State.SearchingForCertificateServer;
                    if (LicenseServer == null)
                    {
                        // Find a license server.
                        ServiceFinder finder = new ServiceFinder(this.Stack);
                        LicenseServer = finder.FindService(
                            Service.CertificateServer.CertificateAuthorityServiceId,
                            timeout, out m_certificateServerNodeInfo);

                        //// Bit of a hack here.  Try again on the find, just in case it should have worked.
                        //if (licenseServer == null)
                        //{
                        //    licenseServer = finder.FindService(
                        //        Service.CertificateServer.CertificateAuthorityServiceId,
                        //        timeout, out certificateServerNodeInfo);
                        //}
                    }
                    else
                    {
                        // Find the server node.
                        m_certificateServerNodeInfo = NodeInfo.Find(
                            this.Stack, LicenseServer.Server, timeout);
                    }

                    if (m_certificateServerNodeInfo == null)
                    {
                        Trace.WriteLine("Unable to find certificate server.  Failing certificate request.");
                        Close();
                        return;
                    }

                    Status.Value = State.FoundCertificateServer;

                    // Create the request to send.
                    // The request is given the ID of this requester.
                    CertificateRequest certificateRequest = new CertificateRequest(
                        this.Id, this.Stack.NodeInfo);
                    certificateRequest.FactList.Add(this.FactList.Facts);

                    if (this.Stack.NodeInfo.Equals(m_certificateServerNodeInfo))
                    {
                        Status.Value = State.SentLicenseRequestToSelf;

                        // Connecting to self.  
                        // TODO: apparently we always forge for our self??
                        Certificate result = new Certificate(LicenseServer,
                            DateTime.Now + TimeSpan.FromDays(1));
                        foreach (ICertifiableFact fact in certificateRequest.FactList.Facts)
                        {
                            result.Add(fact);
                        }
                        result.SignCertificate(Stack.PrivateKey);
                        OnCertificateReceived(this, result);
                    }
                    else
                    {
                        m_certificateConnection = this.Stack.FindConnection(m_certificateServerNodeInfo, false);
                        if (m_certificateConnection != null)
                        {
                            Status.Value = State.SentLicenseRequest;
                            Trace.DebugWriteLine("Succesfully connected to certificate server.  Sending certificate request.");
                            if (this.Stack.SendSecureMessage(m_certificateConnection,
                                certificateRequest.ToMessage()))
                            {
                                return;
                            }
                        }

                        Status.Value = State.SentLicenseRequestIndirectly;
                        Trace.DebugWriteLine("Not connected to certificate server.  Relaying message instead.");
                        this.Stack.SendSecureMessage(m_certificateServerNodeInfo, certificateRequest.ToMessage());
                    }
                }
                catch (TimeoutException ex)
                {
                    throw new ApplicationException("Unable to connect to the certificate server.", ex);
                }
            }

            /// <summary>
            /// Blocks for only a short time.  Sends CertificateRequest to the license server.
            /// Throws ApplicationException.
            /// </summary>
            /// <param name="timeout">The timeout for finding a license server and/or its node.</param>
            /// <param name="licenseServer">The license server, or null if none specified.</param>
            public void Start(ServiceInstance licenseServer, TimeSpan timeout)
            {
                LicenseServer = licenseServer;
                Start(timeout);
            }

            #endregion
        }
    }
}