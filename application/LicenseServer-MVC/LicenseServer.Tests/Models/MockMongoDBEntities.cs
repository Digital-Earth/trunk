using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LicenseServer.App_Utilities;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Models.Mongo.Interface;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.GeoWebStreamService;
using Gallery = LicenseServer.Models.Mongo.Gallery;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;
using Resource = Pyxis.Contract.Publishing.Resource;
using User = LicenseServer.Models.Mongo.User;

namespace LicenseServer.Tests.Models
{
    internal class MockMongoDBEntities : IMongoDBEntities
    {
        private HashSet<Resource> m_resources = new HashSet<Resource>();
        public HashSet<Resource> Resources { get { return m_resources; } }
        private IEnumerable<Resource> VisibleResources
        {
            get
            {
                return Resources.Where(r => r.Metadata.Visibility == VisibilityType.Public 
                    || (m_authorizedUser != null 
                        && (m_authorizedUser.IsPyxisAdmin 
                            || r.Metadata.User.Id == m_authorizedUser.Id 
                            || (m_authorizedUser.IsAdmin && r is Gallery && (r as Gallery).Admin.Id == m_authorizedUser.Id)
                            || (m_authorizedUser.Groups.Intersect(r.Metadata.Providers.Where(p => p.Type == ResourceType.Gallery).Select(g => g.Id)).Any())
                            || (r is Gallery && m_authorizedUser.Groups.Intersect((r as Gallery).Groups.Select(g => g.Id)).Any()))));
            }
        }
        private HashSet<Pyxis.Contract.Publishing.Activity> m_activities = new HashSet<Pyxis.Contract.Publishing.Activity>();
        public HashSet<Pyxis.Contract.Publishing.Activity> Activities { get { return m_activities; } }

        private HashSet<KeyValuePair<string, object>> m_parameters = new HashSet<KeyValuePair<string, object>>();
        public HashSet<KeyValuePair<string, object>> Parameters { get { return m_parameters; } }

        private HashSet<Pyxis.Contract.Publishing.Gwss> m_gwsses = new HashSet<Pyxis.Contract.Publishing.Gwss>();
        public HashSet<Pyxis.Contract.Publishing.Gwss> Gwsses { get { return m_gwsses; } }

        private HashSet<PyxisIdentityUser> m_identities = new HashSet<PyxisIdentityUser>();
        public HashSet<PyxisIdentityUser> Identities { get { return m_identities; } }

        private ISearchProvider m_searchProvider = new MockSearchProvider();
        public ISearchProvider SearchProvider { get { return m_searchProvider; } }

        private class AuthorizedUser : IAuthorizedUserWithResources
        {
            public AuthorizedUser(IAuthorizedUser authorizedUser)
            {
                Id = authorizedUser.Id;
                IsAdmin = authorizedUser.IsAdmin;
                IsPyxisAdmin = authorizedUser.IsPyxisAdmin;
                Groups = new List<Guid>();
                Galleries = new List<Guid>();
            }
            public AuthorizedUser(IAuthorizedUser authorizedUser, List<Guid> groups)
            {
                Id = authorizedUser.Id;
                IsAdmin = authorizedUser.IsAdmin;
                IsPyxisAdmin = authorizedUser.IsPyxisAdmin;
                Groups = new List<Guid>(groups);
                Galleries = new List<Guid>();
            }
            public AuthorizedUser(IAuthorizedUser authorizedUser, List<Guid> groups, List<Guid> galleries)
            {
                // roll in resources in order of permissions granted: 1st set user to get Groups, 2nd set Groups to get Galleries, 3rd set complete authorizedUser
                Id = authorizedUser.Id;
                IsAdmin = authorizedUser.IsAdmin;
                IsPyxisAdmin = authorizedUser.IsPyxisAdmin;
                Groups = new List<Guid>(groups);
                Galleries = new List<Guid>(galleries);
            }
            public Guid Id { get; private set; }
            public bool IsAdmin { get; private set; }
            public bool IsPyxisAdmin { get; private set; }
            public List<Guid> Groups { get; private set; }
            public List<Guid> Galleries { get; private set; }
        }
        private AuthorizedUser m_authorizedUser = null;
        public void SetAuthorizedUser(IAuthorizedUser authorizedUser)
        {
            // roll in permissions: 1st set user to get Groups, 2nd set Groups to get Galleries, 3rd set complete authorizedUser
            m_authorizedUser = new AuthorizedUser(authorizedUser);
            var groups = GetResourceById<User>(m_authorizedUser.Id).Groups.Select(g => g.Id).ToList();
            m_authorizedUser = new AuthorizedUser(authorizedUser, groups);
            var galleries = GetResourcesByGroupIds<Gallery>(m_authorizedUser.Groups).Select(g => g.Id).ToList();
            m_authorizedUser = new AuthorizedUser(m_authorizedUser, groups, galleries);
            SearchProvider.SetAuthorizedUser(m_authorizedUser);
        }

