using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Operations.Workspace
{
    class WorkspaceDatasetsOperation : IOperationMode
    {
        public string Command
        {
            get { return "workspace datasets"; }
        }


        public string Description
        {
            get { return "search datasets on a workspace"; }
        }

        public void Run(string[] args)
        {

            var query = "";
            var top = 100;
            var json = false;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("q|query", (value) => query = value),
                new ArgsParser.Option("n", (value) => top = int.Parse(value)),
                new ArgsParser.Option("json", (value) => json = true)
            );
           
            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} [-q=query] [-n=top] [-json]", Command);
                return;
            }

            foreach (var arg in args)
            {
                var parts = arg.Split('/');

                var filename = parts[0] + ".ggs.json";

                if (File.Exists(filename))
                {
                    var workspace = Pyxis.Contract.Workspaces.WorkspaceParser.ReadFile(filename);

                    if (parts.Length > 1)
                    {
                        var endpoint = workspace.Endpoints[parts[1]];

                        Search(endpoint,query,top, json);
                    }
                    else
                    {
                        foreach (var endpoint in workspace.Endpoints.Values)
                        {
                            Search(endpoint, query, top, json);
                        }
                    }
                }
            }

        }

        private void Search(Pyxis.Contract.Workspaces.Endpoint endpoint, string query, int top, bool json)
        {
            query = query.ToLower().Trim();

            var data = LocalPersistance.AttachData(endpoint);
            var datasets = data.Get<List<DataSet>>("datasets");

            var results =
                datasets.Where(
                    dataset =>
                        LocalPersistance.Hash(dataset.Uri + dataset.Layer) == query ||
                        dataset.Metadata.Name.ToLower().Contains(query) ||
                        dataset.Metadata.Description.ToLower().Contains(query)).Take(top);

            foreach (var dataset in results)
            {
                if (json)
                {
                    Console.WriteLine(JsonConvert.SerializeObject(dataset, Formatting.Indented));
                }
                else
                {
                    Console.WriteLine("{0} {1} {2}", LocalPersistance.Hash(dataset.Uri + dataset.Layer), dataset.Uri, dataset.Metadata.Name);    
                }
                
            }
        }
    }
}