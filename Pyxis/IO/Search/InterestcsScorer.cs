using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Search
{
    public class InterestcsScorer : IGazetteerScorer
    {
        private PYXBoundingCircle m_boundingCircle;

        public InterestcsScorer(PYXBoundingCircle circle)
        {
            m_boundingCircle = circle;
        }

        public float Score(IGazetteerEntry entry)
        {
            var entryWithBoundingCircle = entry as GazetteerEntry;
            
            if (entryWithBoundingCircle == null)
            {
                return 0;
            }

            if (entryWithBoundingCircle.BoundingCircle == null)
            {
                return 0;
            }

            if (m_boundingCircle.intersects(entryWithBoundingCircle.BoundingCircle))
            {
                return 1;
            }

            return 0;
        }
    }

    public class InterestcsBBoxScorer : IGazetteerScorer
    {
        private BoundingBox m_boundingBox;

        public InterestcsBBoxScorer(BoundingBox boundingBox)
        {
            m_boundingBox = boundingBox;
        }

        public float Score(IGazetteerEntry entry)
        {
            if (entry.DataSet.BBox == null || entry.DataSet.BBox.Count == 0)
            {
                return 0;
            }

            if (m_boundingBox.Intersects(entry.DataSet.BBox[0]))
            {
                return 1;
            }

            return 0;
        }
    }
}