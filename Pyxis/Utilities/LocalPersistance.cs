using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Newtonsoft.Json;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Git insparied local storage of object.
    /// 
    /// the LocalPersistance store objects based using sha1 of their serialized json representation.
    /// 
    /// SaveObject(object) will return an hash for that object.
    /// 
    /// However, this also allow you to attach additonal data for a given object. for example: 
    /// var person = new Person("123","idan");
    /// var data = LocalPersistance.AttachData(person); //allow you to attach data to the object's hash
    /// data.Set("picture","http://gavatar.com/shatzi"); //attached data is accessed by name
    /// </summary>
    public static class LocalPersistance
    {
        public static string RootDirectory = Directory.GetCurrentDirectory();
        public static string StorageDirectory = ".ggs";

        public static string SaveObject(object o)
        {
            var content = JsonConvert.SerializeObject(o);
            var hash = Hash(content);
            var file = GetPath(hash);
            if (!File.Exists(file))
            {
                var directory = Path.GetDirectoryName(file);
                if (!Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }
                File.WriteAllText(file, content);    
            }
            return hash;
        }

        public static string LoadObject(string hash)
        {
            return File.ReadAllText(GetPath(hash));
        }

        public static T LoadObject<T>(string hash)
        {
            return JsonConvert.DeserializeObject<T>(LoadObject(hash));
        }

        public static PersitantData AttachData(object o)
        {
            return new PersitantData(GetDataDirectory(o));
        }

        public static string GetDataDirectory(object o)
        {
            SaveObject(o);
            var directory = GetPath(HashObject(o), "data");
            if (!Directory.Exists(directory)) 
            {
                Directory.CreateDirectory(directory);
            }
            return directory;
        }

        public static string GetPath(string hash, string section = "objects")
        {
            return Path.Combine(
                RootDirectory, StorageDirectory, section, 
                hash.Substring(0, 2), hash.Substring(2));
        }

        public static string HashObject(object o)
        {
            return Hash(JsonConvert.SerializeObject(o));
        }

        public static string Hash(string serailzedObject)
        {
            var bytes = Encoding.UTF8.GetBytes(serailzedObject);
            using (var sha1ManagedChecksum = new System.Security.Cryptography.SHA1Managed())
            {
                var checksum = sha1ManagedChecksum.ComputeHash(bytes);
                var key = String.Join("", checksum.Select(x => x.ToString("x2")));
                return key;    
            }
        }
    }

    public class PersitantData
    {
        public string Root { get; private set; }
        
        public PersitantData(string root)
        {
            Root = root;
        }

        public void Set(string name, object data)
        {
            var filename = Path.Combine(Root, name );
            File.WriteAllText(filename, JsonConvert.SerializeObject(data));
        }

        public T Get<T>(string name)
        {
            var filename = Path.Combine(Root, name);
            if (File.Exists(filename))
            {
                return JsonConvert.DeserializeObject<T>(File.ReadAllText(filename));
            }
            return default(T);
        }

        public List<string> List()
        {
            return Directory.EnumerateFiles(Root).ToList();
        }

        public void Delete(string name)
        {
            File.Delete(Path.Combine(Root, name));
        }
    }
}
