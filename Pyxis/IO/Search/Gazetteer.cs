using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;

namespace Pyxis.IO.Search
{
    public class Gazetteer : IGazetteer
    {
        private readonly object m_lock = new object();
        private Tokenizer m_tokenizer;
        private List<GazetteerEntry> m_entries;

        public long? Count
        {
            get { return m_entries.Count; }
        }

        public Gazetteer(Tokenizer tokenizer = null)
        {
            m_tokenizer = tokenizer ?? new Tokenizer();
            m_entries = new List<GazetteerEntry>();
        }

        public void Add(DataSet dataSet)
        {
            var entry = new GazetteerEntry()
            {
                DataSet = dataSet,
                Words = new HashSet<string>(),
                Host = new Uri(dataSet.Uri).Host
            };

            foreach (var word in m_tokenizer.Tokenize(dataSet.Metadata.Name, dataSet.Metadata.Description))
            {
                entry.Words.Add(word);
            }

            if (dataSet.Specification != null && dataSet.Specification.Fields != null)
            {
                foreach (var word in m_tokenizer.Tokenize(dataSet.Specification.Fields.Select(field => field.Metadata.Name).ToArray()))
                {
                    entry.Words.Add(word);
                }
            }

            if (dataSet.BBox != null)
            {
                entry.BoundingCircle = CreateBoundingCircle(dataSet.BBox);
            }

            lock (m_lock)
            {
                m_entries.Add(entry);
            }
        }

        private PYXBoundingCircle CreateBoundingCircle(List<BoundingBox> dataSetBBox)
        {
            try
            {
                foreach (var bbox in dataSetBBox)
                {
                    var coordConverter = PYXCOMFactory.CreateCoordConvertorFromWKT(bbox.Srs ?? "4326");
                    var rect = new PYXRect2DDouble(bbox.LowerLeft.X, bbox.LowerLeft.Y, bbox.UpperRight.X, bbox.UpperRight.Y);
                    var geometry = PYXXYBoundsGeometry.create(rect, coordConverter.get(), 24);
                    return geometry.getBoundingCircle();
                }
            }
            catch (Exception)
            {
                return null;
            }

            return null;
        }

        public void AddRange(IEnumerable<DataSet> dataSets)
        {
            Parallel.ForEach(dataSets, Add);
        }

        public IEnumerable<GazetteerEntryScore> SearchEntries(IGazetteerScorer filter, IGazetteerScorer scorer = null, float filterThreshold = 0)
        {
            scorer = scorer ?? filter;

            lock (m_lock)
            {
                return m_entries
                    //filter results
                    .Where(entry => filter.Score(entry) > filterThreshold)
                    //add score
                    .Select(entry => new GazetteerEntryScore()
                    {
                        Entry = entry, 
                        Score = scorer.Score(entry)
                    });
            }
        }
    }
}
