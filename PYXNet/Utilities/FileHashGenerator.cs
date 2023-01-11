/******************************************************************************
FileHashGenerator.cs

begin      : Dec 18, 2006
copyright  : (c) 2006 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.IO;
using System.Text;

namespace FileHash
{
    /// <summary>
    /// Generates hashes for a specified file.
    /// </summary>
    public class FileHashGenerator
    {
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

        /// <summary>
        /// Return the md5 hash sum for the given file.
        /// </summary>
        public byte[] GenerateMD5()
        {
            byte[] md5Result = null;

            FileStream fileStream = null;
            System.Security.Cryptography.MD5CryptoServiceProvider md5 = null;

            try
            {
                // Create a managed md5 generator.
                md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();

                // Create a file stream for the given file.
                fileStream = new FileStream(m_fileName, FileMode.Open, FileAccess.Read);

                // Generate the hash.
                md5Result = md5.ComputeHash(fileStream);
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("MD5 error: " + e.Message);
            }
            finally
            {
                if (fileStream != null)
                {
                    fileStream.Close();
                }
                if (md5 != null)
                {
                    md5.Clear();
                }
            }

            return md5Result;
        }

        /// <summary>
        /// Returns the hash code as a 32-character string
        /// based in hexadecimal.
        /// </summary>
        /// <returns>The MD5 hash is string format.</returns>
        public static String MD5ToHex(byte[] hash)
        {
            StringBuilder strBuilder = new StringBuilder();
            for (int index = 0; index < hash.Length; index++)
            {
                strBuilder.Append(hash[index].ToString("X2"));
            }
            return strBuilder.ToString();
        }
    }
}
