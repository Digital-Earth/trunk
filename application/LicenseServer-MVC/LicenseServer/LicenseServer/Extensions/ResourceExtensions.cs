using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Web;
using AutoMapper;
using LicenseServer.Models.Mongo;
using Resource = Pyxis.Contract.Publishing.Resource;  

namespace LicenseServer.Extensions
{
    public static class ResourceExtensions
    {
        static ResourceExtensions()
        {
            Mapper.CreateMap<Pyxis.Contract.Publishing.Pipeline, GeoSource>();
        }

        public static GeoSource ToGeoSource(this Pyxis.Contract.Publishing.Pipeline pipeline)
        {
            return Mapper.Map<GeoSource>(pipeline);
        }

        public static IQueryable<Resource> FormatResources(this IQueryable<Resource> result, ResultFormat format)
        {
            if (format == ResultFormat.Basic)
            {
                // Make new resource and reset version and updated time
                return result.Select(r => new Resource(r) { Version = r.Version, Metadata = new Pyxis.Contract.Publishing.Metadata(r.Metadata) { Updated = r.Metadata.Updated } });
            }
            else if (format == ResultFormat.View)
            {
                return result.AsEnumerable().Select(r =>
                {
                    if (r.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource || r.Type == Pyxis.Contract.Publishing.ResourceType.Map || r.Type == Pyxis.Contract.Publishing.ResourceType.Pipeline)
                    {
                        var pipeline = r as Pyxis.Contract.Publishing.Pipeline;
                        pipeline.Definition = null;
                        return pipeline;
                    }
                    else
                    {
                        return r;
                    }
                }).AsQueryable();
            }
            return result;
        }

        public static string FromCsuuid(this Guid Id)
        {
            var idString = Id.ToString();
            var stringBuilder = new StringBuilder(32, 32);
            stringBuilder.Append(idString, 6, 2)
                .Append(idString, 4, 2)
                .Append(idString, 2, 2)
                .Append(idString, 0, 2)
                .Append(idString, 11, 2)
                .Append(idString, 9, 2)
                .Append(idString, 16, 2)
                .Append(idString, 14, 2)
                .Append(idString, 19, 4)
                .Append(idString, 24, 12);
            return stringBuilder.ToString();
        }
    }
}