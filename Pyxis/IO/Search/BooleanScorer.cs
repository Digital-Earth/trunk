using System;

namespace Pyxis.IO.Search
{
    public class BooleanScorer : IGazetteerScorer
    {
        private Func<IGazetteerEntry, bool> m_score;

        public BooleanScorer(Func<IGazetteerEntry, bool> extractFloat)
        {
            m_score = extractFloat;
        }

        public float Score(IGazetteerEntry entry)
        {
            return m_score(entry) ? 1f : 0f;
        }
    }
}