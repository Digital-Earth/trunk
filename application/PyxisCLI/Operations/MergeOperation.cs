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

namespace PyxisCLI.Operations
{
    class MergeOperation : IOperationMode
    {
        class Options
        {
            public bool Preview { get; set; }
            public string Path { get; set; }
        }


        public string Command
        {
            get { return "merge"; }
        }

        public string Description
        {
            get { return "merge a catalog into domain based geo-souce"; }
        }

        public void Run(string[] args)
        {
            var options = new Options()
            {
                Preview = false,
                Path = "",
            };

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("d|dir", (value) => options.Path = value),
                new ArgsParser.Option("p|preview", (name) => options.Preview = true));


            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-dir=output] [-preview] url", Command);
                return;
            }

            foreach (var url in args.Skip(1))
            {
                DiscoverUrl(url, Program.Engine, options);
            }
        }

        private static void DiscoverUrl(string url, Engine engine, Options options)
        {
            var catalog = url.ToLower().EndsWith(".csv") ? 
                BuildCatalogFromCsv(url) : 
                JsonConvert.DeserializeObject<DataSetCatalog>(System.IO.File.ReadAllText(url));

            if (catalog == null)
            {
                Console.Error.WriteLine("Failed to load catalog from url {0}", url);
                return;
            }

            if (options.Path.HasContent())
            {
                if (!System.IO.Directory.Exists(options.Path))
                {
                    System.IO.Directory.CreateDirectory(options.Path);
                }
            }

            TryMergeDataset(engine, catalog, !options.Preview, options.Path);    
        }

        private static DataSetCatalog BuildCatalogFromCsv(string url)
        {
            var reader = new CsvHelper.CsvReader(System.IO.File.OpenText(url));

            var catalog = new DataSetCatalog()
            {
                Uri = url,
                DataSets = new List<DataSet>()
            };

            var knownFields = new[] {"uri", "layer", "fields"};

            while (reader.Read())
            {
                var uri = reader.GetField<string>("uri");
                var layer = reader.GetField<string>("layer");
                var fields = reader.GetField<string>("fields");

                var dataset = new DataSet()
                {
                    Uri = uri,
                    Layer = layer,
                    Metadata = new SimpleMetadata()
                    {
                        Name = uri,
                        Description = layer
                    },
                    Fields = fields.Split(',').ToList(),
                };

                var domains = reader.FieldHeaders.Where(x => !knownFields.Contains(x)).ToList();

                if (domains.Count > 0)
                {
                    dataset.Domains = new List<Domain>();
                    foreach (var domainName in domains)
                    {
                        dataset.Domains.Add(new Domain()
                        {
                            Name = domainName,
                            Metadata = new SimpleMetadata()
                            {
                                Name = domainName,
                            },
                            Type = GetDomainTypeFromName(domainName),
                            Value = reader.GetField(domainName)
                        });
                    }
                }

                catalog.DataSets.Add(dataset);
            }

            return catalog;
        }

        private static PipelineSpecification.FieldType GetDomainTypeFromName(string domainName)
        {
            switch (domainName)
            {
                case "time":
                case "date":
                case "timestamp":
                    return PipelineSpecification.FieldType.Date;
                    break;

                default:
                    return PipelineSpecification.FieldType.String;
            }
        }

        private static bool TryMergeDataset(Engine engine, DataSetCatalog catalog, bool save, string path)
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
                MultiDomainGeoSource mainGeoSource = null;

                foreach (var dataSet in catalog.DataSets)
                {
                    var domainValues = validDomains.ToDictionary(x => x.Name, x => dataSet.Domains.Find(d=>d.Name==x.Name).Value);

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

                    if (mainGeoSource == null)
                    {
                        mainGeoSource = JsonConvert.DeserializeObject<MultiDomainGeoSource>(JsonConvert.SerializeObject(geoSource));
                        mainGeoSource.Id = Guid.NewGuid();
                        mainGeoSource.Version = Guid.NewGuid();
                        mainGeoSource.Domains = validDomains;
                    }
                    
                    var filename = System.IO.Path.Combine(path,string.Format("{0}.json", geoSource.Id));
                    System.IO.File.WriteAllText(filename, JsonConvert.SerializeObject(geoSource));

                    Console.WriteLine("{0} - {1}", filename, JsonConvert.SerializeObject(domainValues, Formatting.None));

                    staticResolver.Register(domainValues, ResourceReference.FromResource(geoSource));
                }

                var finalFilename = System.IO.Path.Combine(path,string.Format("{0}.json", mainGeoSource.Id));
                var finalResolverFilename = System.IO.Path.Combine(path,string.Format("{0}.resolver.json", mainGeoSource.Id));
                System.IO.File.WriteAllText(finalFilename, JsonConvert.SerializeObject(mainGeoSource));
                System.IO.File.WriteAllText(finalResolverFilename, JsonConvert.SerializeObject(staticResolver));

                Console.WriteLine("multi-domain-geosource:");
                Console.WriteLine(finalFilename);
                Console.WriteLine(finalResolverFilename);
            }
            else
            {
                Console.WriteLine(JsonConvert.SerializeObject(validDomains,Formatting.Indented));
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