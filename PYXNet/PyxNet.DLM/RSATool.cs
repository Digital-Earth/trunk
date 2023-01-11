using System;
using System.Collections.Generic;
using System.Text;
using System.Security.Cryptography;
using NUnit.Framework;
using System.IO;

namespace PyxNet.DLM
{
    /// <summary>
    /// Utility class to encapsulate an encoded message.
    /// </summary>
    [Serializable]
    public class CipherMessage
    {
        public byte[] cipherBytes;  // RC2 encrypted message text
        public byte[] rc2Key;       // RSA encrypted rc2 key
        public byte[] rc2IV;        // RC2 initialization vector
    }

    /// <summary>
    /// Utility class to encapsulate a signed message.
    /// </summary>
    /// <remarks>
    /// Consider adding the identity of the signer?
    /// </remarks>
    [Serializable]
    public class SignedMessage
    {
        public byte[] rawMessage;
        public byte[] signature;
    }

    /// <summary>
    /// RSATool provides a simple wrapper of RSACryptoProvider's functionality.
    /// </summary>
    public class RSATool
    {
        private static Pyxis.Utilities.TraceTool s_Log = 
            new Pyxis.Utilities.TraceTool(false);

        /// <summary>
        /// For internal use only.  We don't expose the actual provider,
        /// or the RC2 encoder.
        /// </summary>
        private RSACryptoServiceProvider m_rsa;
        private RC2CryptoServiceProvider m_rc2;

        /// <summary>
        /// Maximum key size for the RC2 algorithm
        /// </summary>
        const int keySize = 128;

        /// <summary>
        /// Default ctor creates a random key.
        /// </summary>
        public RSATool()
        {
            m_rsa = new RSACryptoServiceProvider();
            m_rc2 = new RC2CryptoServiceProvider();
            m_rc2.KeySize = keySize;
        }

        /// <summary>
        /// Create a tool with the specified key.
        /// </summary>
        /// <remarks>
        /// TODO: Can usage of this be replaced with the key container form?
        /// </remarks>
        /// <param name="key"></param>
        public RSATool( PrivateKey key): this()
        {
            /// TODO: This is inefficient.  We are generating a random key, then 
            /// over-writing it.
            SetPrivateKey(key);
        }

        /// <summary>
        /// Create using a key container.
        /// </summary>
        /// <param name="cspParameters">The parameters indicating the key container.</param>
        public RSATool(CspParameters cspParameters)
        {
            m_rsa = new RSACryptoServiceProvider(cspParameters);
            m_rc2 = new RC2CryptoServiceProvider();
            m_rc2.KeySize = keySize;
        }

        /// <summary>
        /// Delete the key in the specified key container.
        /// </summary>
        /// <param name="cspParameters">The key container parameters.</param>
        public static void DeleteKey(CspParameters cspParameters)
        {
            // Create a new instance of RSACryptoServiceProvider that accesses
            // the key container.
            RSACryptoServiceProvider rsa = new RSACryptoServiceProvider(cspParameters);

            // Delete the key entry in the container.
            rsa.PersistKeyInCsp = false;

            // Call Clear to release resources and delete the key from the container.
            rsa.Clear();
        }

        #region Keys

        /// <summary>
        /// Set a specific private key for this tool to use for decrypting
        /// and signing messages.
        /// </summary>
        /// <remarks>
        /// This is a method, rather than a property, to indicate that it is a heavyweight operation.
        /// </remarks>
        /// <param name="key"></param>
        public void SetPrivateKey(PrivateKey key)
        {
            m_rsa.ImportParameters(key.Key);
        }

