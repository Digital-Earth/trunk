using System;
using System.IO;

namespace Pyxis.Storage.BlobProviders
{
    public class CachedBlobProvider : MultiProviderBlobProvider
    {
        public CachedBlobProvider(string ServerURL, string cacheDir)
        {
            BlobProviders.Add(new LocalBlobProvider(cacheDir));
            BlobProviders.Add(new PyxisBlobProvider(ServerURL));
        }

        public CachedBlobProvider()
        {
            BlobProviders.Add(new LocalBlobProvider(
                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PYXIS", "BlobCache")));
            BlobProviders.Add(new PyxisBlobProvider());
        }
    }
}