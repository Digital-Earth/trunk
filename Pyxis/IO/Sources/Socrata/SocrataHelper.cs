using System;
using System.Collections.Generic;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Sources.Socrata
{
    internal static class SocrataHelper
    {
        public class SocrataColumnMetadata
        {
            public long id { get; set; }
            public string name { get; set; }
            public string fieldName { get; set; }
            public string dataTypeName { get; set; }
            public int position { get; set; }
        }

        public class SocrataMetadata
        {
            public string id { get; set; }
            public string name { get; set; }
            public string description { get; set; }
            public string displayType { get; set; }
            public string viewType { get; set; }
            public string[] childViews { get; set; }
            public Dictionary<string, dynamic> metadata { get; set; }
            public List<SocrataColumnMetadata> columns { get; set; }
            public double? rowsUpdatedAt { get; set; }
        }

        public class SocrataDiscoveryAPI
        {
            public int resultSetSize { get; set; }
            public List<DiscoveryAPICountBy> results { get; set; }
        }

        public class DiscoveryAPICountBy
        {
            public string domain { get; set; }
            public int count { get; set; }
        }

        public static readonly DateTime UnixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

        public static DateTime FromUnixTime(double? rowsUpdatedAt)
        {
            if (!rowsUpdatedAt.HasValue)
            {
                return DateTime.MinValue;
            }

            return UnixEpoch.AddSeconds(rowsUpdatedAt.Value);
        }

        public enum FieldType
        {
            Number,
            String,
            Url,
            Location
        }

        public static FieldType ParseDataTypeName(string dataTypeName)
        {
            switch (dataTypeName.ToLower())
            {
                case "number":
                case "money":
                case "double":
                    return FieldType.Number;

                case "line":
                case "location":
                case "multiLine":
                case "multiPoint":
                case "multiPolygon":
                case "point":
                case "polygon":
                    return FieldType.Location;

                case "url":
                    return FieldType.Url;

                case "floating timestamp":
                case "checkbox":
                case "text":
                default:
                    return FieldType.String;
            }
        }

        public static PipelineSpecification CreateSpecification(SocrataMetadata columnsMetadata)
        {
            var spec = new PipelineSpecification
            {
                OutputType = PipelineSpecification.PipelineOutputType.Feature,
                Fields = new List<PipelineSpecification.FieldSpecification>()
            };

            foreach (var column in columnsMetadata.columns)
            {
                var field = new PipelineSpecification.FieldSpecification
                {
                    Metadata = new SimpleMetadata
                    {
                        Name = column.name
                    },
                    Name = column.fieldName,
                    FieldType = PipelineSpecification.FieldType.String
                };

                switch (ParseDataTypeName(column.dataTypeName))
                {
                    case FieldType.Location:
                        continue;

                    case FieldType.Number:
                        field.FieldType = PipelineSpecification.FieldType.Number;
                        break;
                }

                spec.Fields.Add(field);
            }
            return spec;
        }
    }
}