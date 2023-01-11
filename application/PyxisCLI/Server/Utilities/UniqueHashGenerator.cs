using System;
using System.Text;
using Newtonsoft.Json;

namespace PyxisCLI.Server.Utilities
{
    /// <summary>
    /// Hepler class to generate a unique and global hash from object. this mean, if two computers will generate them hash from the same object.
    /// We are doing so by convert the object into json format and then make sha256 hash from it.
    /// </summary>
    static class UniqueHashGenerator
    {
        public static string FromObject(object obj)
        {
            var serializedItem = JsonConvert.SerializeObject(obj);
            var bytes = Encoding.UTF8.GetBytes(serializedItem);
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            var checksum = sha256ManagedChecksum.ComputeHash(bytes);
            var key = serializedItem.Length + Convert.ToBase64String(checksum).Replace("=", "").Replace("/", "").Replace("+", "").Replace("-", "");
            return key;
        }
    }
}
