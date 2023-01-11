using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;
using System.Security.Cryptography;
//using System.Runtime.InteropServices;
using System.IO;

namespace PyxNet.DLM
{
    /// <summary>
    /// Encapsulates a private key.  The private key also embeds the 
    /// corresponding public key.
    /// </summary>
    [Serializable]
    public class PrivateKey
    {
#if DEBUG
        private class KeyDefinition{
            public PrivateKey m_Private;
            public PublicKey m_Public;
            string m_Description;

            public string Description
            {
                get { return m_Description; }
                set { m_Description = value; }
            }
            public KeyDefinition( PrivateKey k, string description)
            {
                m_Private = k;
                m_Public = k.PublicKey;
                m_Description = description;
            }
        };
        private static System.Collections.Generic.Dictionary<PublicKey,KeyDefinition> s_KnownKeys = 
            new Dictionary<PublicKey,KeyDefinition>();

        static private string DumpBytes(PublicKey k)
        {
            string response = "";
            for (int index = 16; index < 32; ++index)
            {
                if (response.Length > 0)
                    response += ", ";
                response += k.Key[index].ToString();
            }
            return response;
        }

        static public string GetKeyDescription(PrivateKey k)
        {
            try
            {
                if (s_KnownKeys.ContainsKey(k.PublicKey))
                {
                    if (!s_KnownKeys[k.PublicKey].m_Private.Equals( k))
                    {
                        System.Diagnostics.Trace.WriteLine( String.Format("Agggg! It appears that private key matches with {0}, but it should be {1}.",
                            GetKeyDescription( k.PublicKey), 
                            GetKeyDescription( k, true)));
                    }
                    return s_KnownKeys[k.PublicKey].Description + " (private) " + DumpBytes( k.PublicKey);
                }
            }
            catch (System.Collections.Generic.KeyNotFoundException)
            { }
            return "Unknown private key";
        }
        static public string GetKeyDescription(PublicKey k)
        {
            try
            {
                if (s_KnownKeys.ContainsKey(k))
                {
                    if (!s_KnownKeys[k].m_Public.Equals( k))
                    {
                        System.Diagnostics.Trace.WriteLine("Agggg!");
                    }
                    return s_KnownKeys[k].Description + " (public) " + DumpBytes(k);
                }
            }
            catch (System.Collections.Generic.KeyNotFoundException)
            { }
            return "Unknown public key";
        }
        static public string GetKeyDescription(PrivateKey k, bool manualLookup)
        {
            foreach (KeyDefinition d in s_KnownKeys.Values)
            {
                if (d.m_Private.Equals(k))
                    return d.Description + "(private!) " + DumpBytes(k.PublicKey);
            }
            return "Unknown key!";
        }
#else
        static public string GetKeyDescription( PrivateKey k)
        {
            return "Unknown private key (diagnostics disabled.)";
        }
        static public string GetKeyDescription( PublicKey k)
        {
            return "Unknown public key (diagnostics disabled.)";
        }
#endif

        [System.Diagnostics.Conditional("DEBUG")]
        public void AssignKeyName(string description)
        {
#if DEBUG
            if (s_KnownKeys.ContainsKey(this.PublicKey))
            {
                System.Diagnostics.Trace.WriteLine("Agggg!");
            }
            s_KnownKeys.Add(this.PublicKey, new KeyDefinition(this, description));
            if (!s_KnownKeys.ContainsKey(this.PublicKey))
            {
                System.Diagnostics.Trace.WriteLine("Agggg!");
            }
#endif
        }

        private readonly RSAParameters m_key;

        /// <summary>
        /// The actual key held in the PrivateKey.  Implementation dependent.
        /// </summary>
        public RSAParameters Key
        {
            get { return m_key; }
        }

        /// <summary>
        /// Create a private key from an RSAParameters instance.
        /// </summary>
        /// <param name="key"></param>
        public PrivateKey(RSAParameters key)
        {
            m_key = key;
        }

        /// <summary>
        /// Creates a new (random) private key.
        /// </summary>
        public PrivateKey()
        {
            RSATool tool = new RSATool();
            m_key = tool.GetPrivateKey().Key;
        }

        /// <summary>
        /// Construct a private key from a key container.
        /// </summary>
        /// <param name="keyContainerName">The name of the key container.</param>
        public PrivateKey(String keyContainerName)
        {
            CspParameters cspParameters = new CspParameters();
            cspParameters.KeyContainerName = keyContainerName;

            RSATool tool = new RSATool(cspParameters);
            m_key = tool.GetPrivateKey().Key;
        }

        /// <summary>
        /// Extracts the public portion of the key.
        /// </summary>
        public PublicKey PublicKey
        {
            get
            {
                RSATool tool = new RSATool();
                tool.SetPrivateKey(this);
                return tool.PublicKey;
            }
        }
    }
}
