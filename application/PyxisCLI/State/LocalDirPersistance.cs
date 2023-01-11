using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace PyxisCLI.State
{
    static class LocalDirPersistance
    {
        public static string Root = Directory.GetCurrentDirectory();

        public static string GetFileName(string name)
        {
            return Path.Combine(Root, ".pyx", name);
        }

        public static T Load<T>(string name)
        {
            var filename = GetFileName(name + ".json");
            if (File.Exists(filename))
            {
                return JsonConvert.DeserializeObject<T>(File.ReadAllText(filename));
            }
            return default(T);
        }

        public static void Save<T>(string name, T value)
        {
            var filename = GetFileName(name + ".json");
            Directory.CreateDirectory(Path.GetDirectoryName(filename));
            File.WriteAllText(filename, JsonConvert.SerializeObject(value));
        }

        public static string SubDirectory(string name)
        {
            var dir = GetFileName(name);
            Directory.CreateDirectory(dir);
            return dir;
        }

        public static string CleanSubDirectory(string name)
        {
            var dir = GetFileName(name);
            if (Directory.Exists(dir))
            {
                Directory.Delete(dir,true);
            }
            Directory.CreateDirectory(dir);
            return dir;
        }
    }
}
