using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Search
{
    public class WeightedScorer : IGazetteerScorer
    {
        private List<Tuple<IGazetteerScorer, float>> m_scoresWeights = new List<Tuple<IGazetteerScorer, float>>();

        public enum AggregatedMode
        {
            Sum,
            Multiply,
            Average
        }

        public AggregatedMode Mode { get; set; }

        public WeightedScorer()
        {
            Mode = AggregatedMode.Sum;
        }

        public void Add(IGazetteerScorer scorer, float weight = 1f)
        {
            m_scoresWeights.Add(new Tuple<IGazetteerScorer, float>(scorer,weight));
        }

        public WeightedScorer Search(params string[] words)
        {
            Add(new TextScorer(words),1);
            return this;
        }

        public WeightedScorer Search(float weight, params string[] words)
        {
            Add(new TextScorer(words), weight);
            return this;
        }

        public WeightedScorer SearchAll(params string[] words)
        {
            Add(new TextScorer(words) { NotFoundScale = 0 }, 1);
            return this;
        }

        public WeightedScorer Where(Func<IGazetteerEntry, bool> condition, float weight = 1f)
        {
            Add(new BooleanScorer(condition), weight);
            return this;
        }

        public WeightedScorer Near(double latitude, double longitude, double raidus, float weight = 1f)
        {
            Add(new NearScorer(new PYXBoundingCircle(PointLocation.fromWGS84(latitude, longitude).asXYZ(), raidus/SphereMath.knEarthRadius)),weight);
            return this;
        }

        public WeightedScorer Intersects(double latitude, double longitude, double raidus, float weight = 1f)
        {
            Add(new InterestcsScorer(new PYXBoundingCircle(PointLocation.fromWGS84(latitude, longitude).asXYZ(), raidus / SphereMath.knEarthRadius)), weight);
            return this;
        }

        public WeightedScorer Intersects(BoundingBox boundingBox, float weight = 1f)
        {
            Add(new InterestcsBBoxScorer(boundingBox), weight);
            return this;
        }

        public float Score(IGazetteerEntry entry)
        {
            var score = Mode == AggregatedMode.Multiply ? 1f : 0f;
            
            foreach (var scorer in m_scoresWeights)
            {
                var newScore = scorer.Item1.Score(entry)*scorer.Item2;

                if (Mode == AggregatedMode.Multiply)
                {
                    score *= newScore;
                }
                else
                {
                    score += newScore;
                }
                
            }
            if (Mode == AggregatedMode.Average && m_scoresWeights.Count > 1)
            {
                score /= m_scoresWeights.Count;
            }
            return score;
        }

        public WeightedScorer Clone()
        {
            return new WeightedScorer()
            {
                m_scoresWeights = new List<Tuple<IGazetteerScorer, float>>(m_scoresWeights),
                Mode = Mode
            };
        }
    }
}