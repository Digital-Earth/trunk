using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;
using System.Security.Cryptography;
using System.IO;

namespace PyxNet.DLM
{
    /// <summary>
    /// Encapsulates a public key. 
    /// </summary>
    [Serializable]
    [DataContract]
    public class PublicKey
    {
        private byte[] m_key;

        /// <summary>
        /// Binary representation of the actual key.  Not for "direct" use.
        /// </summary>
        [DataMember(Name = "Key")]
        public byte[] Key
        {
            get { return m_key; }
            set { m_key = value; }
        }

        public PublicKey(byte[] key)
        {
            m_key = key;
        }

        /// <summary>
        /// For serialization.
        /// </summary>
        public PublicKey()
        {
        }

        #region Equality
        public override bool Equals(object obj)
        {
            return Equals(obj as PublicKey);
        }

        public bool Equals(PublicKey other)
        {
            if (other == null) return false;
            if ((other.Key == null) && (Key == null))
                return true;  // Null keys are equal!
            if ((other.Key == null) || (Key == null))
                return false;
            if (other.Key.Length != Key.Length)
                return false;
            for (int i = 0; i < Key.Length; ++i)
            {
                if (other.Key[i] != Key[i])
                    return false;
            }
            return true;
        }

        private static int GetHashCode(byte[] list)
        {
            int hashCode = 0;
            long workHashCode = 0;

            if (list != null)
            {
                for (int counter = 0; counter < list.Length; counter++)
                {
                    workHashCode = (workHashCode << (counter % 4)) +
                               (int)list[counter];
                }
                workHashCode = workHashCode % (127);
            }
            hashCode = (int)workHashCode;

            return (hashCode);
        }

        public override int GetHashCode()
        {
            return GetHashCode(Key);
        }
        #endregion /* Equality */
    }

    namespace Test
    {
        using NUnit.Framework;

        [TestFixture]
        public class PublicKeyTests
        {
            [Test]
            public void Equality()
            {
                PrivateKey k1 = new PrivateKey();
                PrivateKey k2 = new PrivateKey();
                Assert.AreNotEqual(k1.PublicKey, k2.PublicKey);
                Assert.AreNotEqual(k1.PublicKey.GetHashCode(), k2.PublicKey.GetHashCode());
                PublicKey pubKey = new PublicKey(k1.PublicKey.Key);
                Assert.AreEqual(k1.PublicKey, pubKey);
                Assert.AreEqual(k1.PublicKey.GetHashCode(), pubKey.GetHashCode());
            }
        }
    }
}
