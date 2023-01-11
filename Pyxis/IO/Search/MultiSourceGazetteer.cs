using System.Collections.Generic;
using System.Linq;

namespace Pyxis.IO.Search
{
    public class MultiSourceGazetteer : IGazetteer
    {
        private class Source
        {
            public IGazetteer Gazetteer { get; set; }
            public string Name { get; set; }
            public float Boost { get; set; }
        }

        private List<Source> Sources { get; set; }

        public long? Count
        {
            get
            {
                if (Sources.All(x => x.Gazetteer.Count.HasValue))
                {
                    return Sources.Sum(x => x.Gazetteer.Count ?? 0);
                }
                return null;
            }
        }


        public MultiSourceGazetteer()
        {
            Sources = new List<Source>();
        }

        public MultiSourceGazetteer AddSource(string name, IGazetteer source, float boost = 1.0f) 
        {
            Sources.Add(new Source()
            {
                Gazetteer = source,
                Name = name,
                Boost = boost
            });
            return this;
        }

        public IEnumerable<GazetteerEntryScore> SearchEntries(IGazetteerScorer filter, IGazetteerScorer scorer = null, float filterThreshold = 0)
        {
            return
                Sources.SelectMany(
                    source =>
                        source.Gazetteer.SearchEntries(filter, scorer, filterThreshold)
                            .Select(entryScore => entryScore * source.Boost));
        }
    }
}