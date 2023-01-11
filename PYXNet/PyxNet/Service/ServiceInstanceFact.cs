using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// A certifiable fact that this service instance is valid.
    /// </summary>
    public class ServiceInstanceFact : ICertifiableFact
    {
        private Certificate m_certificate;

        /// <summary>
        /// Gets or sets the certificate.  Note that setting is only
        /// done as part of the construction process.
        /// </summary>
        /// <value>The certificate.</value>
        public Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        private ServiceInstance m_serviceInstance;

        /// <summary>
        /// Gets or sets the service instance.
        /// </summary>
        /// <value>The service instance.</value>
        public ServiceInstance ServiceInstance
        {
            get { return m_serviceInstance; }
            set { m_serviceInstance = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ServiceInstanceFact"/> class.
        /// </summary>
        /// <param name="serviceInstance">The service instance.</param>
        public ServiceInstanceFact(ServiceInstance serviceInstance)
        {
            m_serviceInstance = serviceInstance;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ServiceInstanceFact"/> class.
        /// </summary>
        public ServiceInstanceFact()
        {
            m_serviceInstance = null;
        }

        #region ICertifiableFact Members

        public Guid Id
        {
            get { return ServiceInstance.Id; }
        }

        #endregion

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
            if (m_serviceInstance == null)
            {
                m_serviceInstance = new ServiceInstance();
            }
            m_serviceInstance.FromMessage(reader);
        }

        #endregion

        #region ITransmissible Members

        public const string MessageId = "SIFT";
        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        public void ToMessage(Message message)
        {
            ServiceInstance.ToMessage(message);
        }

        #endregion

        public override string ToString()
        {
            return string.Format("ServiceInstance({0}, ServiceInstance={1}, Certificate={2})",
                this.Id, this.ServiceInstance, this.Certificate);
        }

        public override bool Equals(object obj)
        {
            ServiceInstanceFact otherFact = obj as ServiceInstanceFact;

            return (otherFact != null) && this.Id.Equals(otherFact.Id) &&
                this.ServiceInstance.Equals(otherFact.ServiceInstance) &&
                (((this.Certificate == null) && (otherFact.Certificate == null)) ||
                this.Certificate.Equals(otherFact.Certificate));
        }

        public override int GetHashCode()
        {
            throw new NotImplementedException();
        }

        #region ICertifiableFact Members

        /// <summary>
        /// Gets the keywords.
        /// </summary>
        /// <value>The keywords.</value>
        public IEnumerable<string> Keywords
        {
            get
            {
                // TODO: Consider special-casing out the empy guids here...
                yield return string.Format(m_serviceInstance.ServiceId.ToSearchString());
                yield return string.Format(m_serviceInstance.ServiceId.ToSubserviceIdSearchString());
                yield return string.Format(m_serviceInstance.ServiceId.ToGuidSearchString());
            }
        }

        public String UniqueKeyword
        {
            get
            {
                foreach (string keyword in Keywords)
                {
                    return keyword;
                }
                return null;
            }
        }

        #endregion
    }

}
