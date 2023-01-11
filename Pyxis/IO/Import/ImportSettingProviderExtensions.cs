using System;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Import
{
    public static class ImportSettingProviderExtensions
    {
        public static ImportSettingProvider SRS(this ImportSettingProvider provider, string wktSrs)
        {
            return provider.SRS(SpatialReferenceSystem.CreateFromWKT(wktSrs));
        }

        public static ImportSettingProvider SRS(this ImportSettingProvider provider, SpatialReferenceSystem srs)
        {
            provider.Register(new SRSImportSetting
            {
                SRS = srs
            });
            return provider;
        }

        public static ImportSettingProvider DownloadLocally(this ImportSettingProvider provider, string localPath)
        {
            provider.Register(new DownloadLocallySetting {Path = localPath});
            return provider;
        }

        public static ImportSettingProvider Sampler(this ImportSettingProvider provider, Guid sampler)
        {
            provider.Register(new SamplerImportSetting
            {
                Sampler = sampler.ToString()
            });
            return provider;
        }
    }
}
