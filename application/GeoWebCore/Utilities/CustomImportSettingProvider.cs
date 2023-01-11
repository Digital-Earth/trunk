using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using GeoWebCore.Models;
using Pyxis.IO.Import;
using Pyxis.IO.Import.GeoTagging;
using GeoWebCore.Services;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Custom ImportSettingProvider to allow GeoWebCore to report back on missing information
    /// </summary>
    public class CustomImportSettingProvider : ImportSettingProvider
    {
        private bool m_missingSrs;
        private List<ImportDataSetRequest.GeoTagRequest> m_geoTagOptions;
        private List<ImportDataSetRequest.GeoTagBasedOnSettings> m_additionalGeoTagReferences;

        /// <summary>
        /// Default vector resolution to use
        /// </summary>
        public const int DefaultVectorResolution = 24;

        /// <summary>
        /// Create CustomImportSettingProvider based on the given information on the ImportDataSetRequest.            
        /// 
        /// the CustomImportSettingProvider will fail the import if more information was required.
        /// GetFoundedGeoTagOptions() and WasSpatialReferenceSystemMissing() can be used to figure out
        /// what information was requested.
        /// </summary>
        /// <param name="request">ImportDataSetRequest to use for setup</param>
        public CustomImportSettingProvider(ImportDataSetRequest request)
        {
            if (request.SRS != null)
            {
                this.SRS(request.SRS);
            }
            else
            {
                Register(typeof(SRSImportSetting), (args) =>
                {
                    m_missingSrs = true;

                    var taskSource = new TaskCompletionSource<IImportSetting>();
                    taskSource.SetException(new Exception("No SRS specified"));
                    return taskSource.Task;
                });
            }

            var foundGeoTagMethod = false;

            if (request.GeoTag != null)
            {
                if (request.GeoTag.LatLon != null)
                {
                    foundGeoTagMethod = true;
                    Register(typeof(GeoTagImportSetting), new GeoTagImportSetting()
                    {
                        Method = new GeoTagByLatLonPoint()
                        {
                            LatitudeFieldName = request.GeoTag.LatLon.LatitudeFieldName,
                            LongitudeFieldName = request.GeoTag.LatLon.LongitudeFieldName,
                            Resolution = request.GeoTag.LatLon.Resolution ?? DefaultVectorResolution,
                            ReferenceSystem = request.GeoTag.LatLon.ReferenceSystem ?? request.SRS
                        }
                    });
                }
                if (request.GeoTag.BasedOn != null)
                {
                    foundGeoTagMethod = true;
                    Register(typeof(GeoTagImportSetting), new GeoTagImportSetting()
                    {
                        Method = new GeoTagByFeatureCollectionLookup(GeoSourceInitializer.Engine)
                        {
                            RecordCollectionFieldName = request.GeoTag.BasedOn.TargetFieldName,
                            ReferenceFieldName = request.GeoTag.BasedOn.ReferenceFieldName,
                            ReferenceGeoSource = GeoSourceInitializer.GetGeoSource(request.GeoTag.BasedOn.ReferenceGeoSource.Id)
                        }
                    });
                }
            }

            if (!foundGeoTagMethod)
            {
                Register(typeof(GeoTagImportSetting), (args) =>
                {
                    var references = request.GeoTagReferences ?? new List<ImportDataSetRequest.GeoTagBasedOnSettings>();
                    if (m_additionalGeoTagReferences != null)
                    {
                        var references1 = references;
                        references = references.Concat(
                            m_additionalGeoTagReferences.Where(reference => !references1.Any(other =>
                            {
                                return other.ReferenceGeoSource.Id == reference.ReferenceGeoSource.Id &&
                                       other.ReferenceFieldName == reference.ReferenceFieldName;
                            }))
                        ).ToList();
                    }

                    var finder = new GeoTagMethodFinder(args as ProvideGeoTagImportSettingArgs, references);

                    m_geoTagOptions = finder.FindGeoTagOptions();

                    //popuplate geosouce metadata for client to see
                    if (m_geoTagOptions != null)
                    {
                        m_geoTagOptions.ForEach((option) =>
                        {
                            if (option.BasedOn != null)
                            {
                                option.BasedOn.ReferenceGeoSourceMetadata =
                                    GeoSourceInitializer.GetGeoSource(option.BasedOn.ReferenceGeoSource.Id).Metadata;
                            }
                        });
                    }

                    var taskSource = new TaskCompletionSource<IImportSetting>();
                    taskSource.SetException(new Exception("No geo tag method specified"));
                    return taskSource.Task;
                });
            }
        }

        /// <summary>
        /// Return a list of possible GetPossibleGetTagRequests if GeoTagOptions was requested while importing the dataset.
        /// </summary>
        /// <returns>List of ImportDataSetRequest.GeoTagRequest or null.</returns>
        public List<ImportDataSetRequest.GeoTagRequest> GetPossibleGetTagRequests()
        {
            return m_geoTagOptions;
        }

        /// <summary>
        /// Returns true if SRS was requested and missing while importing the dataset.
        /// </summary>
        /// <returns>True if SRS was requested while importing the dataset.</returns>
        public bool WasSpatialReferenceSystemMissing()
        {
            return m_missingSrs;
        }

        /// <summary>
        /// Add a geo-tag reference to be used for default geo-tagging
        /// </summary>
        /// <param name="geoTagBasedOnSettings">geo tag based on reference details</param>
        public void AddGeoTagReference(ImportDataSetRequest.GeoTagBasedOnSettings geoTagBasedOnSettings)
        {
            if (m_additionalGeoTagReferences == null)
            {
                m_additionalGeoTagReferences = new List<ImportDataSetRequest.GeoTagBasedOnSettings>();
            }
            m_additionalGeoTagReferences.Add(geoTagBasedOnSettings);
        }
    }
}
