/******************************************************************************
CertificateServer.cs

begin      : 05/12/2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define USE_ALTERNATE_LICENSESERVICE_ID
#define GENES_TEST_SYSTEM // Hey, it's for testing.

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// Abstract implementation of a CertificateServer
    /// </summary>
    public abstract class CertificateServer: ServiceBase 
    {
        #region ServiceIds

        /// <summary>
        /// A certificate server must have a certificate authorizing it.  This 
        /// certificate will authorize the node to operate the service 
        /// identified by CertificateAuthorityServiceId.
        /// </summary>
        public static readonly Service.ServiceId CertificateAuthorityServiceId = 
#if USE_ALTERNATE_LICENSESERVICE_ID
#if GENES_TEST_SYSTEM
            new PyxNet.Service.ServiceId(
                new Guid("{6A84C775-65A0-478c-9913-9CE3B9514E2F}"));
#warning "This code should never be checked in!"
#else
            // This service id will only be used by clients compiled with 
            // USE_ALTERNATE_SERVICE_ID.  This will allow us to test licensing
            // without crippling existing network services.
            new PyxNet.Service.ServiceId(
                new Guid("{635CDF8E-380B-4da9-B250-2C7041AE2790}"));
#endif
#else
            new PyxNet.Service.ServiceId(
                new Guid("{F18CA576-957B-41ec-84C5-2618BF7184F4}"));
#endif

        #endregion ServiceIds

        #region Construction

        /// <summary>
        /// Constructor.  Creates a license server attached to the given stack.
        /// This can block for a while.
        /// </summary>
        /// <param name="stack"></param>
        protected CertificateServer(Stack stack):
            base(stack, CertificateAuthorityServiceId)
        {
            if (null == stack)
            {
                throw new ArgumentNullException("stack");
            }

            Tracer.WriteLine("Constructing on stack {0}.", stack.ToString());

            // Register the certificate request message handler.
            this.Stack.SecureStreamRegisterHandler(
                CertificateRequest.MessageID, HandleCertificateRequest);

            // Check to see if there is a valid certificate in the stack's 
            // certificate repository.  If there is, then we are authorized,
            // and the stack will automatically publish that certificate (so
            // other nodes can find us.)
            Tracer.WriteLine("Checking validity of certificate.");
            if (!Certificate.Valid)
            {
            	InvalidOperationException exception = new InvalidOperationException(
            		"This node is not authorized to act as a license server.");
                Tracer.WriteLine("The certificate is invalid.  Throwing exception: {0}.",
                    exception.ToString());
                throw exception;
            }
            Tracer.WriteLine("The certificate is valid.  Construction complete.");
        }

        #endregion Construction

        #region CertificateRequestReceived Event

        /// <summary> EventArgs for a CertificateRequestReceived event. </summary>    
        public class CertificateRequestReceivedEventArgs : EventArgs
        {
            private readonly CertificateRequest m_certificateRequest;

            public CertificateRequest CertificateRequest
            {
                get { return m_certificateRequest; }
            }

            private CertificateRequestResponse m_CertificateRequestResponse;

            public CertificateRequestResponse CertificateRequestResponse
            {
                get { return m_CertificateRequestResponse; }
                set { m_CertificateRequestResponse = value; }
            }

            internal CertificateRequestReceivedEventArgs( CertificateRequest request)
            {
                m_certificateRequest = request;
            }
        }

        /// <summary> Event handler for CertificateRequestReceived. </summary>
        /// <remarks>Not thread-safe.</remarks>
        public event EventHandler<CertificateRequestReceivedEventArgs> CertificateRequestReceived;

        /// <summary>
        /// Raises the CertificateRequestReceived event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theServiceId "></param>
        public List<CertificateRequestResponse> OnCertificateRequestReceived(object sender, 
            CertificateRequest certificateRequest)
        {
            List<CertificateRequestResponse> result = 
                new List<CertificateRequestResponse>();

            EventHandler<CertificateRequestReceivedEventArgs> handler = 
                CertificateRequestReceived;

            if (handler != null)
            {
                CertificateRequestReceivedEventArgs args = 
                    new CertificateRequestReceivedEventArgs(certificateRequest);

                foreach (EventHandler<CertificateRequestReceivedEventArgs> function in
                    handler.GetInvocationList())
                {
                    args.CertificateRequestResponse = null;
                    function(sender, args);
                    if (args.CertificateRequestResponse != null)
                    {
                        result.Add(args.CertificateRequestResponse);
                    }
                }
            }
            return result;
        }

        #endregion CertificateRequestReceived Event

        private class PublishedServiceId: Publishing.Publisher.IPublishedItemInfo
        {
            private readonly ServiceId m_serviceId;

            /// <summary>
            /// Gets the service id.
            /// </summary>
            /// <value>The service id.</value>
            public ServiceId ServiceId
            {
                get { return m_serviceId; }
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="PublishedServiceId"/> class.
            /// </summary>
            /// <param name="serviceId">The service id.</param>
            public PublishedServiceId(ServiceId serviceId)
            {
                m_serviceId = serviceId;
            }

            #region IPublishedItemInfo Members

            /// <summary>
            /// Gets the keywords for this published item.  This is the
            /// set of terms that the item will be indexed on in the
            /// query hash table.
            /// </summary>
            /// <value>The keywords.</value>
            public IEnumerable<string> Keywords
            {
                get 
                {
                    yield return ServiceId.ToSearchString();
                }
            }

            /// <summary>
            /// Does this match the specified query?
            /// </summary>
            /// <param name="query">The query.</param>
            /// <param name="stack">The stack.</param>
            /// <returns>
            /// True iff this matches the specified query.
            /// </returns>
            public QueryResult Matches(Query query, Stack stack)
            {
                foreach (string keyword in Keywords)
                {
                    if (query.Contents.Contains(keyword))
                    {
                        Message result = new Message("SIdX");
                        ServiceId.ToMessage(result);
                        return Publishing.Publisher.CreateQueryResult(stack, query, result);
                    }
                }
                return null;
            }

            #endregion
        }

        /// <summary>
        /// Creates a new certificate.
        /// This can block for a while.
        /// </summary>
        /// <param name="certifiedServiceId">The certified service id.</param>
        /// <param name="certifiedNode">The certified node.</param>
        /// <param name="duration">The duration.</param>
        /// <returns></returns>
        protected PyxNet.Service.Certificate CreateCertificate( 
            CertificateRequest certificateRequest, TimeSpan duration)
        {
            if (null == certificateRequest)
            {
                throw new ArgumentNullException("certificateRequest");
            }

            // TODO: Check for durations that are way too long or small.

            // Create the certificate.
            this.Stack.Tracer.WriteLine("The certificate server is creating a certificate.");
            Certificate certificate = new Certificate(this.Certificate.ServiceInstance,
                DateTime.Now + duration);

            foreach (ICertifiableFact fact in certificateRequest.FactList.Facts)
            {
                certificate.Add(fact);
            }
            certificate.SignCertificate(Stack.PrivateKey);

            // There is no reason that it should not be valid, unless the duration was absurdly small.
            System.Diagnostics.Debug.Assert(certificate.Valid);

            // Add to local repository.
            this.Stack.Tracer.WriteLine("Adding the certificate to the local repository.");
            this.Stack.CertificateRepository.Add(certificate);

            return certificate;
        }

        private class Response : CertificateRequestResponse
        {
            #region Tracer

            private readonly Pyxis.Utilities.NumberedTraceTool<Response> Tracer =
                new Pyxis.Utilities.NumberedTraceTool<Response>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

            #endregion

            private NodeInfo m_recipient;

            public Response(
                Guid requestId,
                bool permissionGranted,
                string url,
                NodeInfo recipient)
                :
                base(requestId, permissionGranted, url)
            {
                m_recipient = recipient;
            }

            public bool Send(Stack stack)
            {
                // Send using a relay.
                Tracer.WriteLine("Attempting to relay message.");
                return stack.SendSecureMessage(m_recipient, this.ToMessage());
            }
        }

        /// <summary>
        /// Sends the positive response with message to certify.
        /// Certificate duration specified with argument.
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="recipient">The recipient.</param>
        /// <param name="messageToCertify">The message to certify.</param>
        /// <param name="duration">The duration of new certificate.</param>
        /// <returns></returns>
        protected bool SendPositiveResponse(
            Guid requestId,
            NodeInfo recipient,
            Message messageToCertify,
            TimeSpan duration)
        {
            // Get the original message.
            CertificateRequest actualMessage = ExtractCertificateRequest(messageToCertify);
            System.Diagnostics.Debug.Assert(null != actualMessage && actualMessage.Id == requestId);

            // Create the certificate.
            Certificate certificate = CreateCertificate(actualMessage, duration);

            // Try to send.
            return SendPositiveResponse(requestId, actualMessage.Requester, certificate);
        }

        /// <summary>
        /// Sends the positive response with message to certify.
        /// Certificate will be generated with standard duration.
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="recipient">The recipient.</param>
        /// <param name="messageToCertify">The message to certify.</param>
        /// <returns></returns>
        protected bool SendPositiveResponse(
            Guid requestId,
            NodeInfo recipient,
            Message messageToCertify)
        {
            return SendPositiveResponse(requestId, recipient, messageToCertify, TimeSpan.FromDays(7));
        }

        /// <summary>
        /// Sends the positive response with certificate.
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="recipient">The recipient.</param>
        /// <param name="certificate">The certificate.</param>
        /// <returns></returns>
        protected bool SendPositiveResponse(
            Guid requestId,
            NodeInfo recipient,
            Certificate certificate)
        {
            // Construct a response...
            Response response = new Response(requestId, true, null, recipient);

            // Add the new certificate.
            response.CertificateList.Add(certificate);

            // Try to send.
            return response.Send(this.Stack);
        }

        /// <summary>
        /// Send a response to the client indicating success.  (That the data
        /// can be published.)
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="originalRequest">The original Certificate Request.</param>
        /// <param name="duration">Duration time for new certificate</param>
        protected bool SendPositiveResponse( 
            Guid requestId,
            MessageHandlerCollection.MessageReceivedEventArgs originalRequest,
            TimeSpan duration)
        {
            // Get the original message.
            CertificateRequest actualMessage = ExtractCertificateRequest(originalRequest.Message);
            System.Diagnostics.Debug.Assert(null != actualMessage && actualMessage.Id == requestId);

            // Create the certificate.
            Certificate certificate = CreateCertificate(actualMessage, duration);

			// Try to send.
            return SendPositiveResponse(requestId, originalRequest, certificate);
        }

        /// <summary>
        /// Send a response to the client indicating success.  (That the data
        /// can be published.)
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="originalRequest">The original Certificate Request.</param>
        protected bool SendPositiveResponse(
            Guid requestId,
            MessageHandlerCollection.MessageReceivedEventArgs originalRequest)
        {
            return SendPositiveResponse(requestId, originalRequest, TimeSpan.FromDays(7));
        }

        /// <summary>
        /// Send a response to the client indicating success.  (That the data
        /// can be published.)
        /// </summary>
        /// <param name="requestId">The request id.</param>
        /// <param name="originalRequest">The original Certificate Request.</param>
        /// <param name="certificate">The certificate.</param>
        protected bool SendPositiveResponse(
            Guid requestId,
            MessageHandlerCollection.MessageReceivedEventArgs originalRequest,
            Certificate certificate)
        {
            // Get the original message.
            CertificateRequest actualMessage = ExtractCertificateRequest(originalRequest.Message);
            System.Diagnostics.Debug.Assert(null != actualMessage && actualMessage.Id == requestId);

            // Construct a response...
            Response response = new Response(requestId, true, null,
                actualMessage.Requester);

            // Add the new certificate.
            response.CertificateList.Add(certificate);

			// Try to send.
            return response.Send(this.Stack);
        }

        /// <summary>
        /// Send a response to the client indicating failure.  (That the data 
        /// cannot be published.)  A URL is sent to let the client negotiate.
        /// </summary>
        /// <param name="requester">The node that originated the certificate request.</param>
        /// <param name="requestId">The specific request's identity (so client can track transaction.)</param>
        /// <param name="url">HTTP address to negotiate the publish req.</param>
        protected bool SendNegativeResponse(
            NodeInfo requester,
            Guid requestId,
            String url)
        {
            // Construct a response...
            Response response = new Response(requestId, false, url, requester);

            // Try to send.
            return response.Send(this.Stack);
        }

        /// <summary>
        /// Extracts the certificate request contained in the raw message.
        /// </summary>
        /// <param name="rawMessage">The raw message.</param>
        /// <returns></returns>
        private static CertificateRequest ExtractCertificateRequest(Message rawMessage)
        {
            if (rawMessage.StartsWith(CertificateRequest.MessageID))
            {
                MessageReader reader = new MessageReader(rawMessage);
                CertificateRequest actualMessage = new CertificateRequest(reader);
                return actualMessage;
            }
            return null;
        }

        /// <summary>
        /// Processes a publish request, by first validating it, then passing 
        /// it on to a virtual function for actual processing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleCertificateRequest(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            this.Stack.Tracer.WriteLine(
                String.Format("CertificateServer: Handling certificate request for {0}.", this.Stack.ToString()));

            CertificateRequest actualMessage = ExtractCertificateRequest(args.Message);
            if (actualMessage != null)
            {
                HandleCertificateRequest(args, actualMessage);
            }
        }

        /// <summary>
        /// Handler for a CertificateRequest message.  (For override).
        /// </summary>
        /// <param name="originalRequest"></param>
        /// <param name="request"></param>
        protected abstract void HandleCertificateRequest(
            MessageHandlerCollection.MessageReceivedEventArgs originalRequest, 
            CertificateRequest request);
    }
}

namespace PyxNet.Test
{
    using PyxNet.Service;

    public class DemoCertificateServer : CertificateServer
    {
        /// <summary>
        /// A fake service that will be authorized.
        /// </summary>
        public static readonly Service.ServiceId TestServiceId =
            new PyxNet.Service.ServiceId(
            new Guid("{1DAA87E8-5B9C-46c8-AD5C-2B53918D257B}"));

        private readonly ServiceInstance m_serviceInstance;

        private bool m_permitPublication = false;

        public bool PermitPublication
        {
            get { return m_permitPublication; }
            set { m_permitPublication = value; }
        }

        public DemoCertificateServer(Stack s)
            : base(s)
        {
            m_serviceInstance = ServiceInstance.FindLocal(Stack, TestServiceId);
        }

        protected override void HandleCertificateRequest(
            MessageHandlerCollection.MessageReceivedEventArgs originalRequest, 
            CertificateRequest request)
        {
            if (m_permitPublication)
            {
                SendPositiveResponse(request.Id, originalRequest, CreateCertificate(request, TimeSpan.FromHours(1)));
            }
            else
            {
                SendNegativeResponse(request.Requester, request.Id, "http://www.google.com");
            }
        }
    }
}
