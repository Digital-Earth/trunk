using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Compression class allows you to apply compression and decompression to byte arrays.
    /// </summary>
    public static class Compression
    {
        /// <summary>
        /// a GZip compression on byte arrays.
        /// </summary>
        public static class GZip
        {
            private static byte[] ApplyGzip(byte[] data, CompressionMode mode)
            {
                using (MemoryStream memory = new MemoryStream())
                {
                    using (GZipStream gzip = new GZipStream(memory, CompressionMode.Compress, true))
                    {
                        gzip.Write(data, 0, data.Length);
                    }
                    return memory.ToArray();
                }
            }

            /// <summary>
            /// Compress the given data.
            /// </summary>
            /// <param name="data">Data to compres.s</param>
            /// <returns>GZip compressed data.</returns>
            public static byte[] Compress(byte[] data)
            {
                return ApplyGzip(data, CompressionMode.Compress);
            }

            /// <summary>
            /// Decompress gzipped data.
            /// </summary>
            /// <param name="compressedData">GZip compressed byte array.</param>
            /// <returns>The original data after decompression.</returns>
            public static byte[] Decompress(byte[] compressedData)
            {
                return ApplyGzip(compressedData, CompressionMode.Decompress);
            }
        }
    }
}
