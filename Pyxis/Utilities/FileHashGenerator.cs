/******************************************************************************
FileHashGenerator.cs

begin      : Dec 18, 2006
copyright  : (c) 2006 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace FileHash
{
    /// <summary>
    /// Generates hashes for a specified file.
    /// </summary>
    public class FileHashGenerator
    {
        // TODO: eventualy, this could get rolled into Pyxis.Utility.MD5Checksummer
        // and this class could go away.

        /// <summary>
        /// The file path.
        /// </summary>
        private string m_fileName;

        /// <summary>
        /// Constructs a hash generator object for the file.
        /// </summary>
        /// <param name="file">The file path.</param>
        public FileHashGenerator(string fileName)
        {
            m_fileName = fileName;
        }

        private byte[] GenerateHash<HashProvider>() where 
            HashProvider: System.Security.Cryptography.HashAlgorithm, new()
        {
            byte[] hashResult = null;

            FileStream fileStream = null;
            HashProvider hashFunction = default(HashProvider);

            try
            {
                // Create a managed sha256 generator.
                hashFunction = new HashProvider();

                // Create a file stream for the given file.
                fileStream = new FileStream(m_fileName, FileMode.Open, FileAccess.Read);

                // Generate the hash.
                hashResult = hashFunction.ComputeHash(fileStream);
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("Hash error: " + e.Message);
            }
            finally
            {
                if (fileStream != null)
                {
                    fileStream.Close();
                }
                if (hashFunction != null)
                {
                    hashFunction.Clear();
                }
            }

            return hashResult;
        }

        /// <summary>
        /// Return the md5 hash sum for the given file.
        /// </summary>
        public byte[] GenerateMD5_deprecated()
        {
            return GenerateHash<System.Security.Cryptography.MD5CryptoServiceProvider>();
        }

        /// <summary>
        /// Return the SHA256 hash sum for the given file.
        /// </summary>
        public byte[] GenerateSHA256()
        {
            return GenerateHash<System.Security.Cryptography.SHA256Managed>();
        }

        /// <summary>
        /// Returns the hash code as a 32-character string
        /// based in hexadecimal.
        /// </summary>
        /// <returns>The MD5 hash is string format.</returns>
        public static String MD5ToHex(byte[] hash)
        {
            if (hash == null)
            {
                return "";
            }

            StringBuilder strBuilder = new StringBuilder();
            for (int index = 0; index < hash.Length; index++)
            {
                strBuilder.Append(hash[index].ToString("X2"));
            }
            return strBuilder.ToString();
        }
    }
}
