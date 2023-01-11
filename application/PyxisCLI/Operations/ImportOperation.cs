using System;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class ImportOperation : IOperationMode
    {
        public string Command
        {
            get { return "import"; }
        }

        public string Description
        {
            get { return "import (or referesh) a dataset"; }
        }

        public void Run(string[] args)
        {
           
            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0}", Command);
                return;
            }

            foreach (var arg in args)
            {
                var reference = new Reference(arg);
                
                if (Program.Workspaces.WorkspaceExists(reference.Workspace))
                {
                    var workspace = Program.Workspaces.GetWorkspace(reference.Workspace);

                    if (!reference.Name.HasContent())
                    {
                        foreach (var import in workspace.Imports)
                        {
                            DisplayResult(arg, Program.Workspaces.ResolveGeoSource(import.Value, reference.Domains, true));
                        }
                    }
                    else
                    {
                        DisplayResult(arg, Program.Workspaces.ResolveGeoSource(reference, true));
                    }
                }
            }

        }

        private static void DisplayResult(string name, GeoSource geoSource)
        {
            if (geoSource != null)
            {
                Console.WriteLine(name + " -> Success (" + geoSource.Id + ")");
            }
            else
            {
                Console.WriteLine(name + " -> Failed");
            }
        }
    }
}