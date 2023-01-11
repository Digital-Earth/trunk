using System.Linq;

namespace Pyxis.IO.Search
{
    public class TextScorer : IGazetteerScorer
    {
        private readonly string[] m_searchWords;
        public float NotFoundScale { get; set; }

        public TextScorer(params string[] searchWords)
        {
            m_searchWords = searchWords;
            NotFoundScale = 0.5f;
        }

        public float Score(IGazetteerEntry entry)
        {
            var score = 0f;
            var factor = 1f;

            foreach (var word in m_searchWords.Select(word => word.ToLower().Trim()))
            {
                if (entry.Words.Contains(word))
                {
                    score += 1;
                }
                else if (entry.Words.Any(x => x.StartsWith(word)))
                {
                    score += 0.25f;
                }
                else if (entry.Words.Any(x => x.Contains(word)))
                {
                    score += 0.1f;
                }
                else
                {
                    //reduce score as this word didn't match
                    factor *= NotFoundScale;
                }
            }
            return score * factor;
        }
    }
}