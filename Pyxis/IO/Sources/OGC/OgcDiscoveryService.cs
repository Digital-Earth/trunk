using System.Linq;
using System.Threading.Tasks;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;

namespace Pyxis.IO.Sources.OGC
{
    public class OgcDiscoveryService<T> : IDiscoveryService where T : OgcWebServiceBase, new()
    {
        private readonly T m_service = new T();

        public string ServiceIdentifier
        {
            get { return m_service.ServiceIdentifier; }
        }

        public bool IsUriSupported(string uri)
        {
            return m_service.IsUriSupported(uri);
        }

        public Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            return Task<DiscoveryResult>.Factory.StartNew(() =>
            {
                var uri = context.Request.Uri;

                var catalog = m_service.BuildCatalog(uri, context.Request.Permit);

                if (catalog == null)
                {
                    return null;
                }

                var result = new DiscoveryResult()
                {
                    Uri = uri.ToString(),
                    Metadata = catalog.Metadata,
                    ServiceIdentifier = ServiceIdentifier
                };

                if (catalog.DataSets == null || catalog.DataSets.Count <= 0)
                {
                    return result;
                }

                result.DataSet = catalog.DataSets[0];

                if (catalog.DataSets.Count > 1)
                {
                    result.AdditionalDataSets = catalog.DataSets.Skip(1).ToList();
                }

                return result;
            });
        }

        private class ImportService : IDataSetImportService
        {
            private T Service { get; set; }
            private DataSet DataSet { get; set; }
            private IPermit Permit { get; set; }


            public ImportService(T service, DataSet dataSet, IPermit permit)
            {
                Service = service;
                DataSet = dataSet;
                Permit = permit;
            }

            public IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
            {
                return Service.BuildPipeline(DataSet, Permit);
            }

            public void EnrichGeoSource(Engine engine, GeoSource geoSource)
            {
                //make sure we have a good style setup for icons.
                if (geoSource.Specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
                {
                    geoSource.Style = engine.CreateDefaultStyle(geoSource);
                }
            }
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new ImportService(m_service, dataSet, permit);
        }
    }
}