using System.Collections.Generic;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Search
{
    public class GazetteerEntry : IGazetteerEntry
    {
        public HashSet<string> Words { get; set; }
        public PYXBoundingCircle BoundingCircle { get; set; }
        public DataSet DataSet { get; set; }
        public string Host { get; set; }
    }
}