        /// <summary>
        /// Used to extract the rsa private key.  Danger!
        /// This is expected to be used so that we can store the key between
        /// runs.
        /// </summary>
        /// <remarks>
        /// This is a method, rather than a property, to indicate that it is a heavyweight operation.
        /// There is no need to save the public key separately, since
        /// it is embedded in the private key.
        /// </remarks>
        /// <returns></returns>
        public PrivateKey GetPrivateKey()
        {
            try
            {
                RSAParameters theKey = m_rsa.ExportParameters(true);
                return new PrivateKey(theKey);
            }
            catch (CryptographicException e)
            {
                s_Log.WriteLine("Failed to GetPrivateKey: {0}", e.Message);
                throw;
            }
        }

        /// <summary>
        /// Provides the public key for this tool.  Using this public key,
        /// anyone can send a secure message that only this node can 
        /// decode with it's private key.
        /// </summary>
        public PublicKey PublicKey
        {
            get
            {
                byte[] result = m_rsa.ExportCspBlob(false);
                return new PublicKey(result);
            }
        }

        #endregion Keys

        #region Encryption/Decryption

        /// <summary>
        /// Utility function for encrypting the given plainText message
        /// with the specified public key.
        /// </summary>
        /// <param name="plainText"></param>
        /// <param name="key">The public key of the intended recipient.</param>
        /// <returns>An encrypted message.</returns>
        public static CipherMessage EncryptMessage(string plainText,
            PublicKey key)
        {
            // Convert string to a byte array
            byte[] plainBytes = Encoding.Unicode.GetBytes(plainText.ToCharArray());
            return EncryptMessage(plainBytes, key);
        }

        /// <summary>
        /// Utility function for encrypting a given buffer
        /// with the specified public key.
        /// </summary>
        /// <param name="plainBytes"></param>
        /// <param name="key">The public key of the intended recipient.</param>
        /// <returns>An encrypted message.</returns>
        public static CipherMessage EncryptMessage(byte[] plainBytes,
            PublicKey key)
        {
            CipherMessage message = new CipherMessage();

            RSACryptoServiceProvider rsa = new RSACryptoServiceProvider();
            rsa.ImportCspBlob(key.Key);
            RC2CryptoServiceProvider rc2 = new RC2CryptoServiceProvider();
            rc2.KeySize = keySize;

            // A new key and iv are generated for every message
            rc2.GenerateKey();
            rc2.GenerateIV();

            // The rc2 initialization doesnt need to be encrypted, but will
            // be used in conjunction with the key to decrypt the message.
            message.rc2IV = rc2.IV;
            try
            {
                // Encrypt the RC2 key using RSA encryption
                message.rc2Key = rsa.Encrypt(rc2.Key, true);
            }
            catch (CryptographicException e)
            {
                // The High Encryption Pack is required to run this code
                // because we are using a 128-bit key. 
                s_Log.WriteLine("Encryption Failed. Ensure that the" +
                  " High Encryption Pack is installed. {0}", e.Message);
                throw;
            }

            // Encrypt the Text Message using RC2 (Symmetric algorithm)
            ICryptoTransform sse = rc2.CreateEncryptor();
            MemoryStream ms = new MemoryStream();
            CryptoStream cs = new CryptoStream(ms, sse, CryptoStreamMode.Write);
            try
            {
                cs.Write(plainBytes, 0, plainBytes.Length);
                cs.FlushFinalBlock();
                message.cipherBytes = ms.ToArray();
            }
            catch (Exception e)
            {
                s_Log.WriteLine("Failed to encrypt message: {0}", e.Message);
                throw;
            }
            finally
            {
                ms.Close();
                cs.Close();
            }

            return message;
        } // method EncryptMessage

