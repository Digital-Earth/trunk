using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using AutoMapper;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Extensions
{
    public static class MetadataExtensions
    {
        static MetadataExtensions()
        {
            Mapper.CreateMap<ImmutableMetadata,Metadata>()
                .ForMember(x => x.Comments, o => o.Ignore())
                .ForMember(x => x.Ratings, o => o.Ignore())
                .ForAllMembers(x => x.Condition(p => !p.IsSourceValueNull));

            Mapper.CreateMap<ImmutableMetadata, Metadata>().ForAllMembers(x => x.Condition(p => !p.IsSourceValueNull));
        }

        public static Pyxis.Contract.Publishing.Metadata ToMetadata(this ImmutableMetadata immutableMetadata)
        {
            return Mapper.Map<Metadata>(immutableMetadata);
        }

        public static ImmutableMetadata ToImmutable(this Metadata metadata)
        {
            return Mapper.Map<ImmutableMetadata>(metadata);
        }
    }
}