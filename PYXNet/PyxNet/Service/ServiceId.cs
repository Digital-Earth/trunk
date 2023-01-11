using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{

    /// <summary>
    /// A stongly typed guid to represent a ServiceId.  A 
    /// service Id uniquely identifies a well known service
    /// (such as a LicenseServer, PyxNetHub, etc).
    /// </summary>
    [Serializable]
    public sealed class ServiceId : TypedGuid
    {
        /// <summary>
        /// Default Constructor - will generate a new ServiceId.
        /// (Likely only used in test code.)
        /// </summary>
        public ServiceId()
            : base(Guid.NewGuid())
        {
            m_subserviceId = Guid.Empty;
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="copy">The ServiceId that you wish to duplicate.</param>
        public ServiceId(ServiceId copy)
            : base(copy.Guid)
        {
            m_subserviceId = copy.SubserviceId;
        }

        /// <summary>
        /// Initialize from Guid. (Subservice defaults to "empty".)
        /// </summary>
        /// <param name="copy">The guid value for this ServiceId.</param>
        public ServiceId(Guid guid)
            : base(guid)
        {
            m_subserviceId = Guid.Empty;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ServiceId"/> class.
        /// </summary>
        /// <param name="serviceId">The service id.</param>
        /// <param name="subserviceId">The subservice id.</param>
        public ServiceId(Guid serviceId, Guid subserviceId)
            : base(serviceId)
        {
            m_subserviceId = subserviceId;
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public ServiceId(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        // Consider using an embedded (possibly null) ServiceId....
        private Guid m_subserviceId;

        /// <summary>
        /// Gets the subservice id.
        /// </summary>
        /// <value>The subservice id.</value>
        public Guid SubserviceId
        {
            get { return m_subserviceId; }
        } 

        #region To/From Message
        /// <summary>
        /// Append the ServiceId to an existing message.  
        /// This does not include any message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(Guid);
            message.Append(SubserviceId);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a ServiceId.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Guid = reader.ExtractGuid();
            m_subserviceId = reader.ExtractGuid();
        }
        #endregion

        static private string ToSearchString(
            String guidString, String subserviceIdString)
        {
            return string.Format("REQ:{0}:{1}", guidString, subserviceIdString);
        }

        /// <summary>
        /// Converts this service id to a well-known search string.
        /// </summary>
        /// <returns></returns>
        public string ToSearchString()
        {
            return ToSearchString(this.Guid.ToString(), this.SubserviceId.ToString());
        }

        public string ToGuidSearchString()
        {
            return ToSearchString(this.Guid.ToString(), "*");
        }

        public string ToSubserviceIdSearchString()
        {
            return ToSearchString("*", this.SubserviceId.ToString());
        }
    }
}
