using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;
using Nest;

namespace LicenseServer.Extensions
{
    public static class SearchDescriptorExtensions
    {
        /// <summary>
        /// Create an ExternalData text search.
        /// </summary>
        /// <param name="descriptor">The search descriptor.</param>
        /// <param name="search">The search string.</param>
        /// <param name="bbox">The bounding box to filter results, if not null.</param>
        /// <returns>A search descriptor describing the search.</returns>
        public static SearchDescriptor<ExternalData> TextSearch(this SearchDescriptor<ExternalData> descriptor, string search, Envelope bbox)
        {
            return descriptor
                .Query(q => q
                    .Bool(b => b
                        .Must(m => m
                            .MultiMatch(mm =>
                                mm.Fields(mf => mf
                                    .Field("name", 2.0)
                                    .Field("description", 1.0))
                                    .Query(search)))
                        .Filter(f => CreateQueryFilter(bbox))));
        }

        /// <summary>
        /// Create an ExternalData spatial search.
        /// </summary>
        /// <param name="descriptor">The search descriptor.</param>
        /// <param name="search">The search string to query, if not null.</param>
        /// <param name="bbox">The bounding box to filter results, if not null.</param>
        /// <param name="center">Point of interest around which to search.</param>
        /// <returns>A search descriptor describing the search.</returns>
        public static SearchDescriptor<ExternalData> SpatialSearch(this SearchDescriptor<ExternalData> descriptor, string search, Envelope bbox, List<double> center)
        {
            return descriptor
                .Query(q => q
                    .FunctionScore(fs => fs
                        .Query(sq => sq
                            .Bool(b => b
                                .Must(m =>
                                {
                                    if (search != null)
                                    {
                                        return m
                                            .MultiMatch(mm =>
                                                mm.Fields(mf => mf
                                                    .Field("name", 2.0)
                                                    .Field("description", 1.0))
                                                    .Query(search));
                                    }
                                    return m.MatchAll();
                                })
                                .Filter(f => CreateQueryFilter(bbox))
                            ))
                        // Use function scoring to prefer spatially near data sets when search is specified
                        // data sets without a center have a score of 1 by default, so a weight > 1 allows near data sets to 
                        // have a score greater than 1.  E.g. a weight of 2 would give scores greater than 1 for data sets
                        // within scale+offset distance of center.
                        .Functions(f => f
                            .ExponentialGeoLocation(e => e.Field("center")
                                .Origin(String.Join(",", center.Select(p => p.ToString(CultureInfo.InvariantCulture)).ToList()))
                                .Offset(SpatialSearchSettings.Offset)
                                .Scale(SpatialSearchSettings.Scale)
                                .Filter(ff => ff.Exists(ex => ex.Field("center")))
                                .Weight(100.0)))));
        }

        /// <summary>
        /// Create an ExternalData spatial filter search.
        /// </summary>
        /// <param name="descriptor">The search descriptor.</param>
        /// <param name="bbox">The bounding box to filter results.</param>
        /// <returns>A search descriptor describing the search.</returns>
        public static SearchDescriptor<ExternalData> SpatialFilter(this SearchDescriptor<ExternalData> descriptor, Envelope bbox)
        {
            return descriptor
                .Query(q => q
                    .Bool(b => b
                        .Must(m => m
                            .MatchAll())
                        .Filter(ff => CreateQueryFilter(bbox))));
        }

        /// <summary>
        /// Create a paged search.
        /// </summary>
        /// <param name="descriptor">The search descriptor.</param>
        /// <param name="skip">Number of results to skip.</param>
        /// <param name="top">Number of results to return.</param>
        /// <returns>A search descriptor describing the search.</returns>
        public static SearchDescriptor<ExternalData> PagedSearch(this SearchDescriptor<ExternalData> descriptor, int skip, int top)
        {
            return descriptor
                .Skip(skip)
                .Take(top);
        }

        private static QueryContainer CreateQueryFilter(Envelope bbox)
        {
            QueryContainer filter;
            var statusFilter = Query<ExternalData>.Term("services.status", OgcServiceStatus.Working.ToString());
            if (bbox != null)
            {
                var bboxFilter = Query<ExternalData>.GeoBoundingBox(g => g.Field("center").BoundingBox(bbox.Coordinates[0][1], bbox.Coordinates[0][0], bbox.Coordinates[1][1], bbox.Coordinates[1][0]));
                filter = Query<ExternalData>
                    .Bool(b => b
                        .Must(statusFilter, bboxFilter));
            }
            else
            {
                filter = statusFilter;
            }
            return filter;
        }

        private static class SpatialSearchSettings
        {
            /// <summary>
            /// Gets the offset around a point where spatial relevance is maximum before decaying
            /// </summary>
            public static string Offset { get { return "1km"; } }

            /// <summary>
            /// Gets the scale specifying the distance at which spatial relevance has decayed to half the maximum
            /// </summary>
            public static string Scale { get { return "1000km"; } }
        }
    }
}