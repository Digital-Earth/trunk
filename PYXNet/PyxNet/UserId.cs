using System;
using System.Collections.Generic;
using System.Text;
using PyxNet.DLM;

namespace PyxNet
{
    public class UserId : IEquatable<UserId>
    {
        public PublicKey PublicKey
        {
            get;
            set;
        }

        public UserId(PublicKey publicKey)
        {
            if (publicKey == null)
            {
                throw new ArgumentNullException("publicKey");
            }

            PublicKey = publicKey;
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of the record.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public UserId(MessageReader reader)
        {
            PublicKey = new PublicKey(reader.ExtractCountedBytes());
        }

        /// <summary>
        /// Append "this" to an existing message.  
        /// (Does not include the message header.)
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            message.AppendCountedBytes(PublicKey.Key);
        }

        #region Equality

        public override bool Equals(object obj)
        {
            return Equals(obj as UserId);
        }

        public override int GetHashCode()
        {
            return PublicKey.GetHashCode();
        }

        #region IEquatable<UserId> Members

        public bool Equals(UserId other)
        {
            return other != null && 
                ((other == this) || PublicKey.Equals(other.PublicKey));
        }

        #endregion

        #endregion
    }
}