        /// <summary>
        /// Decrypts the given message with the tool's private key.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>The decrypted buffer.  Throws on error.</returns>
        public byte[] DecryptMessage(CipherMessage message)
        {
            // Get the RC2 Key and Initialization Vector
            m_rc2.IV = message.rc2IV;
            try
            {
                // Try decrypting the rc2 key
                m_rc2.Key = m_rsa.Decrypt(message.rc2Key, true);
            }
            catch (CryptographicException e)
            {
                s_Log.WriteLine("Decryption Failed: {0}", e.Message);
                throw;
            }

            ICryptoTransform ssd = m_rc2.CreateDecryptor();

            // Put the encrypted message in a memorystream
            MemoryStream ms = new MemoryStream(message.cipherBytes);

            // The CryptoStream will read cipher text from the MemoryStream
            CryptoStream cs = new CryptoStream(ms, ssd, CryptoStreamMode.Read);
            byte[] initialText = new Byte[message.cipherBytes.Length];

            try
            {
                // Decrypt the message and store in byte array
                int decryptedLength = cs.Read(initialText, 0, initialText.Length);

                // There's gotta be a better way to truncate our result!
                byte[] result = new byte[decryptedLength];
                for (int i = 0; i < decryptedLength; ++i)
                {
                    result[i] = initialText[i];
                }
                return result;
            }
            catch (Exception e)
            {
                // TODO: Catch specific exceptions here...
                s_Log.WriteLine("Decrypt Failed: {0}", e.Message);                
                throw;
            }
            finally
            {
                ms.Close();
                cs.Close();
            }
        }


        /// <summary>
        /// Decrypts the given message with the tools private key.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>The decrypted buffer.  Throws on error.</returns>
        public string DecryptMessageToString(CipherMessage message)
        {
            // First decrypt the message
            byte[] rawDecryptedMessage = DecryptMessage( message);

            string result = Encoding.Unicode.GetString(rawDecryptedMessage, 0, rawDecryptedMessage.Length);
            return result;
        }

        #endregion Encryption/Decryption

        #region Signing Messages

        /// <summary>
        /// Generates a signature for the given message.
        /// </summary>
        /// <param name="message">The raw data (will be signed).</param>
        /// <returns>The signature for the given message.</returns>
        public byte[] GenerateSignature(byte[] message)
        {
            try
            {
                return m_rsa.SignData(message,
                    new SHA1CryptoServiceProvider());
            }
            catch (CryptographicException e)
            {
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "Failed to sign message: {0}", e.Message));
                return null;
            }
        }

        /// <summary>
        /// Signs the given message using the tools private key.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>
        /// A signed message that can be verified with the tool's public key.
        /// </returns>
        public SignedMessage SignMessage(byte[] message)
        {
            SignedMessage response = new SignedMessage();
            response.rawMessage = message;
            response.signature = GenerateSignature( response.rawMessage);
            return response;
        }

        /// <summary>
        /// Signs the given message using the tools private key.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>
        /// A signed message that can be verified with the tool's public key.
        /// </returns>
        public SignedMessage SignMessage(String message)
        {
            return SignMessage( Encoding.Unicode.GetBytes(message.ToCharArray()));
        }

        /// <summary>
        /// Verifies that the given signature is correct for the given message.
        /// </summary>
        /// <param name="messageData"></param>
        /// <param name="messageSignature"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool VerifySignature( byte[] messageData, byte[] messageSignature,
            PublicKey key)
        {
            RSACryptoServiceProvider rsa = new RSACryptoServiceProvider();
            rsa.ImportCspBlob(key.Key);
            try
            {
                return rsa.VerifyData(messageData, new SHA1CryptoServiceProvider(),
                    messageSignature);
            }
            catch (CryptographicException e)
            {
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "Failed to verify sign message: {0}", e.Message));

                return false;
            }
        }

        /// <summary>
        /// Verifies that the given signature is correct for the given message.
        /// </summary>
        /// <param name="messageData"></param>
        /// <param name="messageSignature"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool VerifySignature(byte[] messageData, byte[] messageSignature,
            RSATool key)
        {
            try
            {
                return key.m_rsa.VerifyData( messageData, new SHA1CryptoServiceProvider(),
                    messageSignature);
            }
            catch (CryptographicException e)
            {
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "Failed to verify sign message: {0}", e.Message));

                return false;
            }
        }
        /// <summary>
        /// Verifies that a given message was signed with the private key 
        /// which corresponds to the given public key.
        /// </summary>
        /// <param name="message"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        public static bool VerifySignedMessage(SignedMessage message, PublicKey key)
        {
            return VerifySignature(message.rawMessage, message.signature, key);
        }

