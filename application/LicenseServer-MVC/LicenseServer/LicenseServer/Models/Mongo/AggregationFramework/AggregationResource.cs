using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo.AggregationFramework
{
    public class AggregationResource
    {
        public Resource Resource { get; set; }
    }

    public class AggregationResource<T> where T : Resource
    {
        public T Resource { get; set; }
    }
}