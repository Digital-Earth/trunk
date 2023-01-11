using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace PyxNet
{
    // TODO: Consider replacing XML serialization with raw binary.
    /// <summary>
    /// Helper class - Encrypts and decrypts messages.  
    /// </summary>
    public static class EncryptedMessageHelper
    {
        private static Pyxis.Utilities.TraceTool s_trace = 
            new Pyxis.Utilities.TraceTool(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// A globally accessible trace for encryption and related logging.
        /// </summary>
        public static Pyxis.Utilities.TraceTool Trace
        {
            get { return EncryptedMessageHelper.s_trace; }
        }

        /// <summary>
        /// Tag to identify an encrypted message.
        /// </summary>
        public const string MessageID = "ENCR";

        /// <summary>
        /// Encrypts the given rawMessage with with the recipient's public key.
        /// </summary>
        /// <param name="rawMessage"></param>
        /// <param name="recipient"></param>
        /// <returns>A newly created (encrypted) message.</returns>
        public static Message EncryptMessage(Message rawMessage, NodeInfo recipient)
        {
            Trace.WriteLine("Encrypting a message using {0} ({1})",
                DLM.PrivateKey.GetKeyDescription(recipient.PublicKey), rawMessage.Identifier);

            PyxNet.DLM.CipherMessage encryption = PyxNet.DLM.RSATool.EncryptMessage(
                rawMessage.Bytes.Array, recipient.PublicKey);
            Message result = new Message(MessageID);
            result.AppendXmlObject(encryption);
            return result;
        }

        /// <summary>
        /// Decrypts a message, using decoder's private key.
        /// </summary>
        /// <param name="encryptedMessage"></param>
        /// <param name="decoder"></param>
        /// <returns>The embedded (now decrypted) message.  Throws on error.</returns>
        public static Message DecryptMessage(Message encryptedMessage,
            PyxNet.DLM.RSATool decoder)
        {
            MessageReader reader = new MessageReader(encryptedMessage);

            PyxNet.DLM.CipherMessage encryption = 
                reader.ExtractXmlObject < PyxNet.DLM.CipherMessage>();

            Message decryptedMessage = new Message(decoder.DecryptMessage(encryption));

            return decryptedMessage;
        }

        /// <summary>
        /// Decrypts a message, using the specified private key.
        /// </summary>
        /// <param name="encryptedMessage"></param>
        /// <param name="key"></param>
        /// <returns>The embedded (now decrypted) message.  Throws on error.</returns>
        public static Message DecryptMessage(Message encryptedMessage,
            PyxNet.DLM.PrivateKey key)
        {
            Trace.WriteLine("Decrypting a message using {0} ({1})",
                DLM.PrivateKey.GetKeyDescription(key), encryptedMessage.Identifier);

            return DecryptMessage(encryptedMessage, new PyxNet.DLM.RSATool(key));
        }
    }    
}
