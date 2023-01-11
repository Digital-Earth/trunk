using System;
using System.IO;
using System.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceAddOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace add"; }
        }


        public string Description
        {
            get { return "adds an item to a workspace"; }
        }

        public void Run(string[] args)
        {
            bool inline = false;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("inline", v => inline = true));

            if (args.Length <= 5)
            {
                Console.WriteLine("usage: pyx {0} [workspace] [endpoint|import|globe] name [file|value] [-inline]",
                    Command);
                return;
            }

            var workspaceFile = args[2] + ".ggs.json";
            var type = args[3];
            var name = args[4];
            var value = args[5];

            if (!name.All(c => Char.IsLetterOrDigit(c) || c == '_'))
            {
                throw new Exception("name can only only alpha numeric, digit or _");
            }        

            var workspace = WorkspaceParser.ReadFile(workspaceFile);

            var directory = Path.GetDirectoryName(Path.GetFullPath(workspaceFile));

            switch (type)
            {
                case "endpoint":
                {
                    Uri uri;
                    if (Uri.TryCreate(value, UriKind.Absolute, out uri))
                    {
                        WorkspaceParser.UpdateOrInsertEndpoint(workspaceFile, name,
                            new Endpoint {Uri = value});
                    }
                    else
                    {
                        throw new Exception("value is not a valid uri");
                    }
                    break;
                }

                case "import":
                {
                    var import =
                        WorkspaceParser.ParseImport(JObject.Parse(File.ReadAllText(Path.Combine(directory, value))));
                    if (import != null)
                    {
                        WorkspaceParser.UpdateOrInsertImport(workspaceFile, name, import, inline ? null : value);
                    }
                    break;
                }

                case "globe":
                {
                    var globe =
                        JsonConvert.DeserializeObject<GlobeTemplate>(File.ReadAllText(Path.Combine(directory, value)));
                    if (globe != null)
                    {
                        WorkspaceParser.UpdateOrInsertGlobe(workspaceFile, name, globe, inline ? null : value);
                    }
                    break;
                }
            }
        }
    }
}