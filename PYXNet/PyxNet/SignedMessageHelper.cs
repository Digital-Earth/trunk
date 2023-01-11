using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    /// <summary>
    /// Helper class - Signs messages, and verifies signed messages.
    /// TODO: Consider replacing XML serialization with raw binary.
    /// </summary>
    public static class SignedMessageHelper
    {
        /// <summary>
        /// A globally accessible trace for encryption and related logging.
        /// </summary>
        public static Pyxis.Utilities.TraceTool Trace
        {
            get { return EncryptedMessageHelper.Trace;}
        }

        /// <summary>
        /// Tag to identify a signed message.
        /// </summary>
        public const string MessageID = "SGNM";

        /// <summary>
        /// Generates a signature for the given message, using the given key.
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ourPrivateKey"></param>
        /// <returns></returns>
        public static byte[] GenerateSignature(byte[] message,
            PyxNet.DLM.RSATool ourPrivateKey)
        {
            return ourPrivateKey.GenerateSignature(message);
        }

        /// <summary>
        /// Generates a signature for the given message, using the given key.
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ourPrivateKey"></param>
        /// <returns></returns>
        public static byte[] GenerateSignature(byte[] message,
            PyxNet.DLM.PrivateKey ourPrivateKey)
        {
            return GenerateSignature( message,
                new PyxNet.DLM.RSATool(ourPrivateKey));
        }

        /// <summary>
        /// Signs the given message with our private key.
        /// </summary>
        /// <param name="rawMessage">The message to be signed.</param>
        /// <param name="signingNode">The signing node.</param>
        /// <param name="ourPrivateKey">Private key to use for signing.</param>
        /// <returns>A newly created signed message.</returns>
        public static Message SignMessage(Message rawMessage,
            NodeInfo signingNode,
            PyxNet.DLM.RSATool ourPrivateKey)
        {
            // Convert the raw message fragment into an array.
            ArraySegment<byte> rawSegment = rawMessage.Bytes;
            byte [] array = new byte[ rawSegment.Count];
            Array.Copy( rawSegment.Array, rawSegment.Offset, array, 0, rawSegment.Count);

            PyxNet.DLM.SignedMessage signedMessage = ourPrivateKey.SignMessage(array);
            Message result = new Message(MessageID);
            result.AppendXmlObject(signedMessage);
            signingNode.ToMessage(result);

            return result;
        }

        /// <summary>
        /// Signs the given message with our private key.
        /// </summary>
        /// <param name="rawMessage">The message to be signed.</param>
        /// <param name="ourPrivateKey">Private key to use for signing.</param>
        /// <returns>A newly created signed message.</returns>
        public static Message SignMessage(Message rawMessage,
            NodeInfo signingNode,
            PyxNet.DLM.PrivateKey ourPrivateKey)
        {
            Trace.WriteLine("Signing a message using {0} ({1})",
                DLM.PrivateKey.GetKeyDescription(ourPrivateKey), rawMessage.Identifier);
            return SignMessage(rawMessage, signingNode, new PyxNet.DLM.RSATool(ourPrivateKey));
        }

        /// <summary>
        /// Verifies that a given message was signed with the private key 
        /// which corresponds to the given public key.
        /// </summary>
        /// <param name="message"></param>
        /// <param name="rawSignature"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool VerifySignature(byte[] message, byte[] rawSignature, PyxNet.DLM.PublicKey key)
        {
            return PyxNet.DLM.RSATool.VerifySignature( message, rawSignature, key);
        }

        /// <summary>
        /// Verify that the given signedMessage was signed by the sender.
        /// </summary>
        /// <param name="signedMessage"></param>
        /// <param name="sender"></param>
        /// <returns>The embedded signed message.</returns>
        public static Message VerifySignedMessage(Message signedMessage,
            NodeInfo sender)
        {
            Trace.WriteLine("Verifying a signature using {0} ({1})",
                DLM.PrivateKey.GetKeyDescription(sender.PublicKey), signedMessage.Identifier);

            MessageReader reader = new MessageReader(signedMessage);

            PyxNet.DLM.SignedMessage actualSignedMessage = 
                reader.ExtractXmlObject<PyxNet.DLM.SignedMessage>();
            NodeInfo signingNode = sender;
            try
            {
                signingNode = new NodeInfo(reader);
                Trace.WriteLine("Message was signed by {0}.", sender.FriendlyName);
            }
            catch
            {
                // This message was signed by an old version of the runtime.  Assume the sender signed it.
                Trace.WriteLine("Assuming message was signed by {0}.", sender.FriendlyName);
            }

            if (!PyxNet.DLM.RSATool.VerifySignedMessage(actualSignedMessage, signingNode.PublicKey))
            {
                throw new ArgumentException(String.Format(
                    "Signature verification failed from node {0}", signingNode.NodeGUID));
            }
            Message actualMessage = new Message(actualSignedMessage.rawMessage);
            return actualMessage;
        }
    }
}