        #region Resources

        public IQueryable<Resource> GetResources()
        {
            return VisibleResources.AsQueryable();
        }

        public IQueryable<Resource> GetResources(List<Pyxis.Contract.Publishing.ResourceType> types)
        {
            return VisibleResources.Where(r => types.Contains(r.Type)).AsQueryable();
        }

        public IQueryable<T> GetResources<T>() where T : Resource
        {
            return VisibleResources.Where(r => r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByIds(List<Guid> ids)
        {
            return VisibleResources.Where(r => ids.Contains(r.Id)).AsQueryable();
        }

        public IQueryable<T> GetResourcesByIds<T>(List<Guid> ids) where T : Resource
        {
            return VisibleResources.Where(r => ids.Contains(r.Id)).Cast<T>().AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByName(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive)
        {
            return Resources.Where(r => 
                options == MongoStringCompareOptions.CaseSensitive 
                ? r.Metadata.Name == name 
                : string.Compare(name, r.Metadata.Name, StringComparison.InvariantCultureIgnoreCase) == 0)
                .AsQueryable();
        }

        public IQueryable<T> GetResourcesByName<T>(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive) where T : Resource
        {
            return GetResourcesByName(name).Where(r => r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable<T>();
        }

        public IQueryable<Resource> GetResourcesByName(Guid userId, string name)
        {
            return GetResourcesByName(name).Where(r => r.Metadata.User.Id == userId).AsQueryable();
        }

        public IQueryable<T> GetResourcesByName<T>(Guid userId, string name) where T : Resource
        {
            return GetResourcesByName<T>(name).Where(r => r.Metadata.User.Id == userId).AsQueryable();
        }

        // The behavior of this function uses contains while mongo uses complete word matching up to the word stem
        public IQueryable<Resource> SearchResources(string search)
        {
            return VisibleResources.Where(r => SearchSingle(search, r)).AsQueryable();
        }

        public IQueryable<Resource> SearchResources(string search, List<Pyxis.Contract.Publishing.ResourceType> types)
        {
            return VisibleResources.Where(r => SearchSingle(search, r) && types.Contains(r.Type)).AsQueryable();
        }

        public IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int top)
        {
            return SearchProvider.SearchExternal(search, center, bbox, skip, top);
        }

        public IQueryable<string> SuggestTerms(string search, List<Pyxis.Contract.Publishing.ResourceType> types)
        {
            return SearchProvider.SuggestTerms(search, types);
        }

        public IQueryable<string> SuggestCompletions(string search, List<Pyxis.Contract.Publishing.ResourceType> types)
        {
            return SearchProvider.SuggestCompletions(search, types);
        }

        public IQueryable<string> SuggestMatches(string search, List<Pyxis.Contract.Publishing.ResourceType> types)
        {
            return SearchProvider.SuggestMatches(search, types);
        }

        private static bool SearchSingle(string search, Resource r)
        {
            var matched = (r.Metadata.Description != null && r.Metadata.Description.IndexOf(search, StringComparison.OrdinalIgnoreCase) != -1) ||
                (r.Metadata.Name != null && r.Metadata.Name.IndexOf(search, StringComparison.OrdinalIgnoreCase) != -1) ||
                (r.Metadata.Category != null && r.Metadata.Category.IndexOf(search, StringComparison.OrdinalIgnoreCase) != -1);
            if (!matched && r.Metadata.Tags != null)
            {
                matched = r.Metadata.Tags.Where(t => t.IndexOf(search, StringComparison.OrdinalIgnoreCase) != -1).FirstOrDefault() != null;
            }
            return matched;
        }

        public IQueryable<T> SearchResources<T>(string search) where T : Resource
        {
            return SearchResources(search).Where(r => r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable();
        }

        public IQueryable<Resource> SearchGallery(Guid id, string search)
        {
            var gallery = GetResourceById<Gallery>(id);
            var resourceIds = gallery.Resources.Select(r => r.ResourceId).ToList();
            return SearchResources(search).Where(r => resourceIds.Contains(r.Id)).AsQueryable();
        }

        public Resource GetResourceById(Guid id)
        {
            return VisibleResources.Where(r => r.Id == id).FirstOrDefault();
        }

        public T GetResourceById<T>(Guid id) where T : Resource
        {
            return VisibleResources.Where(r => r.Id == id && r.Type.ToString() == GetTypeName<T>()).Cast<T>().FirstOrDefault();
        }

        public IQueryable<Resource> GetResourceVersionsById(Guid id)
        {
            return VisibleResources.Where(r => r.Id == id).AsQueryable();
        }

        public IQueryable<T> GetResourceVersionsById<T>(Guid id) where T : Resource
        {
            return VisibleResources.Where(r => r.Id == id && r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable<T>();
        }

        public Resource GetResourceByIdAndVersion(Guid id, Guid version)
        {
            return VisibleResources.FirstOrDefault(r => r.Id == id && r.Version == version);
        }

        public T GetResourceByIdAndVersion<T>(Guid id, Guid version) where T : Resource
        {
            return Resources.Where(r => r.Id == id && r.Version == version && r.Type.ToString() == GetTypeName<T>()).Cast<T>().FirstOrDefault();
        }

        public IQueryable<Resource> GetResourcesByUserId(Guid id)
        {
            return VisibleResources.Where(r => r.Metadata.User.Id == id).AsQueryable();
        }

        public IQueryable<T> GetResourcesByUserId<T>(Guid id) where T : Resource
        {
            return VisibleResources.Where(r => r.Metadata.User.Id == id && r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable<T>();
        }

        public IQueryable<Resource> GetResourcesByGroupIds(List<Guid> groupIds)
        {
            return VisibleResources.Where(r => r is Gallery).Cast<Gallery>().Where(g => g.Groups.Select(gr => gr.Id).Intersect(groupIds).Any()).AsQueryable();
        }

        public IQueryable<T> GetResourcesByGroupIds<T>(List<Guid> groupIds) where T : Resource
        {
            return VisibleResources.Where(r => r is Gallery && ((Gallery)r).Groups.Select(gr => gr.Id).Intersect(groupIds).Any() && r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable();
        }

        public long GetStorageByUserId(Guid id)
        {
            return Resources.Where(r => r.Type.ToString() == Pyxis.Contract.Publishing.ResourceType.GeoSource.ToString() && r.Metadata.User.Id == id).Cast<MultiDomainGeoSource>().Sum(g => g.DataSize.Value);
        }

        public Resource GetPipelineByProcRef(string procRef)
        {
            return Resources.FirstOrDefault(r => (r.Type == Pyxis.Contract.Publishing.ResourceType.Pipeline || r.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource 
                || r.Type == Pyxis.Contract.Publishing.ResourceType.Map) && (r as Pyxis.Contract.Publishing.Pipeline).ProcRef == procRef);
        }

        public Resource GetPipelineByIdAndVersion(Guid id, Guid version)
        {
            return Resources.FirstOrDefault(r => (r.Type == Pyxis.Contract.Publishing.ResourceType.Pipeline || r.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource
                || r.Type == Pyxis.Contract.Publishing.ResourceType.Map) && (r as Pyxis.Contract.Publishing.Pipeline).Id == id && (r as Pyxis.Contract.Publishing.Pipeline).Version == version);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetResourceGroupings(string field)
        {
            EnsureValidGroupingField(field);
            return GenerateGroupings(field, VisibleResources.AsQueryable());
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetResourceGroupings<T>(string field) where T : Resource
        {
            EnsureValidGroupingField(field);
            var qualifyingResources = VisibleResources.Where(r => r.Type.ToString() == GetTypeName<T>()).AsQueryable();
            return GenerateGroupings(field, qualifyingResources);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetResourceGroupings(string search, string field)
        {
            EnsureValidGroupingField(field);
            var searchResults = SearchResources(search);
            return GenerateGroupings(field, searchResults);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetResourceGroupings(string search, List<Pyxis.Contract.Publishing.ResourceType> types, string field)
        {
            EnsureValidGroupingField(field);
            var searchResults = SearchResources(search, types);
            return GenerateGroupings(field, searchResults);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetResourceGroupings<T>(string field, string search) where T : Resource
        {
            EnsureValidGroupingField(field);
            var searchResults = SearchResources<T>(search);
            return GenerateGroupings(field, searchResults);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetGalleryGroupings(Guid id, string field)
        {
            var gallery = GetResourceById<Gallery>(id);
            var resourceIds = gallery.Resources.Select(r => r.ResourceId).ToList();
            EnsureValidGroupingField(field);
            var consideredResources = VisibleResources.Where(r => resourceIds.Contains(r.Id)).AsQueryable();
            return GenerateGroupings(field, consideredResources);
        }

        public IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GetGalleryGroupings(Guid id, string search, string field)
        {
            var gallery = GetResourceById<Gallery>(id);
            var resourceIds = gallery.Resources.Select(r => r.ResourceId).ToList();
            EnsureValidGroupingField(field);
            var searchResults = SearchGallery(id, search);
            return GenerateGroupings(field, searchResults);
        }

        public IQueryable<Resource> GetUpdates(List<Guid> ids, DateTime lastUpdated)
        {
            return VisibleResources.Where(r => r.Metadata.Updated > lastUpdated).AsQueryable();
        }

        public IQueryable<T> GetUpdates<T>(List<Guid> ids, DateTime lastUpdated) where T : Resource
        {
            return VisibleResources.Where(r => r.Metadata.Updated > lastUpdated && r.Type.ToString() == GetTypeName<T>()).Cast<T>().AsQueryable();
        }

        public void InsertResource<T>(T resource) where T : Resource
        {
            var existingResource = Resources.FirstOrDefault(r => r.Id == resource.Id && r.Type.ToString() == GetTypeName<T>());
            if (existingResource == null)
            {
                Resources.Add(resource);
            }
            else
            {
                throw new DataLayerException("Conflict", DataLayerExceptionType.Conflict);
            }
        }

        public void UpdateResource<T>(Guid id, Guid version, T resource) where T : Resource
        {
            var existingResource = Resources.FirstOrDefault(r => r.Id == id && r.Version == version && r.Type.ToString() == GetTypeName<T>());
            if (existingResource == null)
            {
                throw new DataLayerException("Resource not found - No " + GetTypeName<T>() + " with given ID", DataLayerExceptionType.NotFound);
            }

            if (typeof(T) == typeof(MultiDomainGeoSource))
            {
                var existingGeoSource = existingResource as MultiDomainGeoSource;
                var geoSource = resource as MultiDomainGeoSource;
                var newGeoSource = new MultiDomainGeoSource(existingGeoSource);
                if (geoSource.DataSize != null) { newGeoSource.DataSize = geoSource.DataSize; }
                UpdateResource<MultiDomainGeoSource>(newGeoSource, geoSource);
                newGeoSource.Metadata.Created = DateTime.UtcNow;
                newGeoSource.Metadata.Updated = DateTime.UtcNow;

                Resources.RemoveWhere(r => r.Id == existingGeoSource.Id);
                Resources.Add(newGeoSource);
            }
            else if (typeof(T) == typeof(User))
            {
                var existingUser = existingResource as User;
                var user = resource as User;
                var newUser = new User(existingUser);
                if (user.Subscribed != null) { newUser.Subscribed = new List<Guid>(user.Subscribed); }
                UpdateResource<User>(newUser, user);
                newUser.Metadata.Updated = DateTime.UtcNow;

                Resources.RemoveWhere(r => r.Id == existingUser.Id);
                Resources.Add(newUser);
            }
            else if (typeof(T) == typeof(License))
            {
                var existingLicense = existingResource as License;
                var license = resource as License;
                var newLicense = new License(license);

                if (license.ExportOptions != null)
                {
                    if (license.ExportOptions.Formats != null) { newLicense.ExportOptions.Formats = new List<string>(license.ExportOptions.Formats); }
                    if (license.ExportOptions.AllowReport != null) { newLicense.ExportOptions.AllowReport = license.ExportOptions.AllowReport; }
                }
                UpdateResource<License>(newLicense, license);
                newLicense.Metadata.Updated = DateTime.UtcNow;

                Resources.RemoveWhere(r => r.Id == existingLicense.Id);
                Resources.Add(newLicense);
            }
            else if (typeof(T) == typeof(Gallery))
            {
                var existingGallery = existingResource as Gallery;
                var gallery = resource as Gallery;
                var newGallery = new Gallery(gallery);

                if (gallery.Resources != null) { newGallery.Resources = gallery.Resources; }
                UpdateResource<Gallery>(newGallery, gallery);
                newGallery.Metadata.Updated = DateTime.UtcNow;

                Resources.RemoveWhere(r => r.Id == existingGallery.Id);
                Resources.Add(newGallery);
            }
            else
            {
                throw new NotImplementedException("Resource type " + GetTypeName<T>() + " update method not implemented");
            }
        }

        private static string GetTypeName<T>()
        {
            if (typeof(T).Name == "MultiDomainGeoSource")
            {
                return "GeoSource";
            }
            return typeof(T).Name;
        }

        private static void EnsureValidGroupingField(string field)
        {
            if (field != "Tags" || field != "Category")
            {
                throw new DataLayerException("Invalid field grouping requested", DataLayerExceptionType.BadRequest);
            }
        }

        private static IQueryable<Pyxis.Contract.Publishing.ResourceGrouping> GenerateGroupings(string field, IQueryable<Resource> resources)
        {
            if (field == "Tags")
            {
                return resources.SelectMany(r => r.Metadata.Tags).GroupBy(r => r)
                    .Select(g => new Pyxis.Contract.Publishing.ResourceGrouping { Name = g.Key, Count = g.Count() }).AsQueryable();
            }
            else
            {
                return resources.GroupBy(r => r.Metadata.Category)
                    .Select(g => new Pyxis.Contract.Publishing.ResourceGrouping { Name = g.Key, Count = g.Count() }).AsQueryable();
            }
        }

        public void RemoveResource<T>(Guid id) where T : Resource
        {
            var typeName = GetTypeName<T>();
            int numRemoved = Resources.RemoveWhere(r => r.Id == id && r.Type.ToString() == typeName);
            if (numRemoved == 0)
            {
                throw new DataLayerException("Not found - No " + typeName + " with given ID", DataLayerExceptionType.NotFound);
            }
        }

        private static void UpdateResource<T>(T existingResource, T resource) where T : Resource
        {
            if (resource.Licenses != null) { existingResource.Licenses = new List<Pyxis.Contract.Publishing.LicenseReference>(resource.Licenses); }
            UpdateMetadata<T>(existingResource, resource);
        }
            
        private static void UpdateMetadata<T>(T existingResource, T resource) where T : Resource
        {
            if (resource.Metadata == null)
            {
                return;
            }
            if (resource.Metadata.Category != null) { existingResource.Metadata.Category = resource.Metadata.Category; }
            if (resource.Metadata.Description != null) { existingResource.Metadata.Description = resource.Metadata.Description; }
            if (resource.Metadata.ExternalUrls != null) { existingResource.Metadata.ExternalUrls = new List<Pyxis.Contract.Publishing.ExternalUrl>(resource.Metadata.ExternalUrls); }
            if (resource.Metadata.Name != null) { existingResource.Metadata.Name = resource.Metadata.Name; }
            if (resource.Metadata.Tags != null) { existingResource.Metadata.Tags = new List<string>(resource.Metadata.Tags); }
            if (resource.Metadata.User != null) { existingResource.Metadata.User = resource.Metadata.User; }
            if (resource.Metadata.Providers != null) { existingResource.Metadata.Providers = resource.Metadata.Providers; }
        }

        #endregion Resources

        #region Activities

        public void InsertComment(Pyxis.Contract.Publishing.Comment comment)
        {
            var resource = Resources.Where(r => r.Id == comment.ResourceId).FirstOrDefault();
            if (resource == null)
            {
                throw new DataLayerException("Not found - No resource with given ID and Version", DataLayerExceptionType.NotFound);
            }

            Activities.Add(comment);
            InsertCommentInHierarchy(resource.Metadata.Comments, comment);
        }

        private static void InsertCommentInHierarchy(LinkedList<Pyxis.Contract.Publishing.AggregateComment> comments, Pyxis.Contract.Publishing.Comment comment)
        {
            if (!comments.Insert(comment))
            {
                throw new DataLayerException("Not found - No Comment with given ID to reply to", DataLayerExceptionType.NotFound);
            }
        }

        public void InsertRating(Pyxis.Contract.Publishing.Rating rating)
        {
            var resource = Resources.FirstOrDefault(r => r.Id == rating.ResourceId);
            if (resource == null)
            {
                throw new DataLayerException("Invalid resource specified", DataLayerExceptionType.NotFound);
            }
            if (rating.Value > 0)
            {
                resource.Metadata.Ratings.Likes++;
            }
            else
            {
                resource.Metadata.Ratings.Dislikes++;
            }
            Activities.Add(rating);
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreements(DateTime from, DateTime until)
        {
            return Activities.Where(a => a.Type == Pyxis.Contract.Publishing.ActivityType.Agreement).Cast<Pyxis.Contract.Publishing.Agreement>().AsQueryable();
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreementsByUser(Guid userId, DateTime from, DateTime until)
        {
            return GetAgreements(from, until).Where(a => a.User.Id == userId);
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreementsByLicenseId(Guid licenseId, DateTime from, DateTime until)
        {
            return GetAgreements(from, until).Where(a => a.ResourceId == licenseId);
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreementsByLicenseVersionedId(Guid licenseId, Guid licenseVersion, DateTime from, DateTime until)
        {
            return GetAgreements(from, until).Where(a => a.ResourceId == licenseId && a.LicenseVersion == licenseVersion);
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreementsByUserAndLicenseId(Guid userId, Guid licenseId, DateTime from, DateTime until)
        {
            return GetAgreements(from, until).Where(a => a.User.Id == userId && a.ResourceId == licenseId);
        }

        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetAgreementsByUserAndLicenseVersionedId(Guid userId, Guid licenseId, Guid licenseVersion, DateTime from, DateTime until)
        {
            return GetAgreements(from, until).Where(a => a.User.Id == userId && a.ResourceId == licenseId && a.LicenseVersion == licenseVersion);
        }
        
        public IQueryable<Pyxis.Contract.Publishing.Agreement> GetActiveAgreementsByUserAndLicenseReferences(Guid userId, List<Pyxis.Contract.Publishing.LicenseReference> licenseReferences)
        {
            var licenseIds = licenseReferences.Select(l => l.Id).ToList();
            return GetAgreementsByUser(userId, DateTime.MinValue, DateTime.MaxValue).Where(a => a.Decision == Pyxis.Contract.Publishing.DecisionType.Agree
                && licenseIds.Contains(a.ResourceId) && a.Expiration > DateTime.UtcNow);
        }

        public Pyxis.Contract.Publishing.LicensedAccess GetLicensedAccessToResource(Guid userId, Guid resourceId)
        {
            var resource = GetResourceById(resourceId);
            if (resource == null)
            {
                throw new DataLayerException("No resource found with the specified Resource Id", DataLayerExceptionType.NotFound);
            }
            var licensedAccess = new Pyxis.Contract.Publishing.LicensedAccess { HasAccess = true, UsingLicenses = new List<Pyxis.Contract.Publishing.VersionedLicenseReference>() };
            if (resource.Licenses.Any())
            {
                var agreements = GetAgreements(DateTime.MinValue, DateTime.MaxValue).Where(a => a.Type == Pyxis.Contract.Publishing.ActivityType.Agreement && a.User.Id == userId 
                    && a.Decision == Pyxis.Contract.Publishing.DecisionType.Agree && resource.Licenses.Where(l => l.Id == a.ResourceId).Any() && a.Expiration > DateTime.UtcNow);
                if (!agreements.Any())
                {
                    licensedAccess.HasAccess = false;
                }
                else
                {
                    var agreement = agreements.Where(a => resource.Licenses.Any(l => l.LicenseType == Pyxis.Contract.Publishing.LicenseType.Full && l.Id == a.ResourceId)).FirstOrDefault();
                    if (agreement == null)
                    {
                        agreement = agreements.Where(a => resource.Licenses.Any(l => l.LicenseType == Pyxis.Contract.Publishing.LicenseType.Trial && l.Id == a.ResourceId)).FirstOrDefault();
                    }
                    if (agreement == null)
                    {
                        licensedAccess.HasAccess = false;
                    }
                    licensedAccess.UsingLicenses.Add(new Pyxis.Contract.Publishing.VersionedLicenseReference(agreement.ResourceId, agreement.LicenseType, agreement.Limitations, agreement.LicenseVersion));
                }
            }
            return licensedAccess;
        }

        public void InsertAgreement(Pyxis.Contract.Publishing.Agreement agreement)
        {
            var license = GetResourceByIdAndVersion<License>(agreement.ResourceId, agreement.LicenseVersion);
            if (license == null)
            {
                throw new DataLayerException("Invalid license specified", DataLayerExceptionType.NotFound);
            }

            var existingAgreement = GetAgreementsByUserAndLicenseVersionedId(agreement.User.Id, agreement.ResourceId, agreement.LicenseVersion, DateTime.MinValue, DateTime.MaxValue)
                .OrderByDescending(a => a.Created).FirstOrDefault();
            if (existingAgreement == null || agreement.Decision != existingAgreement.Decision)
            {
                Activities.Add(agreement);
            }
        }

        #endregion Activities

        #region Pipelines

        public IQueryable<PipelineTaggedInfoNoDefinitionDTO> GetPipelines()
        {
            return Resources.Where(r => r.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource).Cast<MultiDomainGeoSource>()
                .DistinctBy(g => g.ProcRef).Select(g => PipelineTaggedInfoNoDefinitionFactory.Create(g)).AsQueryable();
        }

        public PipelineTaggedInfoDTO GetPipeline(string procRef)
        {
            var geoSource = GetPipelineByProcRef(procRef) as Pyxis.Contract.Publishing.GeoSource;
            return PipelineTaggedInfoFactory.Create(geoSource);
        }

        public PublishedPipelineDetailsDTO GetPipelineDetails(string procRef)
        {
            return new PublishedPipelineDetailsDTO
            {
                PublishingServerStatus = Gwsses.Where(g => g.Status.PipelinesStatuses.Any(p => p.ProcRef == procRef)).Select(g =>
                    {
                        var operation = g.Status.OperationsStatuses.FirstOrDefault(o =>
                        {
                            string tmpProcRef = null;
                            var found = (o.Operation.Parameters.TryGetValue("ProcRef", out tmpProcRef) && tmpProcRef == procRef);
                            return found;
                        });
                        return new PipelineServerStatusDTO
                        {
                            NodeId = g.Id,
                            ProcRef = procRef,
                            Status = g.Status.PipelinesStatuses.First(p => p.ProcRef == procRef).StatusCode.ToString(),
                            OperationStatus = operation
                        };
                    })
                    .ToList()
            };
        }

        #endregion Pipelines

        #region Parameters

        public IQueryable<KeyValuePair<string, object>> GetParameters()
        {
            return Parameters.AsQueryable().Select(p => new KeyValuePair<string, object>(p.Key, p.Value));
        }

        public object GetParameter(string key)
        {
            var pair = Parameters.FirstOrDefault(p => p.Key == key);
            return pair.Equals(default(KeyValuePair<string, object>)) ? null : pair.Value;
        }

        public void UpdateParameter(KeyValuePair<string, object> pair)
        {
            Parameters.RemoveWhere(p => p.Key == pair.Key);
            Parameters.Add(pair);
        }

        #endregion Parameters

        #region Gwsses

        public IQueryable<Pyxis.Contract.Publishing.Gwss> GetGwsses()
        {
            return Gwsses.AsQueryable();
        }

        public IQueryable<Pyxis.Contract.Publishing.Gwss> GetGwssesStatuses()
        {
            return GetGwsses();
        }

        public Pyxis.Contract.Publishing.Gwss GetGwssById(Guid id)
        {
            return Gwsses.FirstOrDefault(g => g.Id == id);
        }

        public Pyxis.Contract.Publishing.Gwss GetGwssStatusById(Guid id)
        {
            return GetGwssById(id);
        }

        public void UpdateGwssStatus(Guid id, Pyxis.Contract.Publishing.GwssStatus gwssStatus)
        {
            var gwss = Gwsses.FirstOrDefault(g => g.Id == id);
            if (gwss != null)
            {
                gwss.Status = gwssStatus;
                gwss.LastHeard = DateTime.UtcNow;
            }
            else
            {
                gwss = new Pyxis.Contract.Publishing.Gwss(id, gwssStatus, new Pyxis.Contract.Publishing.LsStatus());
                Gwsses.Add(gwss);
            }
        }

        public void RemoveGwss(Guid id)
        {
            Gwsses.RemoveWhere(g => g.Id == id);
        }

        #endregion Gwsses

        #region Identities

        public IQueryable<PyxisIdentityAffiliate> GetAffiliates(string email)
        {
            var atDomain = email.Substring(email.IndexOf('@'));
            if (m_authorizedUser == null || !m_authorizedUser.IsAdmin || String.IsNullOrWhiteSpace(atDomain))
            {
                return Enumerable.Empty<PyxisIdentityAffiliate>().AsQueryable();
            }
            return Identities.AsQueryable().Where(d => d.Email.EndsWith(atDomain) && d.ResourceId != null).Cast<PyxisIdentityAffiliate>();
        }

        #endregion Identities

        internal static MockMongoDBEntities SetupContext()
        {
            var context = new MockMongoDBEntities();

            var procRef = "{00000000-0000-0000-0000-000000000000}[0]";
            Resource resource = new MultiDomainGeoSource(GeoSourceId, new List<Pyxis.Contract.Publishing.LicenseReference> { LicenseReference },
                new Pyxis.Contract.Publishing.Metadata("Test GeoSource", "This is a test geosource", UserInformation, new List<Pyxis.Contract.Publishing.Provider>(),
                    "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<Pyxis.Contract.Publishing.ExternalUrl>(), Pyxis.Contract.Publishing.VisibilityType.Public,
                    new LinkedList<Pyxis.Contract.Publishing.AggregateComment>(), new Pyxis.Contract.Publishing.AggregateRatings()),
                Version,
                procRef, "<...>", new List<Pyxis.Contract.Publishing.ResourceReference>(),
                Pyxis.Contract.Publishing.PipelineDefinitionState.Active,
                1234, new List<Pyxis.Contract.Publishing.ResourceReference>(), new List<Guid>(), new List<Guid>());
            context.Resources.Add(resource);
            resource = new License(LicenseId, new List<Pyxis.Contract.Publishing.LicenseReference> { LicenseReference },
                new Pyxis.Contract.Publishing.Metadata("Free Use License", "Unrestricted use license", UserInformation, new List<Pyxis.Contract.Publishing.Provider>(), 
                    "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<Pyxis.Contract.Publishing.ExternalUrl>(), Pyxis.Contract.Publishing.VisibilityType.Public,
                    new LinkedList<Pyxis.Contract.Publishing.AggregateComment>(), new Pyxis.Contract.Publishing.AggregateRatings()),
                Version, new Pyxis.Contract.Publishing.LicenseTerms { Text = "Allow everything" }, Pyxis.Contract.Publishing.LicenseType.Full, Pyxis.Contract.Publishing.PublishingType.Open, 
                new Pyxis.Contract.Publishing.LicenseTrialLimitations(), new Pyxis.Contract.Publishing.LicenseExportOptions());
            context.Resources.Add(resource);
            resource = new User(UserInformation.Id, new List<Pyxis.Contract.Publishing.LicenseReference> { LicenseReference },
                new Pyxis.Contract.Publishing.Metadata("Pyxis", "GeoWeb Builder", UserInformation, new List<Pyxis.Contract.Publishing.Provider>(), "Test", new List<string>(), new List<string>(),
                       DateTime.MinValue, DateTime.MinValue, null, new List<Pyxis.Contract.Publishing.ExternalUrl>(), Pyxis.Contract.Publishing.VisibilityType.Public,
                       new LinkedList<Pyxis.Contract.Publishing.AggregateComment>(), new Pyxis.Contract.Publishing.AggregateRatings()),
                Version, Pyxis.Contract.Publishing.UserState.Active, Pyxis.Contract.Publishing.UserType.Beta, new List<Guid> { LicenseId }, false, new List<Guid>(), new List<GroupInfo>());
            context.Resources.Add(resource);

            context.Parameters.Add(new KeyValuePair<string, object>(Pyxis.Contract.Publishing.UserType.Beta + "-StorageLimit", 1024 * 1024 * 1024L));
            context.Parameters.Add(new KeyValuePair<string, object>(Pyxis.Contract.Publishing.UserType.Member + "-StorageLimit", 1024 * 1024 * 1024L));
            context.Parameters.Add(new KeyValuePair<string, object>(Pyxis.Contract.Publishing.UserType.Super + "-StorageLimit", 100*1024*1024*1024L));

            var gwss = new Pyxis.Contract.Publishing.Gwss(GwssId,
                new Pyxis.Contract.Publishing.GwssStatus
                {
                    Name = "TestGwss",
                    ServerStatus = new Pyxis.Contract.Publishing.ServerStatus { AvailableDiskSpaceMB = 1000 },
                    PipelinesStatuses = new List<Pyxis.Contract.Publishing.PipelineStatusImpl> { 
                        new Pyxis.Contract.Publishing.PipelineStatusImpl { ProcRef = procRef, StatusCode = PipelineStatusCode.Published } 
                    },
                    OperationsStatuses = new List<Pyxis.Contract.Publishing.OperationStatus>()
                }
                , null);
            context.Gwsses.Add(gwss);
            return context;
        }

        internal static PyxisIdentityUser GenerateUserIdentity()
        {
            var userIdentity = new PyxisIdentityUser { ResourceId = UserInformation.Id, ProfileName = UserInformation.Name };
            userIdentity.Roles.Add(PyxisIdentityRoles.Admin);
            return userIdentity;
        }

        static private Guid s_geoSourceId = new Guid(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
        static public Guid GeoSourceId { get { return s_geoSourceId; } }
        static private Guid s_licenseId = new Guid(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
        static private Guid s_licenseVersion = new Guid(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2);
        static public Guid LicenseId { get { return s_licenseId; } }
        static public Pyxis.Contract.Publishing.LicenseReference LicenseReference
        {
            get
            {
                return new Pyxis.Contract.Publishing.LicenseReference
                    {
                        Id = s_licenseId,
                        LicenseType = Pyxis.Contract.Publishing.LicenseType.Full,
                        Limitations = new Pyxis.Contract.Publishing.LicenseTrialLimitations()
                    };
            }
        }
        static private Pyxis.Contract.Publishing.UserInfo s_userInformation = new Pyxis.Contract.Publishing.UserInfo { Id = new Guid(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2), Name = "TestUser" };
        static public Pyxis.Contract.Publishing.UserInfo UserInformation { get { return s_userInformation; } }
        static private Guid s_gwssId = new Guid(4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4);
        static public Guid GwssId { get { return s_gwssId; } }
        static private Guid s_version = new Guid(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        static public Guid Version { get { return s_version; } }
    }
}
