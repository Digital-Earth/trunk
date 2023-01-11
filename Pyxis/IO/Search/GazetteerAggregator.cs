using System;
using System.Collections.Generic;

namespace Pyxis.IO.Search
{
    public class GazetteerAggregator : IGazetteerAggregator
    {
        private Func<IGazetteerEntry, IEnumerable<string>> m_extractValue;

        public const int DefaultMaxResults = 50;
        private int m_maxResultsCount = -1;

        public GazetteerAggregator(Func<IGazetteerEntry, string> extractValue, int maxResults = DefaultMaxResults)
        {
            m_extractValue = (entry) => new[]{extractValue(entry)};
            m_maxResultsCount = maxResults;
        }

        public GazetteerAggregator(Func<IGazetteerEntry, IEnumerable<string>> extractValue, int maxResults = DefaultMaxResults)
        {
            m_extractValue = extractValue;
            m_maxResultsCount = maxResults;
        }

        public void Aggregate(GazetteerEntryScore entry, AggregationResults results)
        {
            foreach (var value in m_extractValue(entry.Entry))
            {
                var safeValue = value ?? "null";

                if (results.Values == null)
                {
                    results.Values = new Dictionary<object, AggregationResults.AggregateCount>();
                }

                if (!results.Values.ContainsKey(safeValue))
                {
                    results.Values[safeValue] = new AggregationResults.AggregateCount();
                }

                results.Values[safeValue].Hits++;
                results.Values[safeValue].TotalScore += entry.Score;
            }
        }

        public void FinalizeResults(AggregationResults results)
        {
            results.LimitResults(m_maxResultsCount);
        }
    }
}