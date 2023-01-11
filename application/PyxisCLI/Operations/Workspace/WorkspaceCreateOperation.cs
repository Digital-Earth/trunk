using System;
using System.Linq;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceCreateOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace create"; }
        }


        public string Description
        {
            get { return "creates an empty workspace"; }
        }

        public void Run(string[] args)
        {
            if (args.Length <= 2)
            {
                Console.WriteLine("usage: pyx {0} [workspace]",Command);
                return;
            }

            var workspace = args[2].Trim();

            if (Program.Workspaces.WorkspaceExists(workspace))
            {
                throw new Exception(String.Format("Workspace '{0}' already exists.",workspace));
            }

            Program.Workspaces.CreateWorkspace(workspace);
        }
    }
}