// TODO: Comment
        public static byte[] GenerateSignature(byte[] message, PrivateKey key)
        {
            RSATool tool = new RSATool(key);
            return tool.GenerateSignature(message);
        }
        #endregion Signing Messages
    }

    namespace Test
    {
        /// <summary>
        /// Helper class that generates a random block of bytes.  Supports 
        /// Equals.  Intended for testing purposes.
        /// </summary>
        internal class RandomBlock
        {
            private static Random r = new Random(3010);
            private static byte[] GenerateBytes()
            {
                byte[] buffer = new byte[r.Next(100)];
                r.NextBytes(buffer);
                return buffer;
            }

            private byte[] m_bytes = GenerateBytes();

            public byte[] Bytes
            {
                get { return m_bytes;}
                set { m_bytes = value; }
            }

            // override object.Equals 
            public override bool Equals(object obj)
            {
                RandomBlock objAsBlock = obj as RandomBlock;
                if (objAsBlock == null)
                {
                    return false;
                }

                return Bytes.Equals(objAsBlock.Bytes);
            }

            // override object.GetHashCode (To suppress warnings)
            public override int GetHashCode()
            {
                throw new Exception("The method or operation is not implemented.");
            }
        }

        /// <summary>
        /// RSAToolTest is a collection of unit tests for the RSATool class.
        /// </summary>
        //[TestFixture]
        public class RSAToolTest
        {
            /// <summary>
            /// Encrypt a piece of text using RSATool.  Verify that encryption does 
            /// _something_ to the data, and that decryption gets the original back.
            /// </summary>
            [Test]
            public void SimpleEncryption()
            {
                // Encrypt a string.
                string rawBuffer = "The rain in spain falls mainly in the plain.";
                RSATool testee = new RSATool();
                CipherMessage encryptedBuffer = RSATool.EncryptMessage(rawBuffer, testee.PublicKey);

                // Verify that it was actually changed.
                string temp = Encoding.Unicode.GetString(encryptedBuffer.cipherBytes, 0,
                    encryptedBuffer.cipherBytes.Length).TrimEnd('\0');
                Assert.AreNotEqual(rawBuffer, temp,
                    "Encryption should change the data.");

                // Verify that we can decrypt it.
                string decryptedBuffer = testee.DecryptMessageToString(encryptedBuffer);
                Assert.AreEqual(decryptedBuffer, rawBuffer,
                    "Decryption should produce original data.");
            }

            /// <summary>
            /// Tests that a "key" can be extracted from one node (the server)
            /// and used by a second node (the client).
            /// </summary>
            [Test]
            public void TransmitPublicKey()
            {
                // The server holds both sides of the key.
                RSATool server = new RSATool();
                // The public key is "transmitted"
                PublicKey key = server.PublicKey;

                string rawBuffer = "Another simple test";

                // From a client, create a data block and send it.
                CipherMessage encryptedBuffer = RSATool.EncryptMessage(rawBuffer, key);

                // Third logic block - use correct key to decrypt data
                string decryptedData = server.DecryptMessageToString(encryptedBuffer);

                Assert.AreEqual(decryptedData, rawBuffer,
                    "We should be able to decrypt with the correct key.");
            }

            /// <summary>
            /// Tests that a "key" can be extracted from one node (the server)
            /// and used by a second server.  Note that this is what we will 
            /// do when "restarting" the server.
            /// </summary>
            [Test]
            public void TransmitPrivateKey()
            {
                // The server holds both sides of the key.
                RSATool server = new RSATool();
                // The public key is "transmitted"
                PublicKey publicKey = server.PublicKey;
                PrivateKey privateKey = server.GetPrivateKey();

                string rawBuffer = "Another simple test";

                // From a client, create a data block and send it.
                CipherMessage encryptedBuffer = RSATool.EncryptMessage(rawBuffer, publicKey);

                // Decrypt data in original server.
                string decryptedData = server.DecryptMessageToString(encryptedBuffer);

                Assert.AreEqual(decryptedData, rawBuffer,
                    "We should be able to decrypt with the correct key.");

                RSATool secondServer = new RSATool();
                secondServer.SetPrivateKey(privateKey);

                // Decrypt data in another server instance.
                string decryptedData2 = secondServer.DecryptMessageToString(encryptedBuffer);

                Assert.AreEqual(decryptedData2, rawBuffer,
                    "We should be able to decrypt a second time with the correct key.");

                RSATool thirdServer = new RSATool( privateKey);

                // Decrypt data in another server instance.
                string decryptedData3 = thirdServer.DecryptMessageToString(encryptedBuffer);

                Assert.AreEqual(decryptedData3, rawBuffer,
                    "We should be able to decrypt a second time with the correct key.");
            }

            /// <summary>
            /// Verifies that the same key is retrieved from the same key container every time.
            /// </summary>
            [Test]
            public void TestKeyContainer()
            {
                CspParameters cspParameters = new CspParameters();
                cspParameters.KeyContainerName = Guid.NewGuid().ToString();

                // Get the key from the container.  This call should create it.
                RSATool tool = new RSATool(cspParameters);
                PublicKey publicKey = tool.PublicKey;

                RSATool anotherTool = new RSATool(cspParameters);
                PublicKey anotherPublicKey = anotherTool.PublicKey;

                Assert.AreEqual(publicKey, anotherPublicKey, 
                    "Each retrieval from the key container should result in the same key.");

                RSATool.DeleteKey(cspParameters);
            }

            /// <summary>
            /// Verifies that the DeleteKey method works.
            /// </summary>
            [Test]
            public void TestDeleteKey()
            {
                CspParameters cspParameters = new CspParameters();
                cspParameters.KeyContainerName = Guid.NewGuid().ToString();

                // Get the key from the container.  This call should create it.
                RSATool tool = new RSATool(cspParameters);
                PublicKey publicKey = tool.PublicKey;

                RSATool.DeleteKey(cspParameters);

                // Get the key from the container.  This call should create it.
                RSATool anotherTool = new RSATool(cspParameters);
                PublicKey anotherPublicKey = anotherTool.PublicKey;

                Assert.AreNotEqual(publicKey, anotherPublicKey,
                    "The deletion should result in a different key being returned.");

                RSATool.DeleteKey(cspParameters);
            }

            /// <summary>
            /// Verifies that decrypting with the wrong key throws a 
            /// CryptographicException.
            /// </summary>
            [Test]
            [ExpectedException(typeof(CryptographicException))]
            public void CryptographicAttackerFails()
            {
                // The server holds both sides of the key.
                RSATool server = new RSATool();
                // The public key is "transmitted"
                PublicKey key = server.PublicKey;

                string rawBuffer = "Another simple test";

                // From a client, create a data block and send it.
                CipherMessage encryptedBuffer = RSATool.EncryptMessage(rawBuffer, key);

                // Second client attempts to decrypt data.  This must fail!
                RSATool attacker = new RSATool();
                string badDecryption = attacker.DecryptMessageToString(encryptedBuffer);
                // If we got here, we're in trouble!
            }

            /// <summary>
            /// Tests that a client can receive a signed message from a server, and
            /// validate its signature.
            /// </summary>
            [Test]
            public void VerifyValidSignature()
            {
                RSATool server = new RSATool();
                // Test GenerateSignature
                {
                    byte[] message = new byte[] { 1, 2, 3, 4, 5, 6 };
                    byte[] signature = server.GenerateSignature(message);
                    Assert.IsTrue( RSATool.VerifySignature(message, signature, server));
                }

                // Test with a string message.
                {
                    string message = "This is a signed message.  No foolin.";
                    SignedMessage signature = server.SignMessage(message);

                    Assert.IsTrue(
                        RSATool.VerifySignedMessage(signature, server.PublicKey));
                }
            }

            /// <summary>
            /// Profile the signing operation.  Current times are about .16s.
            /// </summary>
            //[Test]
            public void SignatureTiming()
            {
                RSATool server = new RSATool();
                // Test GenerateSignature
                {
                    int loopcount = 100;

                    byte[] message = new byte[] { 1, 2, 3, 4, 5, 6 };
                    DateTime start = DateTime.Now;
                    for (int i = 0; i < loopcount; ++i)
                    {
                        byte[] junkedsignature = server.GenerateSignature(message);
                    }
                    TimeSpan duration = DateTime.Now - start;
                    System.Diagnostics.Trace.WriteLine( string.Format("Test of signatures took {0}.", duration.TotalMilliseconds.ToString()));

                    start = DateTime.Now;
                    for (int k = 0; k < loopcount; ++k)
                    {
                        SHA1CryptoServiceProvider dummy = new SHA1CryptoServiceProvider();
                    }
                    duration = DateTime.Now - start;
                    System.Diagnostics.Trace.WriteLine( string.Format("Generation of sha1 took {0}.", duration.TotalMilliseconds.ToString()));

                    RSACryptoServiceProvider m_rsa = new RSACryptoServiceProvider();
                    SHA1CryptoServiceProvider provider = new SHA1CryptoServiceProvider();
                    start = DateTime.Now;
                    for (int k = 0; k < loopcount; ++k)
                    {
                        m_rsa.SignData( message, provider);
                    }
                    duration = DateTime.Now - start;
                    System.Diagnostics.Trace.WriteLine( string.Format("Second signature test took {0}.", duration.TotalMilliseconds.ToString()));

                    start = DateTime.Now;
                    byte[] signature = server.GenerateSignature(message);
                    for (int j = 0; j < loopcount; ++j)
                    {
                        Assert.IsTrue(RSATool.VerifySignature(message, signature, server));
                    }
                    duration = DateTime.Now - start;
                    System.Diagnostics.Trace.WriteLine( string.Format("Test of signatures verification took {0}.", duration.TotalMilliseconds.ToString()));
                }
            }

            /// <summary>
            /// Verifies that a "bad" signature fails to verify.
            /// </summary>
            [Test]
            public void BadSignatureFailsToVerify()
            {
                RSATool server = new RSATool();
                string message = "This is a signed message.  No foolin.";
                SignedMessage signature = server.SignMessage(message);

                RSATool differentKeyholder = new RSATool();
                // We don't use the correct key!
                Assert.IsFalse( RSATool.VerifySignedMessage(
                    signature, differentKeyholder.PublicKey));
            }

            /// <summary>
            /// Verifies that raw (binary) encryption and decryption operate correctly.
            /// </summary>
            [Test]
            public void BinaryEncryption()
            {
                // Encrypt and decrypt ten random blocks of data.
                for (int i = 0; i < 10; ++i)
                {
                    RandomBlock rawData = new RandomBlock();
                    RSATool testee = new RSATool();
                    CipherMessage encryptedBuffer = RSATool.EncryptMessage(rawData.Bytes, testee.PublicKey);

                    Assert.AreNotEqual(rawData.Bytes, encryptedBuffer.cipherBytes,
                        "Encryption should change the data.");
                    byte[] decryptedBuffer = testee.DecryptMessage(encryptedBuffer);
                    Assert.AreEqual(decryptedBuffer, rawData.Bytes,
                        "Decryption should produce original data.");
                }
            }
        }
    }
}
