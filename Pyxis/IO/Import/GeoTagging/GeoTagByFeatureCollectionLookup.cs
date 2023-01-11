using ApplicationUtility;
using Pyxis.Core;

namespace Pyxis.IO.Import.GeoTagging
{
    /// <summary>
    /// Geo tag a record collection using another GeoSource as a lookup table.
    /// </summary>
    public class GeoTagByFeatureCollectionLookup : IGeoTagMethod
    {
        private Engine Engine { get; set; }

        /// <summary>
        /// Reference GeoSource to be used a lookup table.
        /// </summary>
        public Pyxis.Contract.Publishing.GeoSource ReferenceGeoSource { get; set; }

        /// <summary>
        /// Reference field name to be indexed as a lookup field from the reference GeoSource.
        /// </summary>
        public string ReferenceFieldName { get; set; }

        /// <summary>
        /// The field to be used as a lookup on the imported RecordCollection.
        /// </summary>
        public string RecordCollectionFieldName { get; set; }

        /// <summary>
        /// Create a lookup geo tag method.
        /// </summary>
        /// <param name="engine">an Engine object to be used to initialize the ReferenceGeoSource.</param>
        public GeoTagByFeatureCollectionLookup(Engine engine)
        {
            Engine = engine;
        }

        /// <summary>
        /// Geo tag a record collection using reference GeoSource as a lookup table.
        /// </summary>
        /// <param name="recordCollectionProcess">a RecordCollection to geo tag.</param>
        /// <returns>GeoTagged FeatureCollection IProcess object.</returns>
        public IProcess_SPtr GeoTag(IProcess_SPtr recordCollectionProcess)
        {
            var featureCollectonProcess = Engine.GetProcess(ReferenceGeoSource);

            var recordCollection = pyxlib.QueryInterface_IRecordCollection(recordCollectionProcess.getOutput());
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(featureCollectonProcess.getOutput());

            int recordCollectionIndex = recordCollection.getRecordDefinition().getFieldIndex(RecordCollectionFieldName);
            int featureCollectionIndex = featureCollection.getFeatureDefinition().getFieldIndex(ReferenceFieldName);

            var pointGeometryProvider = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo("{BFFF86DE-90F9-4EA6-9BC1-60075FD91E79}")
                .AddInput(0, featureCollectonProcess)
                .AddAttribute("RecordFieldIndices", recordCollectionIndex.ToString())
                .AddAttribute("FeatureFieldsIndices", featureCollectionIndex.ToString())
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
