using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{

    /// <summary>
    /// A ServiceInstance identifies an instance of a service running on
    /// a specific node.  Note that a node can run multiple copies
    /// of the same service (each is identified by its ServiceInstanceId).
    /// </summary>
    public class ServiceInstance: ITransmissibleWithReader, IIdentifiable 
    {
        #region Properties
        /// <summary>
        /// The node that is serving up (hosting) this service instance.
        /// </summary>
        private NodeId m_server;

        /// <summary>
        /// The node that is serving up (hosting) this service instance.
        /// </summary>
        public NodeId Server
        {
            get { return m_server; }
        }

        /// <summary>
        /// The service that is being hosted.
        /// </summary>
        private ServiceId m_serviceId;

        /// <summary>
        /// The service that is being hosted.
        /// </summary>
        public ServiceId ServiceId
        {
            get { return m_serviceId; }
        }

        /// <summary>
        /// A unique Id for this instance of this service.
        /// </summary>
        private ServiceInstanceId m_serviceInstanceId;

        /// <summary>
        /// A unique Id for this instance of this service.
        /// </summary>
        public ServiceInstanceId ServiceInstanceId
        {
            get { return m_serviceInstanceId; }
        }

        #endregion /* Properties */

        #region Constructors

        /// <summary>
        /// Private constructor - Use Create to create a new object.
        /// </summary>
        /// <param name="service"></param>
        /// <param name="server"></param>
        private ServiceInstance(ServiceId service, NodeId server)
        {
            m_server = server;
            m_serviceId = service;
            m_serviceInstanceId = new ServiceInstanceId();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ServiceInstance"/> class.
        /// This is for serialization use only!
        /// </summary>
        public ServiceInstance()
        {
        }

        /// <summary>
        /// Factory method to create a new ServiceInstance.  We usually
        /// don't want to make new service instances, unless we are a
        /// signing authority creating a new certificate.
        /// </summary>
        /// <param name="service"></param>
        /// <param name="server"></param>
        /// <returns></returns>
        public static ServiceInstance Create(ServiceId service, NodeId server)
        {
            return new ServiceInstance(service, server);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public ServiceInstance(MessageReader reader)
        {
            FromMessage(reader);
        }

        #endregion /* Constructors */

        /// <summary>
        /// Finds the given serviceId within the stack's certificate repository.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="serviceId">The service id.</param>
        /// <returns></returns>
        internal static ServiceInstance FindLocal(Stack stack, ServiceId serviceId)
        {
            return stack.CertificateRepository.RequestCertificate(
                stack, serviceId).ServiceInstance;
        }

        #region ITransmissibleWithReader Members

        public void FromMessage(Message message)
        {
            if (message.Identifier != MessageId)
            {
                throw new ArgumentException(string.Format(
                    "Invalid message identifier.  Saw {0}, but was expecting {1}.",
                    message.Identifier, MessageId));
            }
            MessageReader reader = new MessageReader(message);
            FromMessage(reader);
        }

        public void FromMessage(MessageReader reader)
        {
            m_server = new NodeId(reader);
            m_serviceId = new ServiceId(reader);
            m_serviceInstanceId = new ServiceInstanceId(reader);
        }

        #endregion

        #region ITransmissible Members

        /// <summary>
        /// Append the ServiceInstance to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }
            if ((m_server == null) || (m_serviceId == null) || (m_serviceInstanceId == null))
            {
                throw new InvalidOperationException("Unable to stream an uninitialized ServiceInstance.");
            }
            m_server.ToMessage(message);
            m_serviceId.ToMessage(message);
            m_serviceInstanceId.ToMessage(message);
        }

        public static string MessageId = "SeIn";

        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        #endregion

        #region IIdentifiable Members

        public Guid Id
        {
            get { return this.ServiceInstanceId.Guid;}
        }

        #endregion

        public override string ToString()
        {
            return string.Format("ServiceInstance( Id=\"{0}\", Service=\"{1}\", Server=\"{2}\")",
                m_serviceInstanceId, m_serviceId, m_server);
        }

        public override bool Equals(object obj)
        {
            ServiceInstance objectAsServiceInstance = obj as ServiceInstance;
            return ((objectAsServiceInstance != null) &&
                (this.Id == objectAsServiceInstance.Id) &&
                (this.Server.Equals( objectAsServiceInstance.Server)) &&
                (this.ServiceId.Equals( objectAsServiceInstance.ServiceId)) &&
                (this.ServiceInstanceId.Equals( objectAsServiceInstance.ServiceInstanceId)));
        }

        public override int GetHashCode()
        {
            return this.ToMessage().GetHashCode();
        }
    }
}
