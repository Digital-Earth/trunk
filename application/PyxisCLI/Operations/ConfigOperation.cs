using System;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class ConfigOperation : IOperationMode
    {
        public string Command
        {
            get { return "config"; }
        }

        public string Description
        {
            get { return "config pyxis CLI"; }
        }

        public void Run(string[] args)
        {
            var es = "";
            var mongo = "";

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("es|elasticsearch", (value) => es = value),
                new ArgsParser.Option("mongo", (value) => mongo = value));

            if (es.HasContent() || mongo.HasContent())
            {
                if (es.HasContent())
                {
                    PyxisCliConfig.Config.ElasticSearch = es;
                }
                if (mongo.HasContent())
                {
                    PyxisCliConfig.Config.Mongo = mongo;
                }
                PyxisCliConfig.Config.Save();
            }
            else
            {
                Console.WriteLine("usage: pyx {0} [-es=path] [-mongo=path]", Command);
            }

            Console.WriteLine(JsonConvert.SerializeObject(PyxisCliConfig.Config,Formatting.Indented));
        }
    }
}