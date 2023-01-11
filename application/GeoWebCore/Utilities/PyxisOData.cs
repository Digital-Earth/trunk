using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text.RegularExpressions;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using GeoWebCore.Services.Cache;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// PyxisOData mimic OData protocol to allow clients perform filtering and transformation on data
    /// using REST API.
    /// 
    /// PyxisOData provide a simplified constructor to extract all supported parameters from a http request.
    /// </summary>
    internal class PyxisOData
    {
        /// <summary>
        /// $skip=number - skip the first #number of records, can be used with $top or without
        /// </summary>
        public int Skip { get; set; }

        /// <summary>
        /// $top=number - return only the first #number of records, can be used with $skip or without
        /// </summary>
        public int Top { get; set; }

        public static int MaximumTop { get { return 1000; } }

        /// <summary>
        /// $select=field[,field] - return only the give fields
        /// </summary>
        public List<string> Select { get; set; }

        /// <summary>
        /// $intersects=pyx-icos-index|geo-json-geometry - return only features that intersects the given geometry
        /// </summary>
        public IGeometry Intersects { get; set; }

        /// <summary>
        /// $aggregate - return results in geo-packets format. by default we return results in flat mode
        /// </summary>
        public bool Aggregate { get; set; }

        /// <summary>
        /// $geometries - include geojson geometries into the results
        /// </summary>
        public bool IncludeGeometries { get; set; }

        static Regex PyxIcosIndexString = new Regex(@"^([1-9]|1[0-2]|[A-T])(-\d*)?$");


        public bool CompleteEnumeration
        {
            get { return IncludeGeometries && !Aggregate && Intersects == null && Select == null; }
        }

        /// <summary>
        /// Build a PyxisOData object for a given request
        /// </summary>
        /// <param name="request">HttpRequestMessage request object</param>
        /// <returns>PyxisOData parsed from request object</returns>
        public static PyxisOData FromRequest(HttpRequestMessage request)
        {
            var odataModel = new PyxisOData
            {
                Skip = 0,
                Top = MaximumTop,
                Select = null,
                Intersects = null,
                Aggregate = false,
                IncludeGeometries = false
            };

            foreach (var keyValue in request.GetQueryNameValuePairs())
            {
                switch (keyValue.Key)
                {
                    case "$top":
                        int top;
                        if (int.TryParse(keyValue.Value, out top))
                        {
                            odataModel.Top = Math.Min(top,MaximumTop);
                        }
                        break;
                    case "$skip":
                        int skip;
                        if (int.TryParse(keyValue.Value, out skip))
                        {
                            odataModel.Skip = skip;
                        }
                        break;
                    case "$aggregate":
                        odataModel.Aggregate = keyValue.Value != "0" && keyValue.Value != "false";
                        break;

                    case "$intersects":
                        if (PyxIcosIndexString.IsMatch(keyValue.Value))
                        {
                            odataModel.Intersects = new CellGeometry(new PYXIcosIndex(keyValue.Value));
                        }
                        else
                        {
                            odataModel.Intersects = GeometryCacheSingleton.Get(keyValue.Value);    
                        }
                        break;

                    case "$select":
                        odataModel.Select = keyValue.Value.Split(',').ToList();
                        break;

                    case "$geometries":
                        odataModel.IncludeGeometries = true;
                        break;
                }
            }

            return odataModel;
        }

        /// <summary>
        /// Apply OData select on a pyxlib IFeature_SPtr
        /// </summary>
        /// <param name="pyxFeature">IFeature_SPtr - input pyxlib feature </param>
        /// <returns>filtered GeoJson.Feature object</returns>
        public Feature Apply(IFeature_SPtr pyxFeature)
        {
            var fields = new Dictionary<string, object>();

            var fieldNames = Select;
            if (Select == null || Select.Count == 0)
            {
                fieldNames = pyxFeature.getDefinition().FieldDefinitions.Select(x => x.getName()).ToList();
            }

            foreach (var name in fieldNames)
            {
                var value = pyxFeature.getFieldValueByName(name);
                fields[name] = value.ToDotNetObject();
            }

            Geometry geometry = null;

            if (IncludeGeometries)
            {
                geometry = Geometry.FromPYXGeometry(pyxFeature.getGeometry());
            }

            return new Feature(pyxFeature.getID(), geometry, fields);
        }

        /// <summary>
        /// Apply OData select on a pyxlib IFeatureGroup_SPtr.
        /// 
        /// for a feature group, for each selected field we return the Min,Max,Average(for numeric fields) 
        /// generated for that feature group
        /// </summary>
        /// <param name="pyxFeatureGroup">IFeatureGroup_SPtr - input pyxlib feature group</param>
        /// <returns>filtered GeoJson.Feature object</returns>
        public Feature Apply(IFeatureGroup_SPtr pyxFeatureGroup)
        {
            var fields = new Dictionary<string, object>();

            var fieldNames = Select;
            if (Select == null || Select.Count == 0)
            {
                fieldNames = pyxFeatureGroup.getFeatureDefinition().FieldDefinitions.Select(x => x.getName()).ToList();
            }

            foreach (var name in fieldNames)
            {
                var fieldIndex = pyxFeatureGroup.getDefinition().getFieldIndex(name);
                if (fieldIndex == -1)
                {
                    continue;
                }

                var histogram = pyxFeatureGroup.getFieldHistogram(fieldIndex);

                var boundaries = histogram.getBoundaries();

                var stats = new Dictionary<string, object>();
                stats["Min"] = boundaries.min.ToDotNetObject();
                stats["Max"] = boundaries.max.ToDotNetObject();

                if (boundaries.min.isNumeric())
                {
                    stats["Average"] = histogram.getAverage().ToDotNetObject();
                }

                fields[name] = stats;
            }

            return new Feature(pyxFeatureGroup.getID(), null, fields);
        }
    }
}