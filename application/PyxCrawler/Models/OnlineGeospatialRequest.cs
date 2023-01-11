using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace PyxCrawler.Models
{
    public enum OnlineGeospatialRequestState
    {
        Pending,
        Crawled,
        Archive,
        Removed
    }

    public class OnlineGeospatialRequest
    {
        public int Id { get; set; }

        public Uri Uri { get; set; }

        public int Count { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public OnlineGeospatialRequestState State { get; set; }

        public List<string> Protocols { get; set; }
    }
}