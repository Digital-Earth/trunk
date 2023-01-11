using System;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class OpenOperation : IOperationMode
    {
        public string Command
        {
            get { return "open"; }
        }

        public string Description
        {
            get { return "open/connect to a url (do small processing if needed)."; }
        }

        public void Run(string[] args)
        {
            bool save = false;
            string srs = null;
            string layer = null;
            string fields = null;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("save", (name) => save = true),
                new ArgsParser.Option("srs", (value) => srs = value),
                new ArgsParser.Option("layer", (value) => layer = value),
                new ArgsParser.Option("fields", (value) => fields = value));

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} url", Command);
                return;
            }

            var start = DateTime.Now;

            foreach (var url in args.Skip(1))
            {
                GeoSource geoSource;
                try
                {
                    geoSource = GeoSourceCreator.CreateFromUrl(Program.Engine, url, srs, layer, fields);
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine("Failed to open url {0} : {1}", url, e.Message);
                    Console.Error.WriteLine(e.StackTrace);
                    geoSource = null;
                }

                if (geoSource == null)
                {
                    continue;
                }

                if (save)
                {
                    var filename = String.Format("{0}.json", geoSource.Id);
                    System.IO.File.WriteAllText(filename, JsonConvert.SerializeObject(geoSource));
                    Console.WriteLine(filename);
                }
                else
                {
                    Console.WriteLine("{0} - ok", url);
                }
            }

            Console.WriteLine("Operation took: {0}[s]", DateTime.Now - start);
        }
    }
}
