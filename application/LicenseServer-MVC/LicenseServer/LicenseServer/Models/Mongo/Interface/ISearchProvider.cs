using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LicenseServer.Models.Mongo.Interface;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    /// <summary>
    /// Provides search related functionality given a search string and filters.
    /// </summary>
    public interface ISearchProvider : IAuthorizableWithResources
    {
        /// <summary>
        /// Search external data sets known to the search provider.
        /// </summary>
        /// <param name="search">Keywords to include in the search.</param>
        /// <param name="center">Point of interest around which to search.</param>
        /// <param name="bbox">The bounding box to filter results.</param>
        /// <param name="skip">The number of search results to skip.</param>
        /// <param name="top">The number of search results to be returned.</param>
        /// <returns>External data sets found in the search.</returns>
        IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int top);

        /// <summary>
        /// Suggest individual search terms given a search string.
        /// </summary>
        /// <param name="search">Term(s) to make suggestions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested individual terms based on <paramref name="search"/>.</returns>
        IQueryable<string> SuggestTerms(string search, List<ResourceType> types);
        
        /// <summary>
        /// Suggest individual search completions given a search string.
        /// </summary>
        /// <param name="search">String to suggest completions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested completions based on <paramref name="search"/>.</returns>
        IQueryable<string> SuggestCompletions(string search, List<ResourceType> types);

        /// <summary>
        /// Suggest matching search phrases given a search string.
        /// </summary>
        /// <param name="search">String to suggest completions of.</param>
        /// <param name="types">Preferred ResourceTypes to make suggestions from.</param>
        /// <returns>Suggested completions based on <paramref name="search"/>.</returns>
        IQueryable<string> SuggestMatches(string search, List<ResourceType> types);
    }
}
