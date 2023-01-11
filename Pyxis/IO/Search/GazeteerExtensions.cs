using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Search
{
    public static class GazeteerExtensions
    {
        public static IEnumerable<DataSet> Search(this IGazetteer gazetteer, IGazetteerScorer filter, IGazetteerScorer scorer = null, float filterThreshold = 0)
        {
            return gazetteer.SearchEntries(filter, scorer, filterThreshold).Select(entry => entry.Entry.DataSet);
        }

        public static SearchResults Search(this IGazetteer gazetteer, SearchRequest request)
        {
            var entries = gazetteer
                .SearchEntries(request.Filter, request.Scorer, request.FilterThershold)
                .OrderByDescending(entry => entry.Score)
                .ToList();

            var results = new SearchResults()
            {
                Hits = entries.Count,
                Results = entries.Select(x => new DataSetAndScore(x.Entry.DataSet,x.Score)).ToList(),
            };

            if (request.Aggregations != null)
            {
                results.Aggregations = new Dictionary<string, AggregationResults>();

                foreach (var nameAndAggregator in request.Aggregations)
                {
                    var aggregationResults = new AggregationResults();

                    foreach (var entry in entries)
                    {
                        nameAndAggregator.Value.Aggregate(entry, aggregationResults);
                    }

                    nameAndAggregator.Value.FinalizeResults(aggregationResults);

                    results.Aggregations[nameAndAggregator.Key] = aggregationResults;
                }
            }

            return results;
        }

        private static DataSet GenerateDiscoveryReportIfMissing(DataSet dataSet)
        {
            if (dataSet.Specification != null &&
                dataSet.Specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage &&
                dataSet.DiscoveryReport == null)
            {
                dataSet.DiscoveryReport = new DataSetDiscoveryReport();
            }
            return dataSet;
        }

        /// <summary>
        /// Remove duplicated datasets.
        /// </summary>
        /// <param name="dataSets">List of Datasets</param>
        /// <returns>List of Datasets with removed duplicated datasets</returns>
        public static List<DataSet> RemoveDuplicates(this IEnumerable<DataSet> dataSets)
        {
            var groups = dataSets
               .Select(GenerateDiscoveryReportIfMissing)
               .Where(x => x.DiscoveryReport != null && x.BBox != null)
               .GroupBy(x => GenerateDatasetKey(x));

            var results = new List<DataSet>();

            foreach (var group in groups)
            {
                var uniqueRemovalLog = new Dictionary<string, List<string>>();

                var candidates = group.OrderByDescending(x => x.Fields.Count).ToList();

                var uniqueDatasets = new List<DataSet>() { };

                foreach (var candidate in candidates)
                {
                    var fields = new HashSet<string>(candidate.Fields);
                    var isUnique = true;
                    //var isCoverage = candidate.Specification != null &&
                    //                 candidate.Specification.OutputType ==
                    //                 PipelineSpecification.PipelineOutputType.Coverage;

                    foreach (var uniqueDataset in uniqueDatasets)
                    {
                        if (uniqueDataset.Metadata.Name == candidate.Metadata.Name &&
                            fields.IsSubsetOf(uniqueDataset.Fields))
                        {
                            uniqueRemovalLog[uniqueDataset.Uri].Add(candidate.Uri);

                            isUnique = false;
                            break;
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

        private static string GenerateDatasetKey(DataSet x)
        {
            if (x.Specification != null)
            {
                if (x.Specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
                {
                    return x.Specification.OutputType.ToString() + ":" + x.DiscoveryReport.FeaturesCount + ":" + JsonConvert.SerializeObject(x.BBox);        
                }
                else if (x.Specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
                {
                    return x.Specification.OutputType.ToString() + ":" + JsonConvert.SerializeObject(x.BBox);
                }
            }
            return JsonConvert.SerializeObject(x.BBox);
        }
    }
}