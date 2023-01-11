using System;
using System.Linq;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceListOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace list"; }
        }


        public string Description
        {
            get { return "list all items in a workspace"; }
        }

        public void Run(string[] args)
        {
            if (args.Length <= 2)
            {
                Program.Workspaces.LoadFolder();

                foreach (var keyValue in Program.Workspaces.Workspaces)
                {
                    var workspace = keyValue.Value;
                    if (workspace.Owner != null)
                    {
                        Console.WriteLine("{0} [Workspace] [owner:{1}]", keyValue.Key, workspace.Owner.Name);
                    }
                    else
                    {
                        Console.WriteLine("{0} [Workspace] [public]", keyValue.Key);
                    }

                    ListWorkspaceItems(keyValue.Key, workspace);
                }
                return;
            }
            else
            {
                foreach (var arg in args.Skip(2))
                {
                    var filename = arg + ".ggs.json";

                    if (File.Exists(filename))
                    {
                        var workspace = Pyxis.Contract.Workspaces.WorkspaceParser.ReadFile(filename);

                        ListWorkspaceItems(arg, workspace);
                    }
                }    
            }
        }

        private static void ListWorkspaceItems(string name, Pyxis.Contract.Workspaces.Workspace workspace)
        {
            foreach (var endpoint in workspace.Endpoints)
            {
                Console.WriteLine("{0}/{1} [Endpoint]", name, endpoint.Key);
            }

            foreach (var nameAndImport in workspace.Imports)
            {
                Console.WriteLine("{0}/{1} [Import]", name, nameAndImport.Key);
            }

            foreach (var nameAndGlobe in workspace.Globes)
            {
                Console.WriteLine("{0}/{1} [Globe]", name, nameAndGlobe.Key);
            }
        }
    }
}