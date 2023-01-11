using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Web;
using Elasticsearch.Net;
using LicenseServer.Extensions;
using LicenseServer.Models.Mongo.Interface;
using Nest;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    /// <summary>
    /// Elasticsearch implementation of ISearchProvider.
    /// </summary>
    public class ElasticsearchProvider : ISearchProvider
    {
        private static readonly ElasticClient s_elasticsearchClient;

        static ElasticsearchProvider()
        {
            string[] connections = new string[Properties.Settings.Default.ElasticSearchUrls.Count];
            Properties.Settings.Default.ElasticSearchUrls.CopyTo(connections, 0);
            var connectionPool = new StaticConnectionPool(connections.Select(c => new Uri(c)));
            var settings = new ConnectionSettings(connectionPool)           
            // .DisableDirectStreaming() // uncomment to view raw queries in responses
            .DefaultIndex(Properties.Settings.Default.ElasticSearchIndex)
            .MapDefaultTypeIndices(d => d.Add(typeof (ExternalData), Properties.Settings.Default.ExternalDataIndex))
            .MapDefaultTypeNames(d => d.Add(typeof (Resource), "Resources"))
            .MapDefaultTypeNames(d => d.Add(typeof (ExternalData), "Ogc"));
            s_elasticsearchClient = new ElasticClient(settings);
        }

        private class AuthorizedUser : IAuthorizedUserWithResources
        {
            public AuthorizedUser(IAuthorizedUserWithResources authorizedUser)
            {
                Id = authorizedUser.Id;
                IsAdmin = authorizedUser.IsAdmin;
                IsPyxisAdmin = authorizedUser.IsPyxisAdmin;
                Groups = new List<Guid>(authorizedUser.Groups);
                Galleries = new List<Guid>(authorizedUser.Galleries);
            }

            public Guid Id { get; private set; }
            public bool IsAdmin { get; private set; }
            public bool IsPyxisAdmin { get; private set; }
            public List<Guid> Groups { get; private set; }
            public List<Guid> Galleries { get; private set; }
        }

        private AuthorizedUser m_authorizedUser;
        public void SetAuthorizedUser(IAuthorizedUserWithResources authorizedUser)
        {
            m_authorizedUser = new AuthorizedUser(authorizedUser);
        }

        /// <summary>
        /// Search external data sets known to the search provider.
        /// </summary>
        /// <param name="search">Keywords to include in the search, if not null.</param> 
        /// <param name="center">Point of interest around which to search, if not null.</param>
        /// <param name="bbox">The bounding box to filter results, if not null.</param>
        /// <param name="skip">The number of search results to skip.</param>
        /// <param name="top">The number of search results to be returned.</param>
        /// <returns>External data sets found in the search.</returns>
        public IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int top)
        {
            var searchDescriptor = new SearchDescriptor<ExternalData>();
            if (search != null && center == null)
            {
                searchDescriptor = searchDescriptor.TextSearch(search, bbox);
            }
            else if (center != null)
            {
                searchDescriptor = searchDescriptor.SpatialSearch(search, bbox, center);
            }
            else if (bbox != null)
            {
                searchDescriptor = searchDescriptor.SpatialFilter(bbox);
            }

            searchDescriptor = searchDescriptor.PagedSearch(skip, top);
            
            var response = s_elasticsearchClient.Search<ExternalData>(searchDescriptor);
            return response.Documents
                .AsQueryable();
        }

        /// <summary>
        /// Suggest individual search terms given a search string.
        /// Suggested terms are distinct from those provided in <paramref name="search"/>.
        /// Suggestions sorted in descending order of relevance.
        /// </summary>
        /// <param name="search">Term(s) to make suggestions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested individual terms based on <paramref name="search"/>.</returns>
        public IQueryable<string> SuggestTerms(string search, List<ResourceType> types)
        {
            // Suggest matching tags
            var response = s_elasticsearchClient.Suggest<Resource>(s => s
                .GlobalText(search)
                .Term("Tags-suggest", t => t.Analyzer("english").Field("Metadata.Tags"))); // NEST lower-cases field names if the non-string overload is used
            return response.Suggestions
                .SelectMany(s => s.Value)
                .SelectMany(v => v.Options)
                .Select(o => o.Text)
                .AsQueryable();
        }

        /// <summary>
        /// Suggest individual search completions given a search string.
        /// Suggestions sorted in descending order of relevance.
        /// </summary>
        /// <param name="search">String to suggest completions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested completions based on <paramref name="search"/>.</returns>
        public IQueryable<string> SuggestCompletions(string search, List<ResourceType> types)
        {
            // suggest completions based on tags
            var response = s_elasticsearchClient.Suggest<Resource>(s => s
                .Index(Properties.Settings.Default.ElasticSearchIndex) // insist on using only default index
                .GlobalText(search)
                .Completion("Tags-completion", t => t.Field("Metadata.Tags.completion"))); // NEST lower-cases field names if the non-string overload is used
            return response.Suggestions
                .SelectMany(s => s.Value)
                .SelectMany(v => v.Options)
                .Select(o => o.Text)
                .Distinct(StringComparer.CurrentCultureIgnoreCase)
                .AsQueryable();
        }

        /// <summary>
        /// Suggest matching search phrases given a search string.
        /// Suggestions sorted in descending order of relevance.
        /// </summary>
        /// <param name="search">String to suggest completions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested completions based on <paramref name="search"/>.</returns>
        public IQueryable<string> SuggestMatches(string search, List<ResourceType> types)
        {
            QueryContainer filter;
            var visibilityFilter = Query<Resource>.Term("Metadata.Visibility", "Public");
            var stateFilter = Query<Resource>.Term("State", "Removed");
            if (m_authorizedUser != null)
            {
                QueryContainer privateFilter;
                if (m_authorizedUser.IsPyxisAdmin)
                {
                    privateFilter = Query<Resource>.Term("Metadata.Visibility", "Private");
                }
                else
                {
                    privateFilter = Query<Resource>.Bool(b => b
                        .Must(m => m.Term("Metadata.User._id", m_authorizedUser.Id.FromCsuuid()), 
                            m => m.Term("Metadata.Visibility", "Private")));
                }
                visibilityFilter = Query<Resource>.Bool(b => b.Should(visibilityFilter, privateFilter)); 
            }
            if (types != null && types.Any())
            {
                var typeFilter = Query<Resource>.Terms(t => t.Field("Type").Terms(types.Select(ty => ty.ToString())));
                filter = Query<Resource>.Bool(b => b.Must(typeFilter, visibilityFilter) // filter order matters (use most exclusive first)
                    .MustNot(stateFilter)); 
            }
            else
            {
                filter = Query<Resource>.Bool(b => b.Must(visibilityFilter)
                    .MustNot(stateFilter)); 
            }
            var response = s_elasticsearchClient.Search<Resource>(s => s
                .Query(q => q.Bool(b => 
                    b.Must(m => 
                        m.MultiMatch(mm => mm.Query(search)
                    .Fields(f => f
                        .Field("Metadata.Name", 5)
                        .Field("Metadata.Tags", 3)
                        .Field("Metadata.Description", 1))
                    .Fuzziness(Fuzziness.EditDistance(1))))
                    .Filter(filter)))
                .Fields(f => f.Field("Metadata.Name")));
            return response.Hits
                .Select(h => h.Fields.Value<string>(new Field{ Name = "Metadata.Name" }))
                .Distinct(StringComparer.CurrentCultureIgnoreCase)
                .AsQueryable();
        }
    }
}