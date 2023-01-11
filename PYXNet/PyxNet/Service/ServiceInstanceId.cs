using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// A stongly typed guid to represent a ServiceInstanceId.
    /// A ServiceInstance is an instance of a service running on
    /// a specific node.  Note that a node can run multiple copies
    /// of the same service (each is identified by its ServiceInstanceId).
    /// </summary>
    [Serializable]
    public sealed class ServiceInstanceId : TypedGuid
    {
        /// <summary>
        /// Default Constructor - will generate a new ServiceInstanceId.
        /// </summary>
        public ServiceInstanceId()
            : base(Guid.NewGuid())
        {
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="copy">The ServiceInstanceId that you wish to duplicate.</param>
        public ServiceInstanceId(ServiceInstanceId copy)
            : base(copy.Guid)
        {
            // Empty body.
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public ServiceInstanceId(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #region To/From Message
        /// <summary>
        /// Append the ServiceInstanceId to an existing message.  
        /// This does not include any message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(Guid);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a ServiceInstanceId.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Guid = reader.ExtractGuid();
        }
        #endregion
    }    
}
