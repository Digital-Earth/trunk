using System;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for UUEncoder
    /// </summary>
    [TestFixture]
    public class UUEncoderTests
    {
        private static Random r = new Random(3010);
        private static byte[] GenerateBytes(int count)
        {
            byte[] buffer = new byte[count];
            r.NextBytes(buffer);
            return buffer;
        }

        [Test]
        public void EncodingDecoding()                
        {
            PerformTest(2114);
            PerformTest(1);
            PerformTest(512);
            PerformTest(16003);
        }

        private static void PerformTest(int length)
        {
            byte[] buffer = GenerateBytes(length);
            string encodedString = UUEncoder.Encode(buffer);
            byte[] decodedBuffer = UUEncoder.Decode(encodedString);
            NUnit.Framework.Assert.AreEqual(buffer.Length, decodedBuffer.Length);
            for (int i = 0; i < buffer.Length; ++i)
            {
                NUnit.Framework.Assert.AreEqual(buffer[i], decodedBuffer[i]);
            }
        }
    }
}