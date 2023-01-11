using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Search
{
    public interface IGazetteerScorer
    {
        float Score(IGazetteerEntry entry);
    }
}