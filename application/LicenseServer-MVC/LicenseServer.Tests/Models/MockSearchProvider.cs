using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;
using LicenseServer.Models.Mongo;
using LicenseServer.Models.Mongo.Interface;

namespace LicenseServer.Tests.Models
{
    /// <summary>
    /// Mock ISearchProvider for unit tests of the Controllers to use.
    /// </summary>
    public class MockSearchProvider : ISearchProvider
    {
        private class MockAuthorizedUser : IAuthorizedUserWithResources
        {
            public MockAuthorizedUser(IAuthorizedUserWithResources authorizedUser)
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

        private MockAuthorizedUser m_mockAuthorizedUser;
        public void SetAuthorizedUser(IAuthorizedUserWithResources authorizedUser)
        {
            m_mockAuthorizedUser = new MockAuthorizedUser(authorizedUser);
        }

        public IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int top)
        {
            return new List<ExternalData>().AsQueryable();
        }

        public IQueryable<string> SuggestTerms(string search, List<ResourceType> types)
        {
            return new List<string>{search}.AsQueryable();
        }

        public IQueryable<string> SuggestCompletions(string search, List<ResourceType> types)
        {
            return new List<string> { search }.AsQueryable();
        }

        public IQueryable<string> SuggestMatches(string search, List<ResourceType> types)
        {
            return new List<string> { search }.AsQueryable();
        }
    }
}
