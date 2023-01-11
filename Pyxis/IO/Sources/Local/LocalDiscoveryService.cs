using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataDiscovery;

namespace Pyxis.IO.Sources.Local
{
    class LocalDiscoveryService : IDiscoveryService
    {
        private class PathAndLayer
        {
            public string Path { get; set; }
            public string Layer { get; set; }
        }

        public string ServiceIdentifier
        {
            get { return "Local"; }
        }

        private PathAndLayer ExtractPathAndLayer(string uri)
        {
            if (uri.ToLower().StartsWith("file://"))
            {
                Uri result;
                if (Uri.TryCreate(uri, UriKind.Absolute, out result))
                {
                    return new PathAndLayer()
                    {
                        Path = result.AbsolutePath,
                        Layer = result.Query
                    };
                }
            } 
            else if (uri.Contains('?'))
            {
                var parts = uri.Split('?');

                return new PathAndLayer()
                {
                    Path = parts[0],
                    Layer = parts[1]
                };
            }
            return new PathAndLayer {Path = uri};
        }

        public bool IsUriSupported(string uri)
        {
            var pathAndLayer = ExtractPathAndLayer(uri);

            return PipelineConstructionUtility.IsDataSourceSupported(pathAndLayer.Path, IPipeBuilder.eCheckOptions.knPartial) || Directory.Exists(uri);
        }

        public Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            var pathAndLayer = ExtractPathAndLayer(context.Request.Uri);
            var result = new DiscoveryResult(context.Request);

            var catalog = DataDiscoveryExtensions.FromPyxis(PipelineConstructionUtility.BuildCatalog(pathAndLayer.Path));

            if (catalog != null)
            {
                //Register datasets founds. Or Add leads if we found multiple layers on that uri
                if (catalog.DataSets != null)
                {
                    if (catalog.DataSets.Count == 1)
                    {
                        result.ServiceIdentifier = ServiceIdentifier;
                        result.DataSet = catalog.DataSets[0];
                        result.Metadata = result.DataSet.Metadata;                        
                    }
                    else if (pathAndLayer.Layer.HasContent())
                    {
                        var foundDataset =
                            catalog.DataSets.FirstOrDefault(dataset => dataset.Layer == pathAndLayer.Layer);
                        if (foundDataset != null)
                        {
                            result.ServiceIdentifier = ServiceIdentifier;
                            result.DataSet = foundDataset;
                            result.Metadata = result.DataSet.Metadata;
                        }
                    }
                    else
                    {
                        foreach (var dataSet in catalog.DataSets)
                        {
                            result.AddLead(new DiscoveryRequest()
                            {
                                Uri = dataSet.Uri + "?" + dataSet.Layer,
                                Permit = context.Request.Permit,
                                ServiceIdentifier = ServiceIdentifier
                            });
                        }
                    }
                }

                //register additional leads (sub catalogs have their own uri to discover)
                if (catalog.SubCatalogs != null)
                {
                    foreach (var subCatalog in catalog.SubCatalogs)
                    {
                        result.AddLead(new DiscoveryRequest()
                        {
                            Uri = subCatalog.Uri,
                            Permit = context.Request.Permit,
                            ServiceIdentifier = ServiceIdentifier
                        });
                    }
                }
            }
            else if (Directory.Exists(pathAndLayer.Path))
            {                
                var entries = Directory.EnumerateFileSystemEntries(pathAndLayer.Path);
                foreach (var entry in entries)
                {
                    result.AddLead(new DiscoveryRequest()
                    {
                        Uri = entry,
                        Permit = context.Request.Permit,
                        ServiceIdentifier = ServiceIdentifier
                    });
                }
            }

            return Task.FromResult(result);
        }        

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new LocalDataSetImportService(dataSet, permit);
        }
    }
}
