using Pyxis.Contract.DataDiscovery;
using Pyxis.Core.IO;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// SRSImportSetting is requested when a GeoSource doesn't provide information on what SpatialReferenceSystem to use
    /// </summary>
    public class SamplerImportSetting : IImportSetting
    {
        /// <summary>
        /// SpatialReferenceSystem to be used when importing a GeoSource
        /// </summary>
        public string Sampler { get; set; }
    }
}
