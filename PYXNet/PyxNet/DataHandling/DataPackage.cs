/******************************************************************************
DataPackage.cs

begin      : 13/02/2007 11:48:14 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// A helper class for PyxNet Messages that contains a byte[] payload of data
    /// and can be transferred to and from PyxNet Messages.  The message storage format
    /// can be compressed or not, and the data can be check summed to verify transmission.
    /// </summary>
    public class DataPackage
    {
        #region Properties and Member Variables
        /// <summary>
        /// Used to make this class thread safe when constructing from a message
        /// and placing it into a message.
        /// </summary>
        private object m_lockThis = new object();

        /// <summary>
        /// Storage for the data.
        /// </summary>
        private byte[] m_array;

        /// <summary>
        /// The Data which is wrapped in this package.
        /// </summary>
        public byte[] Data
        {
            get { return m_array; }
            set
            {
                lock (m_lockThis)
                {
                    m_array = value;
                    StoreMD5();
                }
            }
        }

        /// <summary>
        /// Storage for the check sum on/off control.
        /// </summary>
        private bool m_doCheckSum = true;

        /// <summary>
        /// Controls if this data package will use check sums to verify the data.
        /// </summary>
        public bool DoCheckSum
        {
            get { return m_doCheckSum; }
            set 
            { 
                m_doCheckSum = value;
                StoreMD5();
            }
        }

        /// <summary>
        /// Storage for the compression on/off control.
        /// </summary>
        private bool m_useCompression = true;

        /// <summary>
        /// Controls if this data package will compress its data when transferring 
        /// to a message.
        /// </summary>
        public bool UseCompression
        {
            get { return m_useCompression; }
            set { m_useCompression = value; }
        }

        /// <summary>
        /// Storage for the MD5 checksum.
        /// </summary>
        byte[] m_md5Result = null;
        #endregion

        #region Helper Functions
        /// <summary>
        /// Helper function for remembering the MD5 checksum of the package data.
        /// </summary>
        private void StoreMD5()
        {
            if (DoCheckSum)
            {
                m_md5Result = CalculateMD5();
            }
            else
            {
                m_md5Result = null;
            }
        }

        /// <summary>
        /// Helper function for calculating the MD5 on the current contants of the data
        /// package.
        /// </summary>
        /// <returns></returns>
        private byte[] CalculateMD5()
        {
            if (m_array == null)
            {
                return null;
            }

            byte[] returnMD5 = null;

            System.Security.Cryptography.MD5CryptoServiceProvider md5 = null;
            try
            {
                // Create a managed md5 generator.
                md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
                // Generate the hash.
                returnMD5 = md5.ComputeHash(m_array, 0, m_array.Length);
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("MD5 error: " + e.Message);
            }
            finally
            {
                if (md5 != null)
                {
                    md5.Clear();
                    md5 = null;
                }
            }

            return returnMD5;
        }

        /// <summary>
        /// Compares the stored MD5 checksum, and the checksum of the current data
        /// and returns true if they are the same.  This is useful when the package
        /// has been constructed from a message (thus the stored MD5 check sum
        /// has been read from the message) and we wish to verify that the message
        /// was not damaged in transport.
        /// </summary>
        public bool ValidCheckSum
        {
            get
            {
                if (!DoCheckSum || null == m_md5Result)
                {
                    return true;
                }

                byte[] calculatedMD5 = CalculateMD5();
                if (null == calculatedMD5)
                {
                    return true;
                }

                if (m_md5Result.Length != calculatedMD5.Length)
                {
                    return false;
                }

                for (int index = 0; index < m_md5Result.Length; ++index)
                {
                    if (m_md5Result[index] != calculatedMD5[index])
                    {
                        return false;
                    }
                }

                return true;
            }
        }
        #endregion

        #region Construction
        /// <summary>
        /// Construct a package from an array of byte.  Holds on to a reference
        /// to the array of byte.  Default behaviour for compression and check sum
        /// which is enabled.
        /// </summary>
        /// <param name="data">The data to be wrapped in this package.</param>
        public DataPackage(byte[] data) : this (data, true, true)
        {
        }

        /// <summary>
        /// Construct a package from an array of byte while being able to 
        /// set the behaviour for check summing, and data compression.
        /// </summary>
        /// <param name="data">The data to be wrapped in this package.</param>
        /// <param name="useCompression">Set to true to enable data compression when transferring to a message.</param>
        /// <param name="doChecksum">Set to true to enable the check sum feature of packages.</param>
        public DataPackage(byte[] data, bool useCompression, bool doChecksum)
        {
            DoCheckSum = doChecksum;
            UseCompression = useCompression;
            Data = data;
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public DataPackage(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }
        #endregion

        #region To/From Message
        /// <summary>
        /// Append the DataPackage to an existing message.  
        /// This does not include any message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            lock (m_lockThis)
            {
                if (m_doCheckSum)
                {
                    message.Append((byte)1);
                    message.Append((int)m_md5Result.Length);
                    message.Append(m_md5Result);
                }
                else
                {
                    message.Append((byte)0);
                }

                message.Append((byte)(m_useCompression ? 1 : 0));
                if (m_useCompression)
                {
                    System.IO.MemoryStream compressedTable = new System.IO.MemoryStream();
                    // Use the newly created memory stream for the compressed data.
                    System.IO.Compression.GZipStream compressedzipStream =
                        new System.IO.Compression.GZipStream(compressedTable, System.IO.Compression.CompressionMode.Compress, true);
                    compressedzipStream.Write(m_array, 0, m_array.Length);
                    compressedzipStream.Close();
                    // Only send it compressed if it is smaller than when uncompressed. 
                    if (compressedTable.Length < m_array.Length)
                    {
                        message.Append((byte)1);
                        message.Append((int)m_array.Length);
                        message.Append((int)compressedTable.Length);
                        message.Append(compressedTable.GetBuffer(), 0, (int)compressedTable.Length);
                        return;
                    }
                }
                // send it uncompressed.
                message.Append((byte)0);
                message.Append((int)m_array.Length);
                message.Append(m_array);
            }
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a Data Package.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            lock (m_lockThis)
            {
                m_doCheckSum = (reader.ExtractByte() == 1);
                m_md5Result = null;
                if (m_doCheckSum)
                {
                    int md5Length = reader.ExtractInt();
                    m_md5Result = reader.ExtractBytes(md5Length);
                }

                m_useCompression = reader.ExtractByte() != 0;
                bool wasCompressed = reader.ExtractByte() != 0;
                int expendedLength = reader.ExtractInt();
                if (wasCompressed)
                {
                    try
                    {
                        int compressedLength = reader.ExtractInt();
                        byte[] compressedBytes = reader.ExtractBytes(compressedLength);
                        System.IO.MemoryStream compressedTable = new System.IO.MemoryStream();
                        compressedTable.Write(compressedBytes, 0, compressedLength);
                        compressedTable.Position = 0;
                        System.IO.Compression.GZipStream zipStream =
                            new System.IO.Compression.GZipStream(compressedTable, System.IO.Compression.CompressionMode.Decompress);
                        m_array = new byte[expendedLength];
                        zipStream.Read(m_array, 0, expendedLength);
                    }
                    catch
                    {
                        // error decoding compressed message.
                        // TODO: what do we do here??
                    }
                }
                else
                {
                    // no compression
                    m_array = reader.ExtractBytes(expendedLength);
                }
            }
        }
        #endregion
    }
}
