using Newtonsoft.Json;
using System;
using System.IO;

namespace Pyxis.Storage
{
    public class BlobKeyFactory
    {
        public static string GenerateKey(Stream stream)
        {
            byte[] checksum = null;
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            checksum = sha256ManagedChecksum.ComputeHash(stream);

            var blobKey = new BlobKey(Convert.ToBase64String(checksum), stream.Length);
            return blobKey.ToString();
        }

        public static string GenerateKey(byte[] buffer, int offset, int length)
        {
            using (var stream = new MemoryStream(buffer, offset, length))
            {
                return GenerateKey(stream);
            }
        }

        public static string GenerateKey(object obj)
        {
            using (var stream = new MemoryStream())
            {
                var streamWriter = new StreamWriter(stream);
                streamWriter.Write(JsonConvert.SerializeObject(obj));
                streamWriter.Flush();
                stream.Position = 0;
                return GenerateKey(stream);
            }
        }

        private class BlobKey
        {
            public long Size;
            public string Hash;

            public BlobKey(string hash, long length)
            {
                Size = length;
                Hash = hash;
            }

            public override string ToString()
            {
                return JsonConvert.SerializeObject(this);
            }
        }
    }
}