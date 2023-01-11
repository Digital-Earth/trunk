using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace PyxNet.Service
{
    /// <summary>
    /// Utility class to help find a fact by first looking through the local
    /// certificates for the fact, and then searching over PYXNet for the fact.
    /// </summary>
    public class CertificateFinder
    {
        #region Fields and Properties

        private Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

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

        private ICertifiableFact m_fact;

        /// <summary>
        /// The fact that we want to find.
        /// </summary>
        public ICertifiableFact Fact
        {
            get { return m_fact; }
        }

        private Querier m_querier;

        /// <summary>
        /// Gets or sets the querier.
        /// </summary>
        private Querier Querier
        {
            get { return m_querier; }
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

        // TODO: initialize this value from a new timeout value in the stack.
        private TimeSpan m_timeout = TimeSpan.FromSeconds(15);

        public TimeSpan Timeout
        {
            get { return m_timeout; }
            set { m_timeout = value; }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Construct an instance.
        /// </summary>
        /// <param name="stack">The stack to use.</param>
        /// <param name="requestedServiceId">The requested service id.</param>
        public CertificateFinder(
            Stack stack, 
            ICertifiableFact fact)
        {
            m_stack = stack;
            m_fact = fact;

            // Handle CertificateRequestResponse message arriving at the stack.
            this.Stack.SecureStreamRegisterHandler(
                CertificateRequestResponse.MessageID, HandleCertificateRequestResponse);
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
        ~CertificateFinder()
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

            internal CertificateReceivedEventArgs(Certificate  theCertificate )
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

            m_CertificateReceived.Invoke( sender, new CertificateReceivedEventArgs(theCertificate));
        }
        #endregion CertificateReceived Event

        #region RequestClosed
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
            // Construct the response from the message.
            CertificateRequestResponse response = new CertificateRequestResponse(new MessageReader(args.Message));

            // Add any incoming certificates.
            foreach (PyxNet.Service.Certificate c in response.CertificateList)
            {
                OnCertificateReceived(this, c);
            }

            // Notify that a response was received.
            OnResponseReceived(response);
        }

        void HandleQueryResult(object sender, Querier.ResultEventArgs args)
        {
            if (args.QueryResult.QueryGuid == this.Id)
            {
                try
                {
                    // Check to see if it is the right one.
                    Certificate response = new Certificate(
                        new MessageReader(args.QueryResult.ExtraInfo));
                    OnCertificateReceived(this, response);
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
        }

        #endregion Handlers

        /// <summary>
        /// Start the querying process.  Non-blocking.
        /// </summary>
        public void Start()
        {
            // Send out the query.
            string queryString = m_fact.UniqueKeyword;
            Query query = new Query(this.Stack.NodeInfo, queryString);
            this.Id = query.Guid;
            this.m_querier = new Querier(this.Stack, query, 1000);
            this.Querier.Result += HandleQueryResult;
            this.Querier.Start();
        }

        /// <summary>
        /// Stop the querying process.
        /// </summary>
        public void Stop()
        {
            this.Querier.Stop();
        }

        /// <summary>
        /// Blocking call that handles all searching and waiting.
        /// </summary>
        /// <returns></returns>
        public ICertifiableFact Find()
        {
            string keyword = m_fact.UniqueKeyword;

            // Search for it locally first
            foreach (PyxNet.Service.ICertifiableFact fact in
                m_stack.CertificateRepository.GetMatchingFacts(keyword, m_fact.GetType()))
            {
                return fact;
            }

            // no local fact that matches so look remote
            Pyxis.Utilities.SynchronizationEvent certificateTimer =
                new Pyxis.Utilities.SynchronizationEvent(m_timeout);

            // If we didn't find one, then request one.
            CertificateReceived +=
                delegate(object sender, PyxNet.Service.CertificateFinder.CertificateReceivedEventArgs c)
                {
                    certificateTimer.Pulse();
                };

            Start();
            certificateTimer.Wait();
            Stop();
            if (certificateTimer.TimedOut)
            {
                return null;
            }

            // we should find it locally now.
            foreach (PyxNet.Service.ICertifiableFact fact in
                m_stack.CertificateRepository.GetMatchingFacts(keyword, m_fact.GetType()))
            {
                return fact;
            }

            // Apparently we received a certificate, but it didn't actually match.
            return null;
        }
    }
}
