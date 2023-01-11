using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Authentication;
using MongoDB.Bson;
using MongoDB.Bson.Serialization;
using MongoDB.Bson.Serialization.Attributes;
using MongoDB.Bson.Serialization.Options;
using MongoDB.Bson.Serialization.Serializers;
using MongoDB.Driver;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Newtonsoft.Json;
using Polly;

namespace PyxisCLI.State
{
    public class MongoUrlDiscoveryReport : UrlDiscoveryReport
    {
        [BsonId]
        public ObjectId Id { get; set; }
    }

    public class MongoDataSet : DataSet
    {
        [BsonId]
        public ObjectId Id { get; set; }

        public string Root { get; set; }

        public MongoDataSet()
        {

        }

        public MongoDataSet(DataSet dataSet, string root) : base(dataSet)
        {
            Root = root;
        }
    }

    public class MongoDataSetChangeInfo
    {
        public DataSet NewDataset { get; set; }
        public MongoDataSet ExistingDataset { get; set; }
    }

    public class CommitChangeReport
    {
        public int InsertCount { get; set; }
        public int ReplaceCount { get; set; }
        public int DeleteCount { get; set; }
        public int NothingChangedCount { get; set; }
    }

    static class MongoPersistance
    {
        private static MongoClient s_mongoClient;
        private static IMongoDatabase s_mongoDatabase;

        private static readonly string DatabaseName = "crawler";
        private static readonly string RootsCollectionName = "roots";
        private static readonly string DataSetsCollectionName = "datasets";

        static MongoPersistance()
        {
            string connectionString = PyxisCliConfig.Config.Mongo;

            MongoClientSettings settings = MongoClientSettings.FromUrl(
                new MongoUrl(connectionString)
            );

            settings.SslSettings =
                new SslSettings() {EnabledSslProtocols = SslProtocols.Tls12};

            s_mongoClient = new MongoClient(settings);
            s_mongoDatabase = s_mongoClient.GetDatabase(DatabaseName);

            BsonClassMap.RegisterClassMap<Domain>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Type)
                    .SetSerializer(new EnumSerializer<PipelineSpecification.FieldType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification.FieldUnit>(cm =>
            {
                cm.AutoMap();
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification.FieldSpecification>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.FieldType)
                    .SetSerializer(new EnumSerializer<PipelineSpecification.FieldType>(BsonType.String));
                var seralizer =
                    new DictionaryInterfaceImplementerSerializer<Dictionary<string, string>>(
                        DictionaryRepresentation.ArrayOfArrays);
                cm.GetMemberMap(c => c.ValueTranslation).SetSerializer(seralizer);
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.OutputType)
                    .SetSerializer(
                        new NullableSerializer<PipelineSpecification.PipelineOutputType>(
                            new EnumSerializer<PipelineSpecification.PipelineOutputType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<DataSetDiscoveryReport>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.Status)
                    .SetSerializer(new EnumSerializer<DataSetDiscoveryStatus>(BsonType.String));
            });
        }

        private static Policy CreateRetryPolicy()
        {
            return Policy.Handle<Exception>().WaitAndRetry(3, (retryAttempt) => TimeSpan.FromSeconds(Math.Pow(2, retryAttempt)));
        }

