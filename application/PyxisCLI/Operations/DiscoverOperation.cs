using System;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Workspaces;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations
{
    class DiscoverOperation : IOperationMode
    {
        public string Command
        {
            get { return "discover"; }
        }


        public string Description
        {
            get { return "discover endpoint"; }
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

                    if (reference.Name.HasContent())
                    {
                        var endpoint = workspace.Endpoints[reference.Name];
                        Discover(endpoint);
                    }
                    else
                    {
                        foreach (var endpoint in workspace.Endpoints.Values)
                        {
                            Discover(endpoint);
                        }
                    }
                }
            }
        }

        private void Discover(Pyxis.Contract.Workspaces.Endpoint endpoint)
        {
            var data = LocalPersistance.AttachData(endpoint);
            var endPointStatus = data.Get<UrlDiscoveryReport>("status");

            var expectedCount = 0;
            if (endPointStatus != null)
            {
                expectedCount = endPointStatus.DataSetCount;
            }

            var discoverySummary = LocalGazetteer.DiscoverRecursively(Program.Engine, endpoint.Uri, expectedCount);

            endPointStatus = new UrlDiscoveryReport()
            {
                Uri = endpoint.Uri,
                Host = new Uri(endpoint.Uri).Host,
                Status = "discovered",
                DataSetCount = discoverySummary.DataSets.Count,
                VerifiedDataSetCount = 0,
                BrokenDataSetCount = 0,
                UnknownDataSetCount = 0,
                LastDiscovered = DateTime.UtcNow
            };

            data.Set("status",endPointStatus);
            data.Set("datasets",discoverySummary.DataSets);

            Console.WriteLine("  Status: {0}", endPointStatus.Status);
            Console.WriteLine("  Datasets: {0}", endPointStatus.DataSetCount);
        }
    }
}