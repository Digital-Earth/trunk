using System.Collections.Generic;
using System.Linq;

namespace Pyxis.IO.Search
{
    public class AggregationResults
    {
        public class AggregateCount
        {
            public long Hits {get; set; }
            public float TotalScore { get; set; }
        }

        public Dictionary<object, AggregateCount> Values { get; set; }

        public void Merge(AggregationResults other)
        {
            foreach (var keyValue in other.Values)
            {
                if (!Values.ContainsKey(keyValue.Key))
                {
                    Values[keyValue.Key] = new AggregateCount()
                    {
                        Hits = 0,
                        TotalScore = 0
                    };
                }

                var valueToUpdate = Values[keyValue.Key];
                valueToUpdate.Hits += keyValue.Value.Hits;
                valueToUpdate.TotalScore += keyValue.Value.TotalScore;
            }
        }

        public void LimitResults(int numberOfResults = 50)
        {
            if (Values == null || Values.Count < numberOfResults)
            {
                return;
            }

            Values = Values.OrderByDescending(x => x.Value.TotalScore)
                .Take(numberOfResults)
                .ToDictionary(kv => kv.Key, kv => kv.Value);
        }
    }
}