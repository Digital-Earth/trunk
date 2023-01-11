using PyxisCLI.State;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;

namespace PyxisCLI
{
    public class PyxisCliConfig
    {
        public string ElasticSearch = "";
        public string Mongo = "";

        private static PyxisCliConfig s_instance;

        public static PyxisCliConfig Config
        {
            get 
            {
                return s_instance ?? (s_instance = LocalDirPersistance.Load<PyxisCliConfig>("config") ?? new PyxisCliConfig());
            }
        }

        public static void Load(string path)
        {
            s_instance = JsonConvert.DeserializeObject<PyxisCliConfig>(System.IO.File.ReadAllText(path));
        }

        public void Save()
        {
            LocalDirPersistance.Save("config", this);
        }
    }
}
