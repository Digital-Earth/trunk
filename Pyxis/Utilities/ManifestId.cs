using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// A stongly typed guid to represent a ManifestId.
    /// </summary>
    [Serializable]
    public sealed class ManifestId : TypedGuid
    {
        /// <summary>
        /// Default Constructor - will generate a new ManifestId.
        /// </summary>
        public ManifestId()
            : base(Guid.NewGuid())
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestId"/> class.
        /// </summary>
        /// <param name="copy">The guid to copy.</param>
        public ManifestId(Guid copy)
            : base(copy)
        {
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="copy">The ManifestId that you wish to duplicate.</param>
        public ManifestId(ManifestId copy)
            : base(copy.Guid)
        {
            // Empty body.
        }

        ///// <summary>
        ///// Construct from a message reader.
        ///// </summary>
        //public ManifestId(MessageReader reader)
        //{
        //    this.FromMessageReader(reader);
        //}

        //#region To/From Message
        ///// <summary>
        ///// Append the ManifestId to an existing message.  
        ///// This does not include any message header.
        ///// </summary>
        ///// <param name="message">The message to append to.  (will be modified)</param>
        ///// <returns></returns>
        //public void ToMessage(Message message)
        //{
        //    message.Append(Guid);
        //}

        ///// <summary>
        ///// Initialize the members from a message reader.  The message reader
        ///// should be properly set to point at the start of a ManifestId.
        ///// </summary>
        ///// <param name="reader">The message reader to read from.</param>
        //public void FromMessageReader(MessageReader reader)
        //{
        //    Guid = reader.ExtractGuid();
        //}
        //#endregion
    }    
}
