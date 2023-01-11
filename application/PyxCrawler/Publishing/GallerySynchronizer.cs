using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using PyxCrawler.Import;
using PyxCrawler.Models;
using PyxCrawler.Properties;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing;
using User = Pyxis.Publishing.User;

namespace PyxCrawler.Publishing
{
    public static class GallerySynchronizer
    {
        private static readonly Channel s_channel;
        private static readonly User s_user;
        private static readonly List<string> s_protocolPreference = new List<string>
        {
            "WFS",
            "AGSF",
            "WCS",
            "WMS",
            "AGSM"
        };

        static GallerySynchronizer()
        {
            var userName = Settings.Default.CrawlerUser;
            var data = Convert.FromBase64String(Settings.Default.CrawlerKey);
            var apiKey = ASCIIEncoding.ASCII.GetString(data);
            var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI);
            s_channel = channel.Authenticate(new ApiKey(apiKey, userName));
            s_user = s_channel.AsUser();
        }

        /// <summary>
        /// Insert or update, if necessary, an endpoint as a Gallery in the PYXIS Channel.
        /// When inserted, the endpoint's Provider is updated to reflect the created Gallery's Id.
        /// The endpoint's data sets are similarly inserted or updated, if necessary.
        /// When inserted, a data set's Reference property is set to reflect the created GeoSource.
        /// </summary>
        /// <param name="endpoint">The endpoint to base the Gallery upon.</param>
        /// <returns>The Gallery Provider associated with the endpoint.</returns>
        public static Provider UpsertGallery(OnlineGeospatialEndpoint endpoint)
        {
            // Guard against duplicates in the Gallery until links are supported
            var seedEndpoint = OnlineGeospatialEndpointsDb.GetById(0);
            if (seedEndpoint.Services.FirstOrDefault(s => s.Protocol == "CSW") != null 
                && seedEndpoint.Provider != null)
            {
                throw new InvalidOperationException("Entire catalog has already been added to the PYXIS Channel.");
            }
            var galleryProvider = UpsertEndpoint(endpoint);

            var dataSets = OnlineGeospatialDatasetDb.SearchByServer(endpoint.Id, null, 0, int.MaxValue);
            UpsertDataSets(galleryProvider, dataSets);

            return galleryProvider;
        }

        /// <summary>
        /// Insert or update, if necessary, the entire catalog as Gallery in the PYXIS Channel.
        /// When inserted, the endpoint's Provider is updated to reflect the created Gallery's Id.
        /// The endpoint's data sets are similarly inserted or updated, if necessary.
        /// When inserted, a data set's Reference property is set to reflect the created GeoSource.
        /// </summary>
        /// <returns>The Gallery Provider associated with the catalog.</returns>
        public static Provider UpsertCatalog()
        {
            // Guard against duplicates in the Gallery until links are supported
            var endpoints = OnlineGeospatialEndpointsDb.Servers.Skip(1);
            if (endpoints.Any(e => e.Provider != null))
            {
                throw new InvalidOperationException("Individual endpoints have already been added to the PYXIS Channel.");
            }

            var cswEndpoint = OnlineGeospatialEndpointsDb.GetById(0);
            if (cswEndpoint.Services.FirstOrDefault(s => s.Protocol == "CSW") == null)
            {
                throw new InvalidOperationException("Catalog must be initialized from a CSW");
            }

            var galleryProvider = UpsertEndpoint(cswEndpoint);

            // include already inserted data sets, then remove duplicate name data sets from newly discovered, Working data sets
            var dataSets = OnlineGeospatialDatasetDb.Datasets;
            var previouslyInsertedDataSets = dataSets.Where(d => d.Reference != null).ToList();
            var previouslyInsertedNames = previouslyInsertedDataSets
                .Select(d => d.Name)
                .Distinct()
                .OrderBy(name => name)
                .ToDictionary(s => s, s => true);
            dataSets = dataSets
                .Where(d => d.Reference == null
                            && d.Services.Any(s => s.Status == OnlineGeospatialServiceStatus.Working)
                            && !previouslyInsertedNames.ContainsKey(d.Name))
                .GroupBy(d => d.Name, d => d, (name, ds) => ds)
                .Select(ds =>
                {
                    var preferredServices = ds.GroupBy(d => SelectPreferredService(d).Protocol, d => d, (p, g) => new KeyValuePair<string, List<OnlineGeospatialDataSet>>(p, g.ToList()))
                        .ToDictionary(g => g.Key, g => g.Value);
                    return preferredServices.ContainsKey(s_protocolPreference[0]) ? preferredServices[s_protocolPreference[0]].First()
                        : preferredServices.ContainsKey(s_protocolPreference[1]) ? preferredServices[s_protocolPreference[1]].First()
                        : preferredServices.ContainsKey(s_protocolPreference[2]) ? preferredServices[s_protocolPreference[2]].First()
                        : preferredServices.First().Value[0];
                })
                .ToList();
            dataSets.AddRange(previouslyInsertedDataSets);
            UpsertDataSets(galleryProvider, dataSets);

            return galleryProvider;
        }

        private static Provider UpsertEndpoint(OnlineGeospatialEndpoint endpoint)
        {
            if (endpoint.Provider == null)
            {
                var gallery = GenerateGallery(endpoint);
                gallery = s_user.PostResource(gallery);
                endpoint.Provider = GenerateProvider(gallery);
                OnlineGeospatialEndpointsDb.Save();
            }

            return endpoint.Provider;
        }

        private static void UpsertDataSets(Provider provider, IEnumerable<OnlineGeospatialDataSet> dataSets)
        {
            // Proceed with data sets that are working or already exist in the Channel
            dataSets = dataSets
                .Where(d => d.Reference != null 
                            || d.Services.Any(s => s.Status == OnlineGeospatialServiceStatus.Working))
                .ToList();

            foreach (var dataSet in dataSets)
            {
                try
                {
                    if (dataSet.Reference == null)
                    {
                        InsertDataSet(provider, dataSet);
                        OnlineGeospatialDatasetDb.Save();
                    }
                    else
                    {
                        UpdateDataSet(dataSet);
                        OnlineGeospatialDatasetDb.Save();
                    }
                }
                catch (TypeInitializationException exception)
                {
                    Console.WriteLine(exception);
                    throw;
                }
                catch (Exception exception)
                {
                    Console.WriteLine(exception);
                }
            }
        }
        
        private static Gallery GenerateGallery(OnlineGeospatialEndpoint endpoint)
        {
            // create Gallery from endpoint
            var gallery = new Gallery
            {
                Metadata = new Metadata
                {
                    Name = endpoint.Name,
                    //Description = endpoint.Id.ToString(),
                    ExternalUrls = new List<ExternalUrl>
                    {
                        new ExternalUrl
                        {
                            Type = ExternalUrlType.Reference,
                            Url = endpoint.Uri.AbsoluteUri
                        }
                    },
                    SystemTags = new List<string> { "Crawled" }
                }
            };
            return gallery;
        }

        private static Provider GenerateProvider(Gallery gallery)
        {
            return new Provider
            {
                Name = gallery.Metadata.Name,
                Id = gallery.Id,
                Type = ResourceType.Gallery
            };
        }

        private static void InsertDataSet(Provider provider, OnlineGeospatialDataSet dataSet)
        {
            GeoSource geoSource = new GeoSource();
            var annotatedService = SelectPreferredService(dataSet);

            if (annotatedService.Definition == null || annotatedService.ProcRef == null)
            {
                geoSource = GetGeoSource(dataSet, annotatedService);

                if (geoSource == null)
                {
                    return;
                }
            }

            PopulateGeoSource(provider, dataSet, annotatedService, geoSource);
            
            geoSource = s_user.PostResource(geoSource);
            dataSet.Reference = new ServiceResourceReference
            {
                Resource = ResourceReference.FromResource(geoSource),
                Service = new OnlineGeospatialService(annotatedService)
            };
        }

        private static void UpdateDataSet(OnlineGeospatialDataSet dataSet)
        {
            var referenceService = dataSet.Reference.Service;
            var annotatedService = dataSet.Services
                .First(s => s.Protocol == referenceService.Protocol
                            && s.Version == referenceService.Version);

            // Check if the status changed to determine if it needs updating
            if (referenceService.Status == annotatedService.Status || annotatedService.Status == OnlineGeospatialServiceStatus.NeedsVerifying)
            {
                return;
            }

            var geoSourceUpdate = new GeoSource();
            geoSourceUpdate.State = annotatedService.Status == OnlineGeospatialServiceStatus.Working ? PipelineDefinitionState.Active : PipelineDefinitionState.Broken;

            var latestGeoSource = s_channel.GeoSources.GetById(dataSet.Reference.Resource.Id);
            geoSourceUpdate.Id = latestGeoSource.Id;
            geoSourceUpdate.Version = latestGeoSource.Version;

            s_user.PutResource(geoSourceUpdate);
            dataSet.Reference.Service = annotatedService;
        }

        private static AnnotatedOnlineGeospatialService SelectPreferredService(OnlineGeospatialDataSet dataSet)
        {
            // Prefer working feature, then coverage, then map services
            var workingServices = dataSet.Services.Where(s => s.Status == OnlineGeospatialServiceStatus.Working).ToList();
            return workingServices.FirstOrDefault(s => s.Protocol == s_protocolPreference[0])
                   ?? workingServices.FirstOrDefault(s => s.Protocol == s_protocolPreference[1])
                   ?? workingServices.FirstOrDefault(s => s.Protocol == s_protocolPreference[2])
                   ?? workingServices.FirstOrDefault(s => s.Status == OnlineGeospatialServiceStatus.Working);
        }

        private static void PopulateGeoSource(Provider provider, OnlineGeospatialDataSet dataSet, AnnotatedOnlineGeospatialService service, GeoSource geoSource)
        {
            geoSource.DataSize = 0;
            geoSource.ProcRef = service.ProcRef;
            geoSource.Definition = service.Definition;
            if (geoSource is MultiDomainGeoSource)
            {
////////////////// Uncomment to include Domains
                //((MultiDomainGeoSource)geoSource).Domains = dataSet.Domains;
            }
            if (geoSource.Metadata == null)
            {
                geoSource.Metadata = new Metadata();
            }
            geoSource.Metadata.Name = dataSet.Name;
            geoSource.Metadata.Description = dataSet.Description;
            geoSource.Metadata.Providers = new List<Provider> { provider };
            geoSource.Metadata.Tags = dataSet.Tags ?? new List<string>();
            geoSource.Metadata.SystemTags = dataSet.SystemTags ?? new List<string>();
            geoSource.Metadata.ExternalUrls = new List<ExternalUrl>
            {
                new ExternalUrl
                {
                    Type = ExternalUrlType.Reference,
                    Url = service.Uri.ToString()
                }
            };
        }

        private static GeoSource GetGeoSource(OnlineGeospatialDataSet dataSet, AnnotatedOnlineGeospatialService annotatedService)
        {
            var retry = 0;
            GeoSource geoSource = null;
            while (geoSource == null && retry < 3)
            {
                geoSource = DataSetVerifier.TryVerify(dataSet, annotatedService);

                if (geoSource == null)
                {
                    retry++;
                }
            }
            return geoSource;
        }
    }
}