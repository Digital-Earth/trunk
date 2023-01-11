namespace Pyxis.IO.Search
{
    public interface IGazetteerAggregator
    {
        void Aggregate(GazetteerEntryScore entry, AggregationResults results);
        void FinalizeResults(AggregationResults results);
    }
}