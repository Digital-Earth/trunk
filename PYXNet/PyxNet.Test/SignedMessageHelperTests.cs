using System;

namespace PyxNet.Test
{
    [NUnit.Framework.TestFixture]
    public class SignedMessageHelperTests
    {
        [NUnit.Framework.Test]
        public void SignedMessages()
        {
            PyxNet.Test.MessageTests.RandomMessage message = 
                new PyxNet.Test.MessageTests.RandomMessage();
            PyxNet.DLM.RSATool signer = new PyxNet.DLM.RSATool();
            NodeInfo sendersPublicKey = new NodeInfo();
            sendersPublicKey.PublicKey = signer.PublicKey;

            Message signedMessage = SignedMessageHelper.SignMessage(message, sendersPublicKey, signer);
            Message verifiedMessage = SignedMessageHelper.VerifySignedMessage(
                signedMessage, sendersPublicKey);
            NUnit.Framework.Assert.AreEqual(message.Bytes.Array, verifiedMessage.Bytes.Array);

            byte[] rawSignature = SignedMessageHelper.GenerateSignature(
                message.Bytes.Array, signer);
            NUnit.Framework.Assert.IsTrue(SignedMessageHelper.VerifySignature(
                message.Bytes.Array, rawSignature, sendersPublicKey.PublicKey));
        }

        // put this test in to play with timing of signing messages
        // [NUnit.Framework.Test]
        public void SignedMessagesTimer()
        {
            long signedTicks = 0;
            long verifyTicks = 0;
            long signedSize = 0;
            long verifySize = 0;

            PyxNet.DLM.RSATool signer = new PyxNet.DLM.RSATool();
            NodeInfo sendersPublicKey = new NodeInfo();
            sendersPublicKey.PublicKey = signer.PublicKey;

            for (int count = 0; count < 100; count++)
            {
                PyxNet.Test.MessageTests.RandomMessage message =
                    new PyxNet.Test.MessageTests.RandomMessage(1000);

                DateTime startEncrypt = DateTime.Now;
                Message signedMessage = SignedMessageHelper.SignMessage(message, 
                    sendersPublicKey, signer);
                signedTicks += (DateTime.Now.Ticks - startEncrypt.Ticks);
                signedSize += signedMessage.Length;

                DateTime startDecrypt = DateTime.Now;
                Message verifiedMessage = SignedMessageHelper.VerifySignedMessage(
                    signedMessage, sendersPublicKey);
                verifyTicks += (DateTime.Now.Ticks - startDecrypt.Ticks);

                verifySize += verifiedMessage.Length;
            }

            // second method to time as verification
            PyxNet.Test.MessageTests.RandomMessage messageB =
                new PyxNet.Test.MessageTests.RandomMessage(1000);
            DateTime startEncryptB = DateTime.Now;
            for (int count = 0; count < 100; count++)
            {
                Message signedMessage = SignedMessageHelper.SignMessage(messageB, sendersPublicKey, signer);
            }
            // multiSignedTicks should be about equivelent to signedTicks
            long multiSignedTicks = (DateTime.Now.Ticks - startEncryptB.Ticks);
        }
    }
}