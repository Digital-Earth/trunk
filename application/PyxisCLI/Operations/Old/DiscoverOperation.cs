using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.IO.MultiDomain;
using Pyxis.Utilities;

namespace PyxisCLI.Operations.Old
{
    class DiscoverOperation : IOperationMode
    {
        class Options
        {
            public bool Json { get; set; }
            public bool Merge { get; set; }
            public bool Save { get; set; }
            public int Limit { get; set; }
            public bool Recursive { get; set; }
        }


        public string Command
        {
            get { return "old discover"; }
        }

        public string Description
        {
            get { return "discover data from a given url"; }
        }

        public void Run(string[] args)
        {
            var options = new Options()
            {
                Json = false,
                Merge = false,
                Save = false,
                Recursive = false,
                Limit = 0
            };

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("json", (value) => options.Json = true),
                new ArgsParser.Option("merge", (value) => options.Merge = true),
                new ArgsParser.Option("save", (value) => options.Save = true),
                new ArgsParser.Option("limit", (value) => options.Limit = int.Parse(value)),
                new ArgsParser.Option("r|recursive", (value) => options.Recursive = true));

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-json] [-merge] [-save] [-limit=100] url", Command);
                return;
            }

            foreach (var url in args.Skip(1))
            {
                DiscoverUrl(url, Program.Engine, options);
            }
        }

        private static void DiscoverUrl(string url, Engine engine, Options options)
        {
            var jsonResults = new List<DataSet>();

            var urlStack = new Stack<string>();
            while (url.HasContent())
            {
                Console.WriteLine("--> " + url);
                var catalog = engine.BuildCatalog(url);

                if (catalog == null)
                {
                    Console.Error.WriteLine("Failed to discover data from url {0}", url);
                }
                else
                {
                    if (options.Recursive && catalog.SubCatalogs != null)
                    {
                        foreach (var subCatalog in catalog.SubCatalogs)
                        {
                            urlStack.Push(subCatalog.Uri);
                        }
                    }

                    if (options.Json)
                    {
                        if (options.Recursive)
                        {
                            if (catalog.DataSets != null)
                            {
                                jsonResults.AddRange(catalog.DataSets);
                            }
                        }
                        else
                        {
                            Console.WriteLine(JsonConvert.SerializeObject(catalog, Formatting.Indented));
                        }
                    }
                    else if (options.Merge)
                    {
                        if (TryMergeDataset(engine, catalog, options.Save))
                        {
                            return;
                        }
                    }
                    else
                    {
                        var dataSetCount = catalog.DataSets != null ? catalog.DataSets.Count : 0;
                        var subCatalogCount = catalog.SubCatalogs != null ? catalog.SubCatalogs.Count : 0;

                        Console.WriteLine("subCatalogs {0}, dataSets {1}", subCatalogCount, dataSetCount);

                        if (catalog.SubCatalogs != null)
                        {
                            var subCatalogs = options.Limit > 0
                                ? catalog.SubCatalogs.Take(options.Limit)
                                : catalog.SubCatalogs;

                            foreach (var subCatalog in subCatalogs)
                            {
                                Console.WriteLine(subCatalog.Uri);
                            }

                            if (subCatalogCount > options.Limit && options.Limit > 0)
                            {
                                Console.WriteLine(" ( showing only first {1} from {0} sub catalogs ) ",
                                    catalog.SubCatalogs.Count, options.Limit);
                            }
                        }

                        if (catalog.DataSets != null)
                        {
                            var datasets = options.Limit > 0 ? catalog.DataSets.Take(options.Limit) : catalog.DataSets;

                            foreach (var dataset in datasets)
                            {
                                if (dataset.Layer.HasContent() && !dataset.Uri.Contains(dataset.Layer))
                                {
                                    if (dataset.Fields != null & dataset.Fields.Count == 1)
                                    {
                                        Console.WriteLine("{0} -layer={1} -fields={2}", dataset.Uri, dataset.Layer,
                                            string.Join(",", dataset.Fields));
                                    }
                                    else
                                    {
                                        Console.WriteLine("{0} -layer={1}", dataset.Uri, dataset.Layer);
                                    }
                                }
                                else
                                {
                                    Console.WriteLine(dataset.Uri);
                                }
                            }
                            if (dataSetCount > options.Limit && options.Limit > 0)
                            {
                                Console.WriteLine(" ( showing only first {1} from {0} datasets ) ",
                                    catalog.DataSets.Count, options.Limit);
                            }
                        }
                    }
                }

                url = catalog.NextPage != null ? catalog.NextPage.Uri : null;

                if (!url.HasContent() && urlStack.Count > 0)
                {
                    url = urlStack.Pop();
                }
            }

            if (options.Recursive && options.Json)
            {
                Console.WriteLine(JsonConvert.SerializeObject(jsonResults, Formatting.Indented));
            }

        }

        private static bool TryMergeDataset(Engine engine, DataSetCatalog catalog, bool save)
        {
            var domains = new Dictionary<string, Domain>();

            foreach (var dataset in catalog.DataSets)
            {
                if (dataset.Domains == null)
                {
                    Console.WriteLine("dataset " + dataset.Uri + " has no domains information.");
                    return false;
                }

                foreach (var domain in dataset.Domains)
                {
                    if (!domains.ContainsKey(domain.Name))
                    {
                        domains[domain.Name] = new Domain()
                        {
                            Name = domain.Name,
                            Type = domain.Type,
                            Unit = domain.Unit,
                            Metadata = domain.Metadata,
                            Values = new List<object>()
                        };

                        if (domain.Value != null)
                        {
                            domains[domain.Name].Value = domain.Value;
                        }
                    }

                    MergeDomainValues(domains[domain.Name], domain);
                }
            }

            var validDomains = new List<Domain>();

            foreach (var domain in domains.Values)
            {
                if (!domain.Name.HasContent() || domain.Values == null || domain.Values.Count <= 1)
                {
                    continue;
                }

                if (domain.Type == PipelineSpecification.FieldType.Date)
                {
                    domain.Values.Sort();
                }

                validDomains.Add(domain);
            }

            var staticResolver = new StaticMultiDomianGeoSourceResolver()
            {
                Domains = validDomains
            };


            if (save)
            {
                GeoSource geoSource = null;

                foreach (var dataSet in catalog.DataSets)
                {
                    var domainValues = validDomains.ToDictionary(x => x.Name, x => dataSet.Domains.Find(d => d.Name == x.Name).Value);

                    try
                    {
                        geoSource = GeoSourceCreator.CreateFromDataSet(engine, dataSet);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("failed to intialize {0}, trying once more...", dataSet.Uri);
                        geoSource = engine.BeginImport(dataSet).Task.Result;
                        throw;
                    }

                    var filename = string.Format("{0}.json", geoSource.Id);
                    System.IO.File.WriteAllText(filename, JsonConvert.SerializeObject(geoSource));

                    Console.WriteLine("{0} - {1}", filename, JsonConvert.SerializeObject(domainValues, Formatting.None));

                    staticResolver.Register(domainValues, ResourceReference.FromResource(geoSource));
                }

                var finalfilename = string.Format("resolver.{0}.json", Guid.NewGuid());
                System.IO.File.WriteAllText(finalfilename, JsonConvert.SerializeObject(staticResolver));

                Console.WriteLine(finalfilename + " - static resolver file");
            }
            else
            {
                Console.WriteLine(JsonConvert.SerializeObject(validDomains, Formatting.Indented));
            }
            return true;
        }

        private static void MergeDomainValues(Domain target, Domain source)
        {
            if (source.Values != null)
            {
                foreach (var value in source.Values)
                {
                    if (!target.Values.Contains(value))
                    {
                        target.Values.Add(value);
                    }
                }
            }

            if (source.Value != null)
            {
                if (!target.Values.Contains(source.Value))
                {
                    target.Values.Add(source.Value);
                }
            }

        }
    }
}