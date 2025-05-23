// md4.cs
// Copyright (C) 2002 Matt Zyzik (www.FileScope.com)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

namespace MD4Hash
{
    using System;
    using System.Text;
    using System.IO;

    /// <summary>
    ///   Implements the MD4 message digest algorithm in C#
    /// </summary>
    /// <remarks>
    /// <p>
    /// <b>References:</b>
    /// <ol>
    ///   <li> Ronald L. Rivest,
    ///        "<a href="http://www.roxen.com/rfc/rfc1320.html">
    ///        The MD4 Message-Digest Algorithm</a>",
    ///        IETF RFC-1320 (informational).
    ///   </li>
    /// </ol>         
    /// </p>
    /// </remarks>
    public class MD4
    {
        // MD4 specific object variables
        //-----------------------------------------------------------------------

        /// <summary>
        /// The size in bytes of the input block to the transformation algorithm
        /// </summary>
        private const int BLOCK_LENGTH = 64;		// = 512 / 8

        /// <summary>
        /// 4 32-bit words (interim result)
        /// </summary>
        private uint[] context = new uint[4];

        /// <summary>
        /// Number of bytes procesed so far mod. 2 power of 64.
        /// </summary>
        private long count;

        /// <summary>
        /// 512-bit input buffer = 16 x 32-bit words holds until it reaches 512 bits
        /// </summary>
        private byte[] buffer = new byte[BLOCK_LENGTH];

        /// <summary>
        /// 512-bit work buffer = 16 x 32-bit words
        /// </summary>
        private uint[] X = new uint[16];

        // Constructors
        //------------------------------------------------------------------------

        public MD4()
        {
            engineReset();
        }

        /// <summary>
        /// This constructor is here to implement the clonability of this class
        /// </summary>
        /// <param name="md"> </param>
        private MD4(MD4 md)
            : this()
        {
            //this();
            context = (uint[])md.context.Clone();
            buffer = (byte[])md.buffer.Clone();
            count = md.count;
        }

        // Clonable method implementation
        //-------------------------------------------------------------------------

        public object Clone()
        {
            return new MD4(this);
        }

        // JCE methods
        //-------------------------------------------------------------------------

        /// <summary>
        /// Resets this object disregarding any temporary data present at the
        /// time of the invocation of this call.
        /// </summary>
        private void engineReset()
        {
            // initial values of MD4 i.e. A, B, C, D
            // as per rfc-1320; they are low-order byte first
            context[0] = 0x67452301;
            context[1] = 0xEFCDAB89;
            context[2] = 0x98BADCFE;
            context[3] = 0x10325476;
            count = 0L;
            for (int i = 0; i < BLOCK_LENGTH; i++)
            {
                buffer[i] = 0;
            }
        }

        /// <summary>
        /// Continues an MD4 message digest using the input byte
        /// </summary>
        /// <param name="b">byte to input</param>
        private void engineUpdate(byte b)
        {
            // compute number of bytes still unhashed; ie. present in buffer
            int i = (int)(count % BLOCK_LENGTH);
            count++;			// update number of bytes
            buffer[i] = b;
            if (i == BLOCK_LENGTH - 1)
            {
                transform(ref buffer, 0);
            }
        }

        /// <summary>
        /// MD4 block update operation
        /// </summary>
        /// <remarks>
        /// Continues an MD4 message digest operation by filling the buffer, 
        /// transform(ing) data in 512-bit message block(s), updating the variables
        /// context and count, and leaving (buffering) the remaining bytes in buffer
        /// for the next update or finish.
        /// </remarks>
        /// <param name="input">input block</param>
        /// <param name="offset">start of meaningful bytes in input</param>
        /// <param name="len">count of bytes in input blcok to consider</param>
        private void engineUpdate(byte[] input, int offset, int len)
        {
            // make sure we don't exceed input's allocated size/length
            if (offset < 0 || len < 0 || (long)offset + len > input.Length)
            {
                throw new ArgumentOutOfRangeException();
            }

            // compute number of bytes still unhashed; ie. present in buffer
            int bufferNdx = (int)(count % BLOCK_LENGTH);
            count += len;		// update number of bytes
            int partLen = BLOCK_LENGTH - bufferNdx;
            int i = 0;
            if (len >= partLen)
            {
                Array.Copy(input, offset + i, buffer, bufferNdx, partLen);

                transform(ref buffer, 0);
                for (i = partLen; i + BLOCK_LENGTH - 1 < len; i += BLOCK_LENGTH)
                {
                    transform(ref input, offset + i);
                }
                bufferNdx = 0;
            }

            // buffer remaining input
            if (i < len)
            {
                Array.Copy(input, offset + i, buffer, bufferNdx, len - i);
            }
        }

        private void engineUpdate(FileStream fs)
        {
            try
            {
                int bufferNdx = (int)(count % BLOCK_LENGTH);
                count += fs.Length;
                int partLen = BLOCK_LENGTH - bufferNdx;
                int i = 0;
                if (fs.Length >= partLen)
                {
                    fs.Seek(0, SeekOrigin.Begin);
                    fs.Read(buffer, bufferNdx, partLen);
                    transform(ref buffer, 0);
                    byte[] chunk = new byte[64];
                    for (i = partLen; i + BLOCK_LENGTH - 1 < fs.Length; i += BLOCK_LENGTH)
                    {
                        fs.Read(chunk, 0, BLOCK_LENGTH);
                        transform(ref chunk, 0);
                    }
                    bufferNdx = 0;
                }
                if (i < fs.Length)
                {
                    fs.Read(buffer, bufferNdx, (int)(fs.Length - (long)i));
                }
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("md4prob " + e.StackTrace);
            }
        }

        /// <summary>
        /// Completes the hash computation by performing final operations such
        /// as padding.  At the return of this engineDigest, the MD engine is
        /// reset.
        /// </summary>
        /// <returns>the array of bytes for the resulting hash value.</returns>
        private byte[] engineDigest()
        {
            // pad output to 56 mod 64; as RFC1320 puts it: congruent to 448 mod 512
            int bufferNdx = (int)(count % BLOCK_LENGTH);
            int padLen = (bufferNdx < 56) ? (56 - bufferNdx) : (120 - bufferNdx);

            // padding is always binary 1 followed by binary 0's
            byte[] tail = new byte[padLen + 8];
            tail[0] = (byte)0x80;

            // append length before final transform
            // save number of bits, casting the long to an array of 8 bytes
            // save low-order byte first.
            for (int i = 0; i < 8; i++)
            {
                tail[padLen + i] = (byte)((count * 8) >> (8 * i));
            }

            engineUpdate(tail, 0, tail.Length);

            byte[] result = new byte[16];

            // cast this MD4's context (array of 4 uints) into an array of 16 bytes.
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    result[i * 4 + j] = (byte)(context[i] >> (8 * j));
                }
            }

            // reset the engine
            engineReset();
            return result;
        }

        /// <summary>
        /// Returns a byte hash from a string
        /// </summary>
        /// <param name="s">string to hash</param>
        /// <returns>byte-array that contains the hash</returns>
        public byte[] GetByteHashFromString(string s)
        {
            byte[] b = Encoding.UTF8.GetBytes(s);
            MD4 md4 = new MD4();
            md4.engineUpdate(b, 0, b.Length);
            return md4.engineDigest();
        }

        /// <summary>
        /// Returns a binary hash from an input byte array
        /// </summary>
        /// <param name="b">byte-array to hash</param>
        /// <returns>binary hash of input</returns>
        public byte[] GetByteHashFromBytes(byte[] b)
        {
            MD4 md4 = new MD4();
            md4.engineUpdate(b, 0, b.Length);
            return md4.engineDigest();
        }

        /// <summary>
        /// this is the easy way to get an md4sum from a file
        /// </summary>
        public byte[] GetByteHashCrapFromFileStream(FileStream fs)
        {
            MD4 md4 = new MD4();
            md4.engineUpdate(fs);
            return md4.engineDigest();
        }

        /// <summary>
        /// Returns a string that contains the hexadecimal hash
        /// </summary>
        /// <param name="b">byte-array to input</param>
        /// <returns>String that contains the hex of the hash</returns>
        public string GetHexHashFromBytes(byte[] b)
        {
            byte[] e = GetByteHashFromBytes(b);
            return bytesToHex(e, e.Length);
        }

        /// <summary>
        /// Returns a byte hash from the input byte
        /// </summary>
        /// <param name="b">byte to hash</param>
        /// <returns>binary hash of the input byte</returns>
        public byte[] GetByteHashFromByte(byte b)
        {
            MD4 md4 = new MD4();
            md4.engineUpdate(b);
            return md4.engineDigest();
        }

        /// <summary>
        /// Returns a string that contains the hexadecimal hash
        /// </summary>
        /// <param name="b">byte to hash</param>
        /// <returns>String that contains the hex of the hash</returns>
        public string GetHexHashFromByte(byte b)
        {
            byte[] e = GetByteHashFromByte(b);
            return bytesToHex(e, e.Length);
        }

        /// <summary>
        /// Returns a string that contains the hexadecimal hash
        /// </summary>
        /// <param name="s">string to hash</param>
        /// <returns>String that contains the hex of the hash</returns>
        public string GetHexHashFromString(string s)
        {
            byte[] b = GetByteHashFromString(s);
            return bytesToHex(b, b.Length);
        }

        private static string bytesToHex(byte[] a, int len)
        {
            string temp = BitConverter.ToString(a);

            // We need to remove the dashes that come from the BitConverter
            StringBuilder sb = new StringBuilder((len - 2) / 2);	// This should be the final size

            for (int i = 0; i < temp.Length; i++)
            {
                if (temp[i] != '-')
                {
                    sb.Append(temp[i]);
                }
            }

            return sb.ToString();
        }

        // own methods
        //-----------------------------------------------------------------------------------

        /// <summary>
        /// MD4 basic transformation
        /// </summary>
        /// <remarks>
        /// Transforms context based on 512 bits from input block starting
        /// from the offset'th byte.
        /// </remarks>
        /// <param name="block">input sub-array</param>
        /// <param name="offset">starting position of sub-array</param>
        private void transform(ref byte[] block, int offset)
        {
            // decodes 64 bytes from input block into an array of 16 32-bit
            // entities. Use A as a temp var.
            for (int i = 0; i < 16; i++)
            {
                X[i] = ((uint)block[offset++] & 0xFF) |
                       (((uint)block[offset++] & 0xFF) << 8) |
                       (((uint)block[offset++] & 0xFF) << 16) |
                       (((uint)block[offset++] & 0xFF) << 24);
            }

            uint A = context[0];
            uint B = context[1];
            uint C = context[2];
            uint D = context[3];

            A = FF(A, B, C, D, X[0], 3);
            D = FF(D, A, B, C, X[1], 7);
            C = FF(C, D, A, B, X[2], 11);
            B = FF(B, C, D, A, X[3], 19);
            A = FF(A, B, C, D, X[4], 3);
            D = FF(D, A, B, C, X[5], 7);
            C = FF(C, D, A, B, X[6], 11);
            B = FF(B, C, D, A, X[7], 19);
            A = FF(A, B, C, D, X[8], 3);
            D = FF(D, A, B, C, X[9], 7);
            C = FF(C, D, A, B, X[10], 11);
            B = FF(B, C, D, A, X[11], 19);
            A = FF(A, B, C, D, X[12], 3);
            D = FF(D, A, B, C, X[13], 7);
            C = FF(C, D, A, B, X[14], 11);
            B = FF(B, C, D, A, X[15], 19);

            A = GG(A, B, C, D, X[0], 3);
            D = GG(D, A, B, C, X[4], 5);
            C = GG(C, D, A, B, X[8], 9);
            B = GG(B, C, D, A, X[12], 13);
            A = GG(A, B, C, D, X[1], 3);
            D = GG(D, A, B, C, X[5], 5);
            C = GG(C, D, A, B, X[9], 9);
            B = GG(B, C, D, A, X[13], 13);
            A = GG(A, B, C, D, X[2], 3);
            D = GG(D, A, B, C, X[6], 5);
            C = GG(C, D, A, B, X[10], 9);
            B = GG(B, C, D, A, X[14], 13);
            A = GG(A, B, C, D, X[3], 3);
            D = GG(D, A, B, C, X[7], 5);
            C = GG(C, D, A, B, X[11], 9);
            B = GG(B, C, D, A, X[15], 13);

            A = HH(A, B, C, D, X[0], 3);
            D = HH(D, A, B, C, X[8], 9);
            C = HH(C, D, A, B, X[4], 11);
            B = HH(B, C, D, A, X[12], 15);
            A = HH(A, B, C, D, X[2], 3);
            D = HH(D, A, B, C, X[10], 9);
            C = HH(C, D, A, B, X[6], 11);
            B = HH(B, C, D, A, X[14], 15);
            A = HH(A, B, C, D, X[1], 3);
            D = HH(D, A, B, C, X[9], 9);
            C = HH(C, D, A, B, X[5], 11);
            B = HH(B, C, D, A, X[13], 15);
            A = HH(A, B, C, D, X[3], 3);
            D = HH(D, A, B, C, X[11], 9);
            C = HH(C, D, A, B, X[7], 11);
            B = HH(B, C, D, A, X[15], 15);

            context[0] += A;
            context[1] += B;
            context[2] += C;
            context[3] += D;
        }

        // A basic MD4 atomic function.
        private uint FF(uint a, uint b, uint c, uint d, uint x, int s)
        {
            uint t = a + ((b & c) | (~b & d)) + x;
            return t << s | t >> (32 - s);
        }

        // A basic MD4 atomic function.
        private uint GG(uint a, uint b, uint c, uint d, uint x, int s)
        {
            uint t = a + ((b & (c | d)) | (c & d)) + x + 0x5A827999;
            return t << s | t >> (32 - s);
        }

        // A basic MD4 atomic function.
        private uint HH(uint a, uint b, uint c, uint d, uint x, int s)
        {
            uint t = a + (b ^ c ^ d) + x + 0x6ED9EBA1;
            return t << s | t >> (32 - s);
        }
    }
}
