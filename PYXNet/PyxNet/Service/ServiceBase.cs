using System;
using System.Collections.Generic;
using Pyxis.Utilities;

namespace PyxNet.Service
{
    /// <summary>
    /// Abstract implementation of a Service.  Handles publishing the 
    /// service certificate, attaching to stack, etc.
    /// </summary>
    public abstract class ServiceBase
    {
        #region Tracer

        protected readonly NumberedTraceTool<ServiceBase> Tracer =
            new NumberedTraceTool<ServiceBase>(TraceTool.GlobalTraceLogEnabled);

        #endregion

        #region Properties

        private ServiceId m_serviceId;

        public ServiceId ServiceId
        {
            get { return m_serviceId; }
        }

        /// <summary>
        /// The underlying stack that connects this server to PyxNet.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// The underlying stack that connects this server to PyxNet.
        /// </summary>
        public Stack Stack
        {
            get { return m_stack; }
        }

        private Service.Certificate m_certificate;

        public virtual Service.Certificate Certificate
        {
            get
            {
                if (m_certificate == null)
                {
                    Tracer.WriteLine("No certificate.  Requesting one...");

                    try
                    {
                        // We have no certificate.  Request one.
                        m_certificate = Stack.CertificateRepository.RequestCertificate(
                            this.Stack, this.ServiceId);

                        if (m_certificate != null)
                        {
                            Tracer.WriteLine("Received a certificate.");
                        }
                    }
                    catch (ApplicationException)
                    {
                    }

                    if (null == m_certificate)
                    {
                        Tracer.WriteLine("Didn't receive a certificate.  Creating one...");

                        // Create our own certificate.  This is a forgery, really.
                        // TODO: Add certification of certificates!
                        ServiceInstance serviceInstance = ServiceInstance.Create(
                            this.ServiceId,
                            this.Stack.NodeInfo.NodeId);
                        m_certificate = ServiceInstanceCertificateHelper.Create(
                            this.Stack.PrivateKey,
                            serviceInstance,  // Any authority will do.
                            DateTime.Now + TimeSpan.FromDays(2),
                            serviceInstance);
                        this.Stack.CertificateRepository.Add(m_certificate);
                    }


                    //--
                    //-- short term fix
                    //-- start thread that sleeps until the certificate expires.
                    //-- then null out the certificate field, this will force a
                    //-- request for a new certificate
                    //--
                    System.Threading.Thread thread = new System.Threading.Thread(
                        delegate()
                        {
                            while( m_certificate != null )
                            {
                                TimeSpan timeRemaining = m_certificate.ExpireTime - DateTime.Now;

                                Tracer.WriteLine("Service Certificate: timeRemaining = {0}", timeRemaining.ToString());
                                if (Tracer.Enabled == false)
                                {
                                    System.Diagnostics.Trace.WriteLine(
                                        string.Format("Service Certificate: timeRemaining = {0}", timeRemaining.ToString())
                                        );
                                }

                                if (timeRemaining.TotalMilliseconds > 0)
                                {
                                    //--
                                    //-- wait until the certificate expires
                                    //--
                                    Tracer.WriteLine("Service Certificate: waiting");
                                    new Pyxis.Utilities.SynchronizationEvent(timeRemaining).Wait();
                                    Tracer.WriteLine("Service Certificate: expired");
                                    if (Tracer.Enabled == false)
                                    {
                                        System.Diagnostics.Trace.WriteLine("Service Certificate: expired");
                                    }

                                    //-- 
                                    //-- remove expired certificate
                                    //--
                                    this.Stack.CertificateRepository.Remove(m_certificate);

                                    //--
                                    //-- request new certificate
                                    //--
                                    m_certificate = Stack.CertificateRepository.RequestCertificate(this.Stack, this.ServiceId);
                                    
                                    if (m_certificate != null)
                                    {
                                        Tracer.WriteLine("Service Certificate: received new certificate");
                                        if (Tracer.Enabled == false)
                                        {
                                            System.Diagnostics.Trace.WriteLine("Service Certificate: received new certificate");
                                        }
                                    }
                                }
                            }
                            System.Diagnostics.Debug.Assert(false, "Certificate Watch Thread Terminating");
                        });

                    thread.Name = "Service Certificate Watch";
                    thread.IsBackground = true;
                    thread.Start();

                }

                return m_certificate;
            }
        }

        #endregion

        #region Construction

        /// <summary>
        /// Constructor.  Creates a server attached to the given stack.
        /// This can block for a while.
        /// </summary>
        /// <param name="stack"></param>
        protected ServiceBase(Stack stack, ServiceId serviceId)
        {
            if (null == stack)
            {
                throw new ArgumentNullException("stack");
            }

            Tracer.WriteLine("Constructing on stack {0}.", stack.ToString());

            m_stack = stack;
            m_serviceId = serviceId;

            // Check to see if there is a valid certificate in the stack's 
            //  certificate repository.  If there is, then we are authorized,
            //  and the stack will automatically publish that certificate (so
            //  other nodes can find us.)
            Tracer.WriteLine("Checking validity of certificate.");
            if (!Certificate.Valid)
            {
                InvalidOperationException exception = new InvalidOperationException(
                    "This node is not authorized to act as a server.");
                Tracer.WriteLine("The certificate is invalid.  Throwing exception: {0}.",
                    exception.ToString());
                throw exception;
            }
            Tracer.WriteLine("The certificate is valid.  Construction complete.");
        }

        #endregion Construction

        private class PublishedServiceId : Publishing.Publisher.IPublishedItemInfo
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
                        return Publishing.Publisher.CreateQueryResult(
                            stack, query, result);
                    }
                }
                return null;
            }

            #endregion
        }
    }
}