        public static IEnumerable<UrlDiscoveryReport> GetRootsToDiscover(TimeSpan refreshRate)
        {
            var threshold = DateTime.UtcNow.Subtract(refreshRate);

            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                    .Find(x => x.Status == "New" || x.LastDiscovered < threshold)
                    .SortBy(x => x.LastDiscovered)
                    .ToEnumerable());
        }

        public static IEnumerable<UrlDiscoveryReport> GetRoots()
        {
            return CreateRetryPolicy().Execute(() => s_mongoDatabase
                .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                .Find(x => x.Uri != "")
                .ToEnumerable());
        }


        public static UrlDiscoveryReport GetRoot(string uri)
        {

            return CreateRetryPolicy().Execute(() => s_mongoDatabase
                .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName).Find(x => x.Uri == uri).FirstOrDefault());
        }

        /// <summary>
        /// Take a potential url and check agaisnt the database to see if there is any conflict with existing roots
        /// </summary>
        /// <param name="uri">url needs to be added</param>
        /// <returns>The root from database that conflicts with input</returns>
        public static UrlDiscoveryReport GetConflictedRoot(string uri)
        {
            Func<string, string> sanitizeUrl = (string s) => s.Replace("http://", "https://").Trim('/').ToLower();
            var existingRoots = GetRootSuggestion(uri);
            UrlDiscoveryReport result = null;

            foreach (var url in existingRoots)
            {
                var a = sanitizeUrl(uri);
                var b = sanitizeUrl(url.Uri);

                if (a == b || a.StartsWith(b) || b.StartsWith(a))
                {
                    result = url;
                    break;
                }
            }

            return result;
        }

        public static List<UrlDiscoveryReport> GetRootSuggestion(string uri)
        {
            string host = new Uri(uri).Host;

            return CreateRetryPolicy().Execute(() =>
            {
                List<UrlDiscoveryReport> resultList = s_mongoDatabase
                    .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                    .Find(x => x.Host == host)
                    .ToEnumerable()
                    .Cast<UrlDiscoveryReport>()
                    .ToList();

                return resultList;
            });
        }

        public static UrlDiscoveryReport AddRoot(string uri, string serviceIdentifier = "")
        {
            var discoveryReport = new MongoUrlDiscoveryReport()
            {
                Id = ObjectId.GenerateNewId(),
                Uri = uri,
                Host = new Uri(uri).Host,
                ServiceIdentifier = serviceIdentifier,
                Status = "New",
                DataSetCount = 0,
                VerifiedDataSetCount = 0,
                BrokenDataSetCount = 0,
                UnknownDataSetCount = 0,
                LastDiscovered = DateTime.MinValue,
                LastVerified = DateTime.MinValue,
                Metadata = new SimpleMetadata()
                {
                    Name = new Uri(uri).Host,
                    Description = ""
                }
            };

            CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                    .ReplaceOne(x => x.Uri == uri, discoveryReport, new UpdateOptions() {IsUpsert = true}));

            return discoveryReport;
        }

        public static void RemoveRoot(string uri)
        {
            CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                    .DeleteMany(root => root.Uri == uri));

            CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .DeleteMany(dataSet => dataSet.Root == uri));
        }

        public static void UpdateRoot(UrlDiscoveryReport urlDiscovery)
        {
            var mongoUrlDiscovery = urlDiscovery as MongoUrlDiscoveryReport;
            if (mongoUrlDiscovery != null)
            {
                CreateRetryPolicy().Execute(() =>
                    s_mongoDatabase.GetCollection<MongoUrlDiscoveryReport>(RootsCollectionName)
                        .ReplaceOne(x => x.Id == mongoUrlDiscovery.Id, mongoUrlDiscovery));
            }
        }

        public class IssueAndCount
        {
            public string Issue { get; set; }
            public int Count { get; set; }
        }

        public static IEnumerable<IssueAndCount> GetDataSetsIssues()
        {
            return
                CreateRetryPolicy().Execute(() =>
                {
                    return s_mongoDatabase
                        .GetCollection<MongoDataSet>(DataSetsCollectionName)
                        .Find(dataSet=>dataSet.DiscoveryReport.Status == DataSetDiscoveryStatus.Failed)
                        .Project(dataset => dataset.DiscoveryReport.Issues)
                        .ToList()
                        .SelectMany(issues=>issues)
                        .GroupBy(issue=>issue)
                        .Select(x=>new IssueAndCount{ Issue = x.Key, Count = x.Count() });
                });
        }

        public static IEnumerable<DataSet> GetDataSetsForRoot(string url)
        {
            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .Find(dataSet => dataSet.Root == url)
                    .ToEnumerable());
        }

        public static IEnumerable<DataSet> GetVerifiedDataSetsForRoot(string url)
        {
            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .Find(
                        dataSet =>
                            dataSet.Root == url &&
                            dataSet.DiscoveryReport.Status == DataSetDiscoveryStatus.Successful)
                    .ToEnumerable());
        }

        public static IEnumerable<DataSet> GetUnverifiedDataSetsForRoot(string url)
        {
            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .Find(
                        dataSet =>
                            dataSet.Root == url &&
                            (dataSet.DiscoveryReport == null ||
                             dataSet.DiscoveryReport.Status == DataSetDiscoveryStatus.Unknown))
                    .ToEnumerable());
        }


        public static Dictionary<DataSetDiscoveryStatus,long> CountDataSetsStatusForRoot(string url)
        {
            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .Find(dataSet => dataSet.Root == url)
                    .Project(dataset => dataset.DiscoveryReport.Status)
                    .ToList().GroupBy(x=>x).ToDictionary(x=>x.Key,x=>x.LongCount()));
        }

        public static long CountDataSetsForRoot(string url)
        {
            return CreateRetryPolicy().Execute(() =>
                s_mongoDatabase
                    .GetCollection<MongoDataSet>(DataSetsCollectionName)
                    .Count(dataSet => dataSet.Root == url));
        }

        /// <summary>
        /// Retrieve new datasets from targeted servers and old ones from our database. This method is used in pair with CommitDataSetChanges
        /// </summary>
        /// <param name="report"></param>
        /// <param name="datasets"></param>
        /// <returns></returns>
        public static List<MongoDataSetChangeInfo> PrepareChangesForRoot(UrlDiscoveryReport report, List<DataSet> datasets)
        {
            List<MongoDataSetChangeInfo> result = new List<MongoDataSetChangeInfo>();

            var originalList =
                CreateRetryPolicy().Execute(() =>
                    s_mongoDatabase
                        .GetCollection<MongoDataSet>(DataSetsCollectionName)
                        .Find(dataSet => dataSet.Root == report.Uri)
                        .ToList());

            //use hash function to compare the datasets
            Func<DataSet, string> hashDataset =
                dataSet => string.Format("{0}:{1}:{2}", dataSet.Uri, dataSet.Layer, string.Join(",", dataSet.Fields ?? new List<string>()));

            var indexedExistingDataSets = originalList.ToDictionary(hashDataset, (MongoDataSet x) => x);
            var indexedNewDataSets = datasets.ToDictionary(hashDataset, x => x);

            foreach(var key in indexedExistingDataSets.Keys.Concat(indexedNewDataSets.Keys).Distinct())
            {
                MongoDataSetChangeInfo datasetInfo = new MongoDataSetChangeInfo()
                {
                    ExistingDataset = indexedExistingDataSets.ContainsKey(key) ? indexedExistingDataSets[key] : null,
                    NewDataset = indexedNewDataSets.ContainsKey(key) ? indexedNewDataSets[key] : null
                };

                result.Add(datasetInfo);
            }

            return result;
        }

        /// <summary>
        /// Identify which dataset is new, existing, and old. Then, perform either insert for new, replace for existing (if there is any changes), and delete for old datasets
        /// </summary>
        /// <param name="report"></param>
        /// <param name="datasetsChanges"></param>
        /// <returns>List of statistical information</returns>
        public static CommitChangeReport CommitChangesForRoot(UrlDiscoveryReport report, List<MongoDataSetChangeInfo> datasetsChanges)
        {
            var result = new CommitChangeReport();
            var collection = s_mongoDatabase.GetCollection<MongoDataSet>(DataSetsCollectionName);

            foreach (var changeInfo in datasetsChanges)
            {
                if (changeInfo.NewDataset != null)
                {
                    if (changeInfo.ExistingDataset == null)
                    {
                        CreateRetryPolicy().Execute(() =>
                                collection.InsertOne(new MongoDataSet(changeInfo.NewDataset, report.Uri)));
                        result.InsertCount++;
                    }
                    else
                    {
                        //Compare the 2 datasets again for any other changes such as metadata and discovery report
                        var string1 = JsonConvert.SerializeObject(changeInfo.NewDataset);
                        var string2 = JsonConvert.SerializeObject(new DataSet(changeInfo.ExistingDataset));

                        if (string1 != string2)
                        {
                            CreateRetryPolicy().Execute(() =>
                                collection.ReplaceOne(x => x.Id == changeInfo.ExistingDataset.Id,
                                    new MongoDataSet(changeInfo.NewDataset, report.Uri)
                                    {
                                        Id = changeInfo.ExistingDataset.Id
                                    }));
                            result.ReplaceCount++;
                        }
                        else
                        {
                            result.NothingChangedCount++;
                        }
                    }
                }
                else if (changeInfo.ExistingDataset != null)
                {
                    CreateRetryPolicy().Execute(() =>
                            collection.DeleteOne(x => x.Id == changeInfo.ExistingDataset.Id));
                    result.DeleteCount++;
                }
                else
                {
                    result.NothingChangedCount++;
                }
            }

            UpdateRoot(report);
            return result;
        }

        public static void UpdateDataSet(DataSet dataSet)
        {
            var mongoDataSet = dataSet as MongoDataSet;
            if (mongoDataSet != null)
            {
                CreateRetryPolicy().Execute(() =>
                    s_mongoDatabase.GetCollection<MongoDataSet>(DataSetsCollectionName)
                        .ReplaceOne(x => x.Id == mongoDataSet.Id, mongoDataSet));
            }
        }
    }
}
