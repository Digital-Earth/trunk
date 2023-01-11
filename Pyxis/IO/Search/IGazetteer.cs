using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Search
{
    public interface IGazetteerEntry
    {
        HashSet<string> Words { get; }
        string Host { get; }
        DataSet DataSet { get; set; }
    }

    public interface IGazetteer
    {
        IEnumerable<GazetteerEntryScore> SearchEntries(IGazetteerScorer filter, IGazetteerScorer scorer = null, float filterThreshold = 0);
        long? Count { get; }
    }
}
