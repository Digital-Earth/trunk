using System.Collections.Generic;

namespace Pyxis.IO.Search
{
    public class SearchRequest
    {
        public IGazetteerScorer Filter { get; set; }
        public IGazetteerScorer Scorer { get; set; }

        public Dictionary<string,IGazetteerAggregator> Aggregations { get; set; }
        public float FilterThershold { get; set; }
    }
}