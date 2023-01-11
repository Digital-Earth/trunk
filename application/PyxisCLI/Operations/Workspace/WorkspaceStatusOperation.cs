using System;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceStatusOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace status"; }
        }


        public string Description
        {
            get { return "show a status of a workspace"; }
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
                var filename = arg + ".ggs.json";

                if (File.Exists(filename))
                {
                    var workspace = Pyxis.Contract.Workspaces.WorkspaceParser.ReadFile(filename);

                    foreach (var endpoint in workspace.Endpoints)
                    {
                        var uri = endpoint.Value.Uri;
                        Console.WriteLine("{0} [Endpoint] -> {1}", endpoint.Key, uri );

                        var endPointStatus = LocalPersistance.AttachData(endpoint.Value).Get<UrlDiscoveryReport>("status");

                        if (endPointStatus == null)
                        {
                            endPointStatus = new UrlDiscoveryReport()
                            {
                                Uri = uri,
                                Host = new Uri(uri).Host,
                                Status = "new"
                            };
                        }
                                               
                        Console.WriteLine("  Status: {0} {1}", endPointStatus.Status, endPointStatus.LastDiscovered != DateTime.MinValue ? endPointStatus.LastDiscovered.ToString() : "");
                        Console.WriteLine("  Datasets: {0}", endPointStatus.DataSetCount);
                        Console.WriteLine("  Verified: {0}", endPointStatus.VerifiedDataSetCount);
                    }

                    foreach (var nameAndImport in workspace.Imports)
                    {
                        var import = nameAndImport.Value;
                        Console.WriteLine("{0} [{1}] -> {2}", nameAndImport.Key, import.Type, import.Uri);

                        if (import.Type == "DataSet")
                        {
                            var geoSource = LocalPersistance.AttachData(import).Get<GeoSource>("geosource");

                            if (geoSource != null)
                            {
                                Console.WriteLine("  Id: {0}", geoSource.Id);
                                Console.WriteLine("  Imported: {0}", geoSource.Metadata.Created);
                                Console.WriteLine("  DataSize: {0}", geoSource.DataSize);
                            }    
                        }
                    }

                    foreach (var nameAndGlobe in workspace.Globes)
                    {
                        Console.WriteLine("{0} [Globe] -> {1} layers", nameAndGlobe.Key, nameAndGlobe.Value.Layers.Count);
                    }
                }
            }

        }
    }
}