using System;
using System.Collections.Generic;
using System.Linq;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.DataSet
{
    /// <summary>
    /// Data wrapper around a resource (local or external) that represents a GeoSource that can be imported
    /// </summary>
    public class DataSetModel
    {
        /// <summary>
        /// Type of the resource
        /// </summary>
        public string Type { get; set; }

        /// <summary>
        /// The path to the resource
        /// </summary>
        public string Uri { get; set; }

        /// <summary>
        /// Resource Metadata
        /// </summary>
        public SimpleMetadata Metadata { get; set; }

        /// <summary>
        /// The layer name
        /// </summary>
        public string Layer { get; set; }

        /// <summary>
        /// Fields within the layer.
        /// </summary>
        public List<string> Fields { get; set; }


        /// <summary>
        /// Named constructor
        /// </summary>
        public static DataSetModel Create(IDataSetCatalogEntry dataSet)
        {
            if (dataSet == null)
            {
                // Instead of returning a null, return an empty object
                // (because passing a null object to JS asynchronously has issues)
                return new DataSetModel();
            }
            var model = new DataSetModel
            {
                Type = ResourceType.DataSet.ToString(),
                Uri = dataSet.Uri,
                Metadata = dataSet.Metadata,
                Layer = dataSet.Layer,
                Fields = new List<string>()
            };
            // Note: the Fields property may throw a NotImplemented exception
            try
            {
                model.Fields.AddRange(dataSet.Fields);
            }
            catch (Exception)
            {
                // Ignore
            }
            return model;
        }

        public override string ToString()
        {
            // Note: Fields.Aggregate() throws InvalidOperationException if Fields is empty
            var fields = Fields != null ? Fields.Any() ? Fields.Aggregate((x, y) => x + "," + y) : null : null;
            return Uri + ":" + Layer + ":" + fields;
        }
    }
}
