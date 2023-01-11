using System.Collections.Generic;
using System.Linq;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Search
{
    public class DataSetAndScore
    {
        public DataSet DataSet { get; set; }
        public float Score { get; set; }

        public DataSetAndScore()
        {
            
        }

        public DataSetAndScore(DataSet dataSet, float score)
        {
            DataSet = dataSet;
            Score = score;
        }
    }

    public class SearchResults
    {
        public long Hits { get; set; }
        public List<DataSetAndScore> Results { get; set; }
        public Dictionary<string,AggregationResults> Aggregations { get; set; }

        public SearchResults()
        {
            Hits = 0;
            Results = new List<DataSetAndScore>();
            Aggregations = null;
        }

        public void Add(SearchResults other)
        {
            if (other.Results != null)
            {
                Results = Results.Concat(other.Results).OrderByDescending(x => x.Score).ToList();
            }
            Hits += other.Hits;

            if (other.Aggregations != null)
            {
                if (Aggregations == null)
                {
                    Aggregations = new Dictionary<string, AggregationResults>();
                }

                foreach (var keyValue in other.Aggregations)
                {
                    if (Aggregations.ContainsKey(keyValue.Key))
                    {
                        Aggregations[keyValue.Key].Merge(keyValue.Value);
                    }
                }
            }
        }
    }
}