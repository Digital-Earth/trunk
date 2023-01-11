using System;

namespace PyxNet.Test
{
    [NUnit.Framework.TestFixture]
    public class EncryptedMessageHelperTests
    {
        [NUnit.Framework.Test]
        public void EncryptedMessages()
        {
            PyxNet.Test.MessageTests.RandomMessage message = 
                new PyxNet.Test.MessageTests.RandomMessage();
            PyxNet.DLM.RSATool recipient = new PyxNet.DLM.RSATool();
            NodeInfo recipientsPublicKey = new NodeInfo();
            recipientsPublicKey.PublicKey = recipient.PublicKey;

            Message encryptedMessage =
                EncryptedMessageHelper.EncryptMessage(message, recipientsPublicKey);
            Message decryptedMessage = EncryptedMessageHelper.DecryptMessage(
                encryptedMessage, recipient);
            NUnit.Framework.Assert.AreEqual(message.Bytes.Array, decryptedMessage.Bytes.Array);
        }

        // put this test in to play with timing of encryption and decryption
        // [NUnit.Framework.Test]
        public void EncryptedMessagesTimer()
        {
            long encyrptTicks = 0;
            long decryptTicks = 0;
            long encryptSize = 0;
            long decryptSize = 0;

            PyxNet.DLM.RSATool recipient = new PyxNet.DLM.RSATool();
            NodeInfo recipientsPublicKey = new NodeInfo();
            recipientsPublicKey.PublicKey = recipient.PublicKey;

            for (int count = 0; count < 100; count++)
            {
                PyxNet.Test.MessageTests.RandomMessage message =
                    new PyxNet.Test.MessageTests.RandomMessage(128000);

                DateTime startEncrypt = DateTime.Now;
                Message encryptedMessage =
                    EncryptedMessageHelper.EncryptMessage(message, recipientsPublicKey);
                encyrptTicks += (DateTime.Now.Ticks - startEncrypt.Ticks);
                encryptSize += encryptedMessage.Length;

                DateTime startDecrypt = DateTime.Now;
                Message decryptedMessage = EncryptedMessageHelper.DecryptMessage(
                    encryptedMessage, recipient);
                decryptTicks += (DateTime.Now.Ticks - startDecrypt.Ticks);

                decryptSize += decryptedMessage.Length;
            }
        }
    }
}