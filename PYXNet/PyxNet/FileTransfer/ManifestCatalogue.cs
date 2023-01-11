using System;
using System.Linq;

namespace PyxNet.FileTransfer
{

    /// <summary>
    /// Provides a catalogue of available manifests for a product.
    /// The catalogue obtains manifests from a variety of sources.
    /// </summary>
    public class ManifestCatalogue
    {
        private static System.Net.WebClient s_webClient;

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestCatalogue"/> class.
        /// </summary>
        static ManifestCatalogue()
        {
            s_webClient = new System.Net.WebClient();
            // setting proxy to null avoids a time-consuming initialization 
            // (Proxy = WebRequest.GetSystemWebProxy is reported better when using VPN but slower in the general case)
            s_webClient.Proxy = null;
        }

        /// <summary>
        /// Default: Searches all available sources
        /// Web: Searches only online
        /// Locally: Searches only for local versions
        /// </summary>
        public enum SearchModeOptions { Default, Web, Locally };

        static public PyxNet.Service.ResourceInstanceFact GetProductVersion(Pyxis.Utilities.ProductSpecification product, SearchModeOptions searchMode)
        {
            PyxNet.Service.ResourceInstanceFact productVersionFact = null;
            if (searchMode == SearchModeOptions.Default || searchMode == SearchModeOptions.Web)
            {
                productVersionFact = GetProductVersionFromWebAuthority(product);
                if (productVersionFact != null)
                {
                    return productVersionFact;
                }
            }
            if (searchMode == SearchModeOptions.Default || searchMode == SearchModeOptions.Locally)
            {
                productVersionFact = GetProductVersionFromRegistry(product);
            }
            // returns null if nothing was found
            return productVersionFact;
        }

        static private PyxNet.Service.ResourceInstanceFact GetProductVersionFromWebAuthority(Pyxis.Utilities.ProductSpecification product)
        {
            try
            {
                String manifestEntry = s_webClient.DownloadString("http://www.pyxisinnovation.com/api/products/" + product.Name);
                var manifestVersionsFromJson = Pyxis.Utilities.JsonTool<System.Collections.Generic.List<ProductVersion>>.FromJson(manifestEntry);
                ProductVersion foundManifest;
                if (product.Version == null)
                {
                    // get the current version if no version is specified
                    foundManifest = (from manifest in manifestVersionsFromJson
                                     where manifest.Current
                                     select manifest)
                                     .First();
                }
                else
                {
                    foundManifest = (from manifest in manifestVersionsFromJson
                                     where manifest.Version == product.Version
                                     select manifest)
                                     .First();
                }
                return new PyxNet.Service.ResourceInstanceFact(new Pyxis.Utilities.ManifestEntry("PYXIS." + product.Name + "." + foundManifest.Version + ".Manifest",
                                                                                                 ".",
                                                                                                 foundManifest.Checksum,
                                                                                                 foundManifest.Size));
            }
            catch // failed to deserialize downloaded string
            {
                System.Diagnostics.Trace.WriteLine("Failed to get current manifest of " + product + "from web authority.");
                return null;
            }
        }

        static internal PyxNet.Service.ResourceInstanceFact GetProductVersionFromRegistry(Pyxis.Utilities.ProductSpecification product)
        {
            PyxNet.FileTransfer.ManifestManager manager = new PyxNet.FileTransfer.ManifestManager();
            if (product.Version == null)
            {
                return manager.GetApplicationCurrentVersionManifest(product.Name);
            }
            else
            {
                return manager.GetApplicationSpecificVersionManifest(product);
            }
        }
    }

    [System.Runtime.Serialization.DataContract]
    internal class ProductVersion
    {
        [System.Runtime.Serialization.DataMember(Name = "version")]
        internal string Version { get; set; }
        [System.Runtime.Serialization.DataMember(Name = "published")]
        internal bool Published { get; set; }
        [System.Runtime.Serialization.DataMember(Name = "current")]
        internal bool Current { get; set; }
        [System.Runtime.Serialization.DataMember(Name = "checksum")]
        internal string Checksum { get; set; }
        [System.Runtime.Serialization.DataMember(Name = "size")]
        internal int Size { get; set; }
        [System.Runtime.Serialization.DataMember(Name = "comments")]
        internal string Comments { get; set; }
    }
}
