using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Search;
using Pyxis.Utilities;

namespace PyxisCLI.State
{
    class DiscoverySummary
    {
        public int LeadCount { get; set; }

        public string ServiceIdentifier { get; set; }

        public List<DataSet> DataSets { get; set; }

        public List<DiscoveryRequest> AdditionalRoots { get; set; }
    }

    static class LocalGazetteer
    {
        public static DiscoverySummary DiscoverRecursively(Engine engine, string url, int expectedDataSetCount = 0)
        {
            var serviceIdentifier = "";
            var dataSets = new List<DataSet>();
            var additionalRoots = new List<DiscoveryRequest>();
            var rootUrl = url;

            var progress = 0;
            var leadCount = 0;

            //urls to discover
            var urlStack = new Stack<string>();

            //urls that have been discover (to avoid rediscovery of the same urls)
            var urlDiscoveredSet = new HashSet<string>();

            while (url.HasContent())
            {
                leadCount++;
                Console.Write(url + " --> ");

                urlDiscoveredSet.Add(url);

                try
                {
                    var result = engine.Discover(url).Result;

                    if (result.ServiceIdentifier != null)
                    {
                        //update the root service identifier
                        if (rootUrl == url)
                        {
                            serviceIdentifier = result.ServiceIdentifier;
                        }

                        var allDataSetsFromResult = new List<DataSet>();

                        if (result.DataSet != null)
                        {
                            allDataSetsFromResult.Add(result.DataSet);
                        }
                        if (result.AdditionalDataSets != null)
                        {
                            allDataSetsFromResult.AddRange(result.AdditionalDataSets);
                        }

                        if (allDataSetsFromResult.Count > 0)
                        {
                            if (allDataSetsFromResult.Count == 1)
                            {
                                Console.WriteLine("{0} + DataSet", result.ServiceIdentifier);
                            }
                            else
                            {
                                Console.WriteLine("{0} + {1} DataSets", result.ServiceIdentifier, allDataSetsFromResult.Count);
                            }

                            foreach (var dataSet in allDataSetsFromResult)
                            {
                                try
                                {
                                    dataSet.BBox = dataSet.BBox.ConvertBBoxToWgs84();
                                }
                                catch
                                {
                                    Console.WriteLine("Failed to convert bbox to wgs84");
                                }

                                dataSets.Add(dataSet);
                            }

                            if (dataSets.Count < expectedDataSetCount)
                            {
                                var newProgress = Math.Min(99, 100 * dataSets.Count / expectedDataSetCount);
                                if (newProgress != progress)
                                {
                                    progress = newProgress;
                                    AutomationLog.UpdateInfo("progress", progress);
                                    AutomationLog.UpdateInfo("datasets", dataSets.Count);
                                }
                            }
                            else if (dataSets.Count % 5 == 0 || allDataSetsFromResult.Count > 5)
                            {
                                AutomationLog.UpdateInfo("datasets", dataSets.Count);
                            }
                        }
                        else
                        {
                            Console.WriteLine(result.ServiceIdentifier);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Not supported");
                    }

                    if (result.Leads != null)
                    {
                        foreach (var lead in result.Leads.AsEnumerable().Reverse())
                        {
                            if (!urlDiscoveredSet.Contains(lead.Uri))
                            {
                                urlStack.Push(lead.Uri);    
                            }
                        }
                    }

                    if (result.AdditionalRoots != null)
                    {
                        foreach (var root in result.AdditionalRoots)
                        {
                            if (!additionalRoots.Contains(root))
                            {
                                additionalRoots.Add(root);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error: " + ex.Message);
                }

                url = null;

                if (urlStack.Count > 0)
                {
                    url = urlStack.Pop();
                }
            }

            AutomationLog.UpdateInfo("progress", 100);
            AutomationLog.UpdateInfo("datasets", dataSets.Count);

            return new DiscoverySummary
            {
                ServiceIdentifier = serviceIdentifier,
                LeadCount = leadCount,
                DataSets = dataSets,
                AdditionalRoots = additionalRoots
            };
        }

        public static DiscoveryResult DiscoveryOnce(Engine engine, string url)
        {
            try
            {
                var result = engine.Discover(url).Result;
                if (result.ServiceIdentifier != null)
                {
                    return result;
                }
                return null;
            }
            catch
            {
                return null;
            }
        }

        private static string HashUrl(string url)
        {
            var bytes = Encoding.UTF8.GetBytes(url);
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            var checksum = sha256ManagedChecksum.ComputeHash(bytes);
            var key = Convert.ToBase64String(checksum).Replace("=", "").Replace("/", "").Replace("+", "").Replace("-", "");
            return key;
        }

        public static string GetLocalDir(DataSet dataSet)
        {
            return LocalDirPersistance.SubDirectory(Path.Combine(GetLocalDirForServer(new Uri(dataSet.Uri)), HashUrl(dataSet.Uri)));
        }

        public static string GetLocalDirForServer(Uri uri)
        {
            return LocalDirPersistance.SubDirectory(uri.Host);
        }

        public static List<DataSet> DetectDuplicatedDataSets(IEnumerable<DataSet> dataSets)
        {
            var groups =
                dataSets.Where(x => x.DiscoveryReport != null && x.BBox != null)
                    .GroupBy(x => x.DiscoveryReport.FeaturesCount + JsonConvert.SerializeObject(x.BBox));

            var results = new List<DataSet>();

            foreach (var group in groups)
            {
                var uniqueRemovalLog = new Dictionary<string,List<string>>();

                var candidates = group.OrderByDescending(x=>x.Fields != null ? x.Fields.Count : 0).ToList();

                var uniqueDatasets = new List<DataSet>() {};

                foreach (var candidate in candidates)
                {
                    var fields = new HashSet<string>(candidate.Fields ?? new List<string>());
                    var isUnique = true;
                    var isCoverage = candidate.Specification != null &&
                                     candidate.Specification.OutputType ==
                                     PipelineSpecification.PipelineOutputType.Coverage;

                    if (!isCoverage)
                    {
                        foreach (var uniqueDataset in uniqueDatasets)
                        {
                            if (fields.IsSubsetOf(uniqueDataset.Fields))
                            {
                                uniqueRemovalLog[uniqueDataset.Uri].Add(candidate.Uri);

                                isUnique = false;
                                break;
                            }
                        }
                    }
                    
                    if (isUnique)
                    {
                        uniqueDatasets.Add(candidate);
                        uniqueRemovalLog[candidate.Uri] = new List<string>();
                    }
                }

                results.AddRange(uniqueDatasets);
            }

            return results;
        }
    }
}
