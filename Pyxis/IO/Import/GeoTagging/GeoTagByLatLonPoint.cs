using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core.IO;

namespace Pyxis.IO.Import.GeoTagging
{
    /// <summary>
    /// Geo tag a record collection using latitude and longitude fields.
    /// </summary>
    public class GeoTagByLatLonPoint : IGeoTagMethod
    {
        /// <summary>
        /// Gets or sets the SpatialReferenceSystem to use when geo tagging the latitude and longitutde fields.
        /// </summary>
        public SpatialReferenceSystem ReferenceSystem { get; set; }

        /// <summary>
        /// Gets or sets the name for latitude field.
        /// </summary>
        public string LatitudeFieldName { get; set;}

        /// <summary>
        /// Gets or sets the name for longitude field.
        /// </summary>
        public string LongitudeFieldName { get; set;}

        /// <summary>
        /// Pyxis Resolution to use. the default resolution is 24.
        /// </summary>
        public int Resolution { get; set; }

        /// <summary>
        /// Constructor sets default resolution.
        /// </summary>
        public GeoTagByLatLonPoint()
        {
            Resolution = 24;
        }

        /// <summary>
        /// Geo tag a record collection using latitude and longitude fields.
        /// </summary>
        /// <param name="recordCollectionProcess">a RecordCollection to geo tag.</param>
        /// <returns>GeoTagged FeatureCollection IProcess object.</returns>
        public IProcess_SPtr GeoTag(IProcess_SPtr recordCollectionProcess)
        {
            var srs = (ReferenceSystem ?? SpatialReferenceSystem.WGS84).CreateSRSProcess();
            
            var pointGeometryProvider = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo("{5EFB4E9F-7D13-4808-A9A4-058AD38A5DC2}")
                .AddInput(0, srs)
                .AddAttribute("latFieldName", LatitudeFieldName)
                .AddAttribute("lonFieldName", LongitudeFieldName)
                .AddAttribute("Resolution", Resolution.ToString())
                );

            var geoTagProcess = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo("{11E2EBE8-71D3-4D9B-B01A-308BFBB76F71}")
                .AddInput(0, recordCollectionProcess)
                .AddInput(1, pointGeometryProvider)
                .BorrowNameAndDescription(recordCollectionProcess)
                );

            var featuresSummary = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeaturesSummary)
                .AddInput(0, geoTagProcess)
                .BorrowNameAndDescription(recordCollectionProcess)
                );

            return featuresSummary;
        }
    }
}
