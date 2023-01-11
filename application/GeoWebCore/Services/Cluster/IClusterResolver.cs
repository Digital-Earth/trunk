namespace GeoWebCore.Services.Cluster
{
    public interface IClusterResolver
    {
        string Key { get; }
        bool IsLocal { get; }
        bool IsReady { get; }
        bool IsFaulted { get; }
    }
}