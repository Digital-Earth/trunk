using System;
using System.Linq;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceRemoveOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace remove"; }
        }


        public string Description
        {
            get { return "removes an workspace or an item from a workspace"; }
        }

        public void Run(string[] args)
        {
            if (args.Length <= 2)
            {
                Console.WriteLine("usage: pyx {0} [workspace] [endpoint|import|globe] [name]", Command);
                return;
            }

            var workspace = args[2];
            
            if (args.Length >= 4)
            {
                var type = args[3];
                var name = args[4];

                if (!name.All(c => Char.IsLetterOrDigit(c) || c == '_'))
                {
                    throw new Exception("name can only only alpha numeric, digit or _");
                }

                var file = Program.Workspaces.GetWorkspaceFile(workspace);

                switch (type)
                {
                    case "endpoint":
                        file.RemoveEndpoint(name);
                        break;
                    case "import":
                        file.RemoveImport(name);
                        break;
                    case "globe":
                        file.RemoveGlobe(name);
                        break;
                }
            }
            else
            {
                Program.Workspaces.DeleteWorkspace(args[2]);   
            }
            
        }
    }
}