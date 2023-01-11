using System;

namespace Pyxis.IO.Search
{
    public class FloatScorer : IGazetteerScorer
    {
        private Func<IGazetteerEntry, float> m_score;

        public FloatScorer(Func<IGazetteerEntry, float> extractFloat)
        {
            m_score = extractFloat;
        }

        public FloatScorer(Func<IGazetteerEntry, float> extractFloat, float idealValue, float range)
        {
            m_score = (IGazetteerEntry entry) =>
            {
                var value = extractFloat(entry) - idealValue;
                return Gauss(value, idealValue, range);
            };
        }

        private float Gauss(float x, float mean, float std)
        {
            var value = x - mean;
            var valueSquare = value*value;

            return (float) Math.Exp(-valueSquare/std);
        }

        public float Score(IGazetteerEntry entry)
        {
            return m_score(entry);
        }
    }
}