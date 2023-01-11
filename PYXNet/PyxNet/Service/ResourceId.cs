using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// A stongly typed guid to represent a Resource.
    /// </summary>
    [Serializable]
    public sealed class ResourceId : TypedGuid
    {
        /// <summary>
        /// Default Constructor - will generate a new Resource.
        /// </summary>
        public ResourceId()
            : base(Guid.NewGuid())
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourceId"/> class.
        /// </summary>
        /// <param name="value">The value.</param>
        public ResourceId(Guid value)
            : base(value)
        {
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="copy">The Resource that you wish to duplicate.</param>
        public ResourceId(ResourceId copy)
            : base(copy.Guid)
        {
            // Empty body.
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public ResourceId(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #region To/From Message
        /// <summary>
        /// Append the Resource to an existing message.  
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
        /// should be properly set to point at the start of a Resource.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Guid = reader.ExtractGuid();
        }
        #endregion
    }
}
