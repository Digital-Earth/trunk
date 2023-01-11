namespace Pyxis.IO.Search
{
    public class GazetteerEntryScore 
    {
        public IGazetteerEntry Entry { get; set; }
        public float Score { get; set; }

        public static GazetteerEntryScore operator *(GazetteerEntryScore entry, float boost)
        {
            return new GazetteerEntryScore()
            {
                Entry = entry.Entry,
                Score = entry.Score*boost
            };
        }
    }
}