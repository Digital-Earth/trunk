using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text.RegularExpressions;
using LicenseServer.App_Utilities;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models.Mongo.AggregationFramework;
using LicenseServer.Models.Mongo.Interface;
using MongoDB.Bson;
using MongoDB.Bson.Serialization;
using MongoDB.Driver;
using MongoDB.Driver.Builders;
using MongoDB.Driver.Linq;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class MongoDBEntities : IMongoDBEntities
    {
        public MongoDBEntities()
        {
            db = s_server.GetDatabase("pyxis_licenseserver");
            m_searchProvider = new ElasticsearchProvider();
        }

        // Allow integration testing on a separate db
        public MongoDBEntities(string databaseName)
        {
            db = s_server.GetDatabase(databaseName);
            m_searchProvider = new ElasticsearchProvider();
        }

        private static MongoClient s_client = new MongoClient(ConfigurationManager.ConnectionStrings["pyxis_licenseserverMongo"].ConnectionString);
        private static MongoServer s_server = s_client.GetServer();
        private MongoDatabase db;
        private MongoCollection Resources { get { return db.GetCollection("Resources"); } }
        // store all versions of resources in the archive (previous and current version)
        private MongoCollection ResourcesArchive { get { return db.GetCollection("ResourcesArchive"); } }
        private MongoCollection Activities { get { return db.GetCollection("Activities"); } }
        private MongoCollection<MongoKeyValuePair> Parameters { get { return db.GetCollection<MongoKeyValuePair>("Parameters"); } }
        private MongoCollection Identities { get { return db.GetCollection("AspNetUsers"); } }
        private MongoCollection Gwsses { get { return db.GetCollection<Gwss>("Gwsses"); } }
        private ISearchProvider m_searchProvider;

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

        private AuthorizedUser m_authorizedUser;
        public void SetAuthorizedUser(IAuthorizedUser authorizedUser)
        {
            // roll in resources in order of permissions granted
            // 1st set User to get User.Groups and .Galleries
            m_authorizedUser = new AuthorizedUser(authorizedUser);
            var user = GetResourceById<User>(m_authorizedUser.Id);
            var groups = user.Groups.Select(g => g.Id).ToList();
            var galleries = new List<Guid>(user.Galleries);
            // 2nd set Groups to get Group Galleries
            m_authorizedUser = new AuthorizedUser(authorizedUser, groups);
            if (groups.Any())
            {
                galleries.AddRange(GetResourcesByGroupIds<Gallery>(m_authorizedUser.Groups).Select(g => g.Id).ToList());
            }
            // 3rd set complete authorizedUser
            m_authorizedUser = new AuthorizedUser(m_authorizedUser, groups, galleries);
            m_searchProvider.SetAuthorizedUser(m_authorizedUser);
        }

        #region Resources

        public bool GetIdentityNameAvailability(string name)
        {
            var query = Query<PyxisIdentityUser>.Matches(p => p.UserName, new BsonRegularExpression(new Regex("^" + name + "$", RegexOptions.IgnoreCase)));
            return Identities.FindOneAs<PyxisIdentityUser>(query) == null;
        }

        public bool GetEmailAvailability(string email)
        {
            var query = Query<PyxisIdentityUser>.Matches(p => p.Email, new BsonRegularExpression(new Regex("^" + Regex.Escape(email) + "$", RegexOptions.IgnoreCase)));
            return Identities.FindOneAs<PyxisIdentityUser>(query) == null;
        }

        public IQueryable<Resource> GetResources()
        {
            var query = CreateVisibilityQuery();
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<Resource> GetResources(List<ResourceType> types)
        {
            var query = Query.And(CreateVisibilityQuery(), Query<Resource>.In(r => r.Type, types));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResources<T>() where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery();
            query = Query.And(query, Query<Resource>.EQ(r => r.Type, resourceType));
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByIds(List<Guid> ids)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.In(r => r.Id, ids));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourcesByIds<T>(List<Guid> ids) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.In(r => r.Id, ids), Query<Resource>.EQ(r => r.Type, resourceType));
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByName(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive)
        {
            var query = options == MongoStringCompareOptions.CaseSensitive 
                ? Query<Resource>.EQ(r => r.Metadata.Name, name) 
                : Query<Resource>.Matches(r => r.Metadata.Name, new BsonRegularExpression(new Regex("^" + name + "$", RegexOptions.IgnoreCase)));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourcesByName<T>(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var nameQuery = options == MongoStringCompareOptions.CaseSensitive
                ? Query<Resource>.EQ(r => r.Metadata.Name, name)
                : Query<Resource>.Matches(r => r.Metadata.Name, new BsonRegularExpression(new Regex("^" + name + "$", RegexOptions.IgnoreCase)));
            var query = Query.And(Query<Resource>.EQ(r => r.Type, resourceType), nameQuery);
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByName(Guid userId, string name)
        {
            var query = Query.And(Query<Resource>.EQ(r => r.Metadata.Name, name), Query<Resource>.EQ(r => r.Metadata.User.Id, userId));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourcesByName<T>(Guid userId, string name) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = Query.And(Query<Resource>.EQ(r => r.Type, resourceType), Query<Resource>.EQ(r => r.Metadata.Name, name), Query<Resource>.EQ(r => r.Metadata.User.Id, userId));
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public IQueryable<Resource> SearchResources(string search)
        {
            return SearchResources(search, null);
        }

        public IQueryable<Resource> SearchResources(string search, List<ResourceType> types)
        {
            return SearchKernel(search, types);
        }

        public IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int take)
        {
            return m_searchProvider.SearchExternal(search, center, bbox, skip, take);
        }

        public IQueryable<string> SuggestTerms(string search, List<ResourceType> types)
        {
            return m_searchProvider.SuggestTerms(search, types);
        }

        public IQueryable<string> SuggestCompletions(string search, List<ResourceType> types)
        {
            return m_searchProvider.SuggestCompletions(search, types);
        }

        public IQueryable<string> SuggestMatches(string search, List<ResourceType> types)
        {
            return m_searchProvider.SuggestMatches(search, types);
        }

        public IQueryable<T> SearchResources<T>(string search) where T : Resource
        {
            return SearchKernel<T>(search);
        }

        public IQueryable<Resource> SearchGallery(Guid id, string search)
        {
            var gallery = GetResourceById<Gallery>(id);
            var filter = Query<Resource>.In(r => r.Id, gallery.Resources.Select(resource => resource.ResourceId).ToList());
            return SearchKernel(search, null, filter);
        }

        public Resource GetResourceById(Guid id)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id));
            return Resources.FindAs<Resource>(query).FirstOrDefault<Resource>();
        }

        public T GetResourceById<T>(Guid id) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id), Query<Resource>.EQ(r => r.Type, resourceType));
            return Resources.FindAs<T>(query).FirstOrDefault<T>();
        }

        public IQueryable<Resource> GetResourceVersionsById(Guid id)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id));
            return ResourcesArchive.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourceVersionsById<T>(Guid id) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id), Query<Resource>.EQ(r => r.Type, resourceType));
            return ResourcesArchive.FindAs<T>(query).AsQueryable();
        }

        public Resource GetResourceByIdAndVersion(Guid id, Guid version)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id), Query<Resource>.EQ(r => r.Version, version));
            return ResourcesArchive.FindOneAs<Resource>(query);
        }

        public T GetResourceByIdAndVersion<T>(Guid id, Guid version) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.EQ(r => r.Id, id), Query<Resource>.EQ(r => r.Version, version),
                Query<Resource>.EQ(r => r.Type, resourceType));
            return ResourcesArchive.FindOneAs<T>(query);
        }

        public Resource GetPipelineByProcRef(string procRef)
        {
            var query = Query.And(
                    Query.Or(Query<Resource>.EQ(p => p.Type, ResourceType.Pipeline),
                    Query<Resource>.EQ(p => p.Type, ResourceType.GeoSource),
                    Query<Resource>.EQ(p => p.Type, ResourceType.Map)),
                Query<Pipeline>.EQ(p => p.ProcRef, procRef));
            return Resources.FindOneAs<Resource>(query);
        }

        public Resource GetPipelineByIdAndVersion(Guid id, Guid version)
        {
            var query = Query.And(Query.Or(Query<Resource>.EQ(p => p.Type, ResourceType.Pipeline),
                    Query<Resource>.EQ(p => p.Type, ResourceType.GeoSource),
                    Query<Resource>.EQ(p => p.Type, ResourceType.Map)),
                Query<Pipeline>.EQ(p => p.Id, id), Query<Pipeline>.EQ(p => p.Version, version));
            return Resources.FindOneAs<Resource>(query);
        }

        public IQueryable<ResourceGrouping> GetResourceGroupings(string field)
        {
            return ResourceGroupingAggregation(field);
        }

        public IQueryable<ResourceGrouping> GetResourceGroupings<T>(string field) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            return ResourceGroupingAggregation(field, resourceType);
        }

        public IQueryable<ResourceGrouping> GetResourceGroupings(string search, string field)
        {
            return ResourceGroupingAggregation(field, search);
        }

        public IQueryable<ResourceGrouping> GetResourceGroupings(string search, List<ResourceType> types, string field)
        {
            return ResourceGroupingAggregation(field, types, search, null);
        }

        public IQueryable<ResourceGrouping> GetResourceGroupings<T>(string search, string field) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            return ResourceGroupingAggregation(field, resourceType, search);
        }

        public IQueryable<ResourceGrouping> GetGalleryGroupings(Guid id, string field)
        {
            var gallery = GetResourceById<Gallery>(id);
            return ResourceGroupingAggregation(field, gallery.Resources.Select(r => r.ResourceId).ToList());
        }

        public IQueryable<ResourceGrouping> GetGalleryGroupings(Guid id, string search, string field)
        {
            var gallery = GetResourceById<Gallery>(id);
            return ResourceGroupingAggregation(field, search, gallery.Resources.Select(r => r.ResourceId).ToList());
        }

        public IQueryable<Resource> GetResourcesByUserId(Guid id)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            var permissionQuery = Query.Or(Query<Resource>.EQ(r => r.Metadata.User.Id, id),
                Query<Gallery>.EQ(g => g.Admin.Id, id));
            if (m_authorizedUser != null)
            {
                permissionQuery = Query.Or(permissionQuery,
                    Query<Gallery>.ElemMatch(g => g.Groups, gi => Query.And(
                        Query<GroupPermissionInfo>.EQ(pr => pr.Permission, GroupPermission.Publish),
                        Query<GroupPermissionInfo>.In(pr => pr.Id, m_authorizedUser.Groups))));
            }
            query = Query.And(query, permissionQuery);
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourcesByUserId<T>(Guid id) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            var permissionQuery = Query.Or(Query<T>.EQ(r => r.Metadata.User.Id, id),
                Query<Gallery>.EQ(g => g.Admin.Id, id));
            if (m_authorizedUser != null)
            {
                permissionQuery = Query.Or(permissionQuery,
                    Query<Gallery>.ElemMatch(g => g.Groups, gi => Query.And(
                        Query<GroupPermissionInfo>.EQ(pr => pr.Permission, GroupPermission.Publish),
                        Query<GroupPermissionInfo>.In(pr => pr.Id, m_authorizedUser.Groups))));
            }
            query = Query.And(query,
                Query<T>.EQ(r => r.Type, resourceType),
                permissionQuery);
            var sortOrder = new SortByBuilder<T>()
                .Ascending(r => r.Metadata.Created);
            return Resources.FindAs<T>(query).SetSortOrder(sortOrder).AsQueryable();
        }

        public IQueryable<Resource> GetResourcesByGroupIds(List<Guid> groupIds)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, 
                Query.Not(Query<Gallery>.Size(g => g.Groups, 0)),
                Query<Gallery>.ElemMatch(g => g.Groups, gi => gi.In(group => group.Id, groupIds)));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetResourcesByGroupIds<T>(List<Guid> groupIds) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query,
                Query<T>.EQ(r => r.Type, resourceType),
                Query.Not(Query<Gallery>.Size(g => g.Groups, 0)),
                Query<Gallery>.ElemMatch(g => g.Groups, gi => gi.In(group => group.Id, groupIds)));
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public long GetStorageByUserId(Guid id)
        {
            var matchUserGeoSources = new BsonDocument { { "$match", new BsonDocument { { "Type", ResourceType.GeoSource.ToString() }, { "State", "Active" }, { "Metadata.User._id", id } } } };
            var sumUserGeoSources = new BsonDocument{ { "$group", new BsonDocument{ { "_id", "$Metadata.User._id" }, { "Usage", new BsonDocument{ { "$sum", "$DataSize" } } } } } };

            var operations = new[] { matchUserGeoSources, sumUserGeoSources  };
            var aggregateArgs = new AggregateArgs { Pipeline = operations };
            var result = Resources.Aggregate(aggregateArgs);
            return result.Any() ? result.First().First(x => x.Name == "Usage").Value.AsInt64 : 0L;
        }

        public IQueryable<Resource> GetUpdates(List<Guid> ids, DateTime lastUpdated)
        {
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<Resource>.In(r => r.Id, ids), Query<Resource>.GT(r => r.Metadata.Updated, lastUpdated));
            return Resources.FindAs<Resource>(query).AsQueryable();
        }

        public IQueryable<T> GetUpdates<T>(List<Guid> ids, DateTime lastUpdated) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery(VisibilityQueryOption.IncludeNonDiscoverable);
            query = Query.And(query, Query<T>.EQ(r => r.Type, resourceType), Query<T>.In(r => r.Id, ids), Query<T>.GT(r => r.Metadata.Updated, lastUpdated));
            return Resources.FindAs<T>(query).AsQueryable();
        }

        public void InsertResource<T>(T resource) where T : Resource
        {
            ArchivedInsertResource(resource);
        }

        public void UpdateResource<T>(Guid id, Guid version, T resource) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();

            Resource foundResource;
            foundResource = GetResourceById<T>(id);
            if (foundResource == null)
            {
                throw new DataLayerException("Not found - No " + resourceType.ToString() + " with given ID", DataLayerExceptionType.NotFound);
            }

            if (foundResource.Version != version)
            {
                throw new DataLayerException("Forbidden Update - Attempt to update a previous version of a resource", DataLayerExceptionType.InvalidUpdate);
            }

            if (resource == null)
            {
                throw new DataLayerException("Not Modified - Empty request to modify resource", DataLayerExceptionType.NotModified);
            }

            switch (resourceType)
            {
                case ResourceType.GeoSource:
                    var geoSource = resource as MultiDomainGeoSource;
                    var foundGeoSource = foundResource as MultiDomainGeoSource;
                    var newGeoSource = new MultiDomainGeoSource(foundGeoSource);
                    UpdateGeoSource(geoSource, newGeoSource);
                    ArchivedUpdateResource(newGeoSource);
                    break;

                case ResourceType.Map:
                    var map = resource as Map;
                    var foundMap = foundResource as Map;
                    var newMap = new Map(foundMap);
                    UpdateMap(map, newMap);
                    ArchivedUpdateResource(newMap);
                    break;

                case ResourceType.User:
                    var user = resource as User;
                    var foundUser = foundResource as User;
                    var newUser = new User(foundUser);
                    UpdateUser(user, newUser);
                    ArchivedUpdateResource(newUser);
                    break;

                case ResourceType.Group:
                    var group = resource as Group;
                    var foundGroup = foundResource as Group;
                    var newGroup = new Group(foundGroup);
                    UpdateGroup(group, newGroup);
                    ArchivedUpdateResource(newGroup);
                    break;

                case ResourceType.License:
                    var license = resource as License;
                    var foundLicense = foundResource as License;
                    var newLicense = new License(foundLicense);
                    UpdateLicense(license, newLicense);
                    ArchivedUpdateResource(newLicense);
                    break;

                case ResourceType.Gallery:
                    var gallery = resource as Gallery;
                    var foundGallery = foundResource as Gallery;
                    var newGallery = new Gallery(foundGallery);
                    UpdateGallery(gallery, newGallery);
                    ArchivedUpdateResource(newGallery);
                    break;

                case ResourceType.Product:
                    var product = resource as Product;
                    var foundProduct = foundResource as Product;
                    var newProduct = new Product(foundProduct);
                    UpdateProduct(product, newProduct);
                    ArchivedUpdateResource(newProduct);
                    break;

                default:
                    throw new NotImplementedException("Resource type " + resourceType.ToString() + " update method not implemented");
            }
        }

        public void RemoveResource<T>(Guid id) where T : Resource
        {
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = Query.And(Query<T>.EQ(r => r.Id, id), Query<T>.EQ(r => r.Type, resourceType));
            var result = Resources.Remove(query, WriteConcern.Acknowledged);
            if (result.DocumentsAffected == 0)
            {
                throw new DataLayerException("Not found - No " + resourceType.ToString() + " with given ID", DataLayerExceptionType.NotFound);
            }
            HandleWriteConcernResult(result);
            result = ResourcesArchive.Remove(query);
            HandleWriteConcernResult(result);
        }

        private IMongoQuery CreateVisibilityQuery(VisibilityQueryOption option = VisibilityQueryOption.Default)
        {
            var query = Query<Resource>.EQ(r => r.Metadata.Visibility, VisibilityType.Public);
            if (option == VisibilityQueryOption.IncludeNonDiscoverable)
            {
                query = Query.Or(query, Query<Resource>.EQ(r => r.Metadata.Visibility, VisibilityType.NonDiscoverable));
            }
            if (m_authorizedUser != null)
            {
                if (!m_authorizedUser.IsPyxisAdmin)
                {
                    query = Query.Or(query, 
                        Query<Resource>.EQ(r => r.Metadata.User.Id, m_authorizedUser.Id),
                        Query<Resource>.ElemMatch(r => r.Metadata.Providers, p => Query.And(
                            Query<Provider>.EQ(pr => pr.Type, ResourceType.Gallery), 
                            Query<Provider>.In(pr => pr.Id, m_authorizedUser.Galleries))),
                        Query.And(Query<Resource>.EQ(r => r.Type, ResourceType.Gallery),
                            Query<Gallery>.ElemMatch(r => r.Groups, q => q.In(g => g.Id, m_authorizedUser.Groups))));
                }
                if (m_authorizedUser.IsAdmin)
                {
                    query = Query.Or(query, Query<Gallery>.EQ(r => r.Admin.Id, m_authorizedUser.Id));
                }
                if(m_authorizedUser.IsPyxisAdmin)
                {
                    // Everything is visible
                    query = Query<Resource>.Exists(r => r.Metadata.Visibility);
                }
            }
            return query;
        }


        private IQueryable<Resource> SearchKernel(string search)
        {
            return SearchKernel(search, null, null);
        }

        private IQueryable<Resource> SearchKernel(string search, List<ResourceType> types)
        {
            return SearchKernel(search, types, null);
        }

        private IQueryable<Resource> SearchKernel(string search, List<ResourceType> types, IMongoQuery filter)
        {
            if (search == null)
            {
                throw new ArgumentNullException("search");
            }
            var query = CreateVisibilityQuery();
            if (filter != null)
            {
                query = Query.And(filter, query);
            }
            if (types == null || types.Count == 0)
            {
                query = Query.And(Query.Text(search), Query<Resource>.NotIn(r => r.Type, new[] { ResourceType.Pipeline, ResourceType.File }), query);
            }
            else
            {
                query = Query.And(Query.Text(search), Query<Resource>.In(r => r.Type, types), query);
            }

            return AggregateSort<Resource>(query);
        }

        private IQueryable<T> AggregateSort<T>(IMongoQuery query) where T : Resource
        {
            var operations = new List<BsonDocument>();

            // $match the query
            operations.Add(new BsonDocument {{"$match", query.ToBsonDocument()}});
            // project out ExternalUrls and score
            operations.Add(new BsonDocument {{"$project", new BsonDocument {{"doc", "$$ROOT"}, {"xurls", "$Metadata.ExternalUrls"}, {"score", new BsonDocument {{"$meta", "textScore"}}}}}});
            // unwind ExternalUrls (to find if an image is present)
            operations.Add(new BsonDocument {{"$unwind", new BsonDocument {{"path", "$xurls"}, {"preserveNullAndEmptyArrays", true}}}});
            // project if an image is present
            operations.Add(new BsonDocument
            {
                {
                    "$project", new BsonDocument
                    {
                        {"doc", "$doc"}, {"score", "$score"},
                        {
                            "hasImage", new BsonDocument
                            {
                                {
                                    "$cond", new BsonDocument
                                    {
                                        {"if", new BsonDocument {{"$eq", new BsonArray(new List<string> {"$xurls.Type", "Image"})}}},
                                        {"then", 1},
                                        {"else", 0}
                                    }
                                }
                            }
                        }
                    }
                }
            });
            // group to decide if a doc has an image
            operations.Add(new BsonDocument
            {
                {
                    "$group", new BsonDocument
                    {
                        {"_id", "$doc.Id"},
                        {"doc", new BsonDocument {{"$first", "$doc"}}},
                        {"hasImage", new BsonDocument {{"$max", "$hasImage"}}},
                        {"score", new BsonDocument {{"$first", "$score"}}}
                    }
                }
            });
            // sort by image presence, then text score, then Metadata.Updated
            operations.Add(new BsonDocument
            {
                {
                    "$sort", new BsonDocument
                    {
                        {"hasImage", -1},
                        {"score", -1},
                        {"doc.Metadata.Updated", -1}
                    }
                }
            });
            // project only the Resource
            operations.Add(new BsonDocument {{"$project", new BsonDocument {{"Resource", "$doc"}, {"_id", 0}}}});

            var aggregateArgs = new AggregateArgs { Pipeline = operations };
            var result = Resources.Aggregate(aggregateArgs);
            return result
                .Select(g => BsonSerializer.Deserialize<AggregationResource<T>>(g))
                .Select(d => d.Resource)
                .AsQueryable();
        }

        private IQueryable<T> SearchKernel<T>(string search) where T : Resource
        {
            return SearchKernel<T>(search, null);
        }

        private IQueryable<T> SearchKernel<T>(string search, IMongoQuery filter) where T : Resource
        {
            if (search == null)
            {
                throw new ArgumentNullException("search");
            }
            var resourceType = ResourceTypeResolver.Resolve<T>();
            var query = CreateVisibilityQuery();
            query = Query.And(query, Query<T>.EQ(r => r.Type, resourceType));
            if (resourceType == ResourceType.GeoSource)
            {
                query = Query.And(query, Query<GeoSource>.EQ(r => r.State, PipelineDefinitionState.Active));
            }
            else if (resourceType == ResourceType.Map)
            {
                query = Query.And(query, Query<Map>.EQ(r => r.State, PipelineDefinitionState.Active));
            }
            if (filter != null)
            {
                query = Query.And(query, filter);
            }
            query = Query.And(Query.Text(search), query);
            
            return AggregateSort<T>(query);
        }

        private void ArchivedInsertResource<T>(T resource) where T : Resource
        {
            var metadata = new Metadata(resource.Metadata);
            // Archive the resource
            try
            {
                resource.Metadata = metadata.ToImmutable().ToMetadata();
                InsertToCollection(ResourcesArchive, resource);
            }
            catch (MongoWriteConcernException exception)
            {
                HandleWriteConcernException(exception);
            }
            resource.Metadata = metadata;

            // Add the resource
            try
            {
                InsertToCollection(Resources, resource);
            }
            catch (Exception exception)
            {
                var query = Query<Resource>.EQ(r => r.Id, resource.Id);
                ResourcesArchive.Remove(query, RemoveFlags.Single, WriteConcern.Unacknowledged);
                if (exception.GetType() == typeof(MongoWriteConcernException))
                {
                    HandleWriteConcernException(exception as MongoWriteConcernException);
                }
                else
                {
                    throw;
                }
            }
        }

        private void ArchivedUpdateResource<T>(T newResource) where T : Resource
        {
            var metadata = new Metadata(newResource.Metadata);
            // Archive this version of the resource
            try
            {
                newResource.Metadata = metadata.ToImmutable().ToMetadata();
                InsertToCollection(ResourcesArchive, newResource);
            }
            catch (MongoWriteConcernException exception)
            {
                HandleWriteConcernException(exception);
            }
            newResource.Metadata = metadata;

            // Update the latest version of the resource
            var query = Query<Resource>.EQ(r => r.Id, newResource.Id);
            try
            {
                var result = Resources.Update(query, Update.Replace(newResource), WriteConcern.Acknowledged);
                HandleWriteConcernResult(result);
            }
            catch (Exception exception)
            {
                query = Query.And(query, Query<Resource>.EQ(r => r.Version, newResource.Version));
                ResourcesArchive.Remove(query, RemoveFlags.Single, WriteConcern.Unacknowledged);
                if (exception.GetType() == typeof(MongoWriteConcernException))
                {
                    HandleWriteConcernException(exception as MongoWriteConcernException);
                }
                else
                {
                    throw;
                }
            }
        }

        private static void HandleWriteConcernResult(WriteConcernResult result)
        {
            if (result.HasLastErrorMessage)
            {
                throw new MongoException(result.LastErrorMessage);
            }
        }

        private static void HandleWriteConcernException(MongoWriteConcernException exception)
        {
            if (exception.Code == 11000)
            {
                throw new DataLayerException(exception.Message, DataLayerExceptionType.Conflict);
            }
            else
            {
                throw exception;
            }
        }

        private void InsertToCollection<T>(MongoCollection collection, T resource) where T : Resource
        {
            var result = collection.Insert(resource, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
        }

        private static void UpdateGeoSource(MultiDomainGeoSource geoSource, MultiDomainGeoSource newGeoSource)
        {
            if (geoSource.DataSize != null) { newGeoSource.DataSize = geoSource.DataSize; }
            if (geoSource.State != null) { newGeoSource.State = geoSource.State; }
            if (geoSource.Styles != null) { newGeoSource.Styles = geoSource.Styles; }
            if (geoSource.Style != null) { newGeoSource.Style = geoSource.Style; }
            if (geoSource.Related != null) { newGeoSource.Related = geoSource.Related; }
            if (geoSource.UsedBy != null) { newGeoSource.UsedBy = geoSource.UsedBy; }
            if (geoSource.Domains != null) { newGeoSource.Domains = geoSource.Domains; }
            UpdatePipeline(geoSource, newGeoSource);
        }

        private static void UpdateMap(Map map, Map newMap)
        {
            if (map.Camera != null) { newMap.Camera = map.Camera; }
            if (map.State != null) { newMap.State = map.State; }
            if (map.Time != null) { newMap.Time = map.Time; }
            if (map.Related != null) { newMap.Related = map.Related; }
            if (map.Groups != null) { newMap.Groups = map.Groups; }
            if (map.Dashboards != null) { newMap.Dashboards = map.Dashboards; }
            if (map.Theme != null) { newMap.Theme = map.Theme; }
            UpdatePipeline(map, newMap);
        }

        private static void UpdatePipeline<T>(T pipeline, T newPipeline) where T : Pyxis.Contract.Publishing.Pipeline
        {
            if (pipeline.Definition != null) { newPipeline.Definition = pipeline.Definition; }
            if (pipeline.ProcRef != null) { newPipeline.ProcRef = pipeline.ProcRef; }
            if (pipeline.Specification != null) { newPipeline.Specification = pipeline.Specification; }
            if (pipeline.BasedOn != null) { newPipeline.BasedOn = pipeline.BasedOn; }
            UpdateResourceFields(pipeline, newPipeline);
        }

        private static void UpdateUser(User user, User newUser)
        {
            if (user.Seller != null) { newUser.Seller = user.Seller; }
            if (user.Subscribed != null) { newUser.Subscribed = new List<Guid>(user.Subscribed); }
            if (user.Galleries != null) { newUser.Galleries = new List<Guid>(user.Galleries); }
            if (user.Groups != null) { newUser.Groups = new List<GroupInfo>(user.Groups); }
            UpdateResourceFields(user, newUser);
        }


        private static void UpdateGroup(Group group, Group newGroup)
        {
            if (group.Members != null) { newGroup.Members = new List<UserInfo>(group.Members); }
            UpdateResourceFields(group, newGroup);
        }

        private static void UpdateLicense(License license, License newLicense)
        {
            if (license.Terms != null)
            {
                if(license.Terms.Text != null) { newLicense.Terms.Text = license.Terms.Text; }
            }
            if (license.ExportOptions != null)
            {
                if (license.ExportOptions.Formats != null) { newLicense.ExportOptions.Formats = new List<string>(license.ExportOptions.Formats); }
                if (license.ExportOptions.AllowReport != null) { newLicense.ExportOptions.AllowReport = license.ExportOptions.AllowReport; }
            }
            UpdateResourceFields(license, newLicense);
        }

        private static void UpdateGallery(Gallery gallery, Gallery newGallery)
        {
            if (gallery.Resources != null) { newGallery.Resources = new List<GalleryResource>(gallery.Resources); }
            if (gallery.Groups != null) { newGallery.Groups = new List<GroupPermissionInfo>(gallery.Groups); }
            UpdateResourceFields(gallery, newGallery);
        }

        private static void UpdateProduct(Product product, Product newProduct)
        {
            if (product.Url != null) { newProduct.Url = product.Url; }
            if (product.Key != null) { newProduct.Key = product.Key; }
            if (product.TransferType != null) { newProduct.TransferType = product.TransferType; }
            UpdateResourceFields(product, newProduct);
        }

        private static void UpdateResourceFields<T>(T resource, T newResource) where T : Resource
        {
            if (resource.Licenses != null) { newResource.Licenses = new List<LicenseReference>(resource.Licenses); }
            UpdateMetadataFields(resource, newResource);
        }

        private static void UpdateMetadataFields<T>(T resource, T newResource) where T : Resource
        {
            if (resource.Metadata == null)
            {
                return;
            }
            if (resource.Metadata.Category != null) { newResource.Metadata.Category = resource.Metadata.Category; }
            if (resource.Metadata.Description != null) { newResource.Metadata.Description = resource.Metadata.Description; }
            if (resource.Metadata.ExternalUrls != null) { newResource.Metadata.ExternalUrls = new List<ExternalUrl>(resource.Metadata.ExternalUrls); }
            if (resource.Metadata.Name != null) { newResource.Metadata.Name = resource.Metadata.Name; }
            if (resource.Metadata.Tags != null) { newResource.Metadata.Tags = new List<string>(resource.Metadata.Tags); }
            if (resource.Metadata.SystemTags != null) { newResource.Metadata.SystemTags = new List<string>(resource.Metadata.SystemTags); }
            if (resource.Metadata.User != null) { newResource.Metadata.User = resource.Metadata.User; }
            if (resource.Metadata.Providers != null) { newResource.Metadata.Providers = resource.Metadata.Providers; }
            if (resource.Metadata.Visibility.HasValue) { newResource.Metadata.Visibility = resource.Metadata.Visibility.Value; }
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field)
        {
            return ResourceGroupingAggregation(field, null, null, null);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, string search)
        {
            return ResourceGroupingAggregation(field, null, search, null);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, ResourceType? resourceType)
        {
            return resourceType.HasValue ? ResourceGroupingAggregation(field, resourceType.Value, null, null) : ResourceGroupingAggregation(field, null, null, null);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, ResourceType? resourceType, string search)
        {
            return resourceType.HasValue ? ResourceGroupingAggregation(field, resourceType.Value, search, null) : ResourceGroupingAggregation(field, null, search, null);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, List<Guid> resourceIds)
        {
            return ResourceGroupingAggregation(field, null, null, resourceIds);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, string search, List<Guid> resourceIds)
        {
            return ResourceGroupingAggregation(field, null, search, resourceIds);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, ResourceType resourceType, string search, List<Guid> resourceIds)
        {
            return ResourceGroupingAggregation(field, new List<ResourceType> { resourceType }, search, resourceIds);
        }

        private IQueryable<ResourceGrouping> ResourceGroupingAggregation(string field, List<ResourceType> resourceTypes, string search, List<Guid> resourceIds)
        {
            if (field != "Tags" && field != "Category")
            {
                return new List<ResourceGrouping>().AsQueryable();
            }

            var operations = new List<BsonDocument>();
            var sortCount = new BsonDocument { { "$sort", new BsonDocument { { "Count", -1 } } } };
            if (field == "Tags")
            {
                var unwindTags = new BsonDocument { { "$unwind", "$Metadata.Tags" } };
                var groupTags = new BsonDocument{ { "$group", new BsonDocument{ { "_id", new BsonDocument{ { "_id", "$_id"}, {"Tag", "$Metadata.Tags"} } }, 
                { "Num", new BsonDocument{ { "$addToSet", "$Metadata.Tags"} } } } } };
                var unwindNum = new BsonDocument { { "$unwind", "$Num" } };
                var groupCount = new BsonDocument{ { "$group", new BsonDocument{ { "_id", "$_id.Tag" }, 
                { "Count", new BsonDocument{ { "$sum", 1 } } } } } };

                operations.AddRange(new[] { unwindTags, groupTags, unwindNum, groupCount, sortCount });
            }
            else // Category
            {
                var groupCount = new BsonDocument{ { "$group", new BsonDocument{ { "_id", "$Metadata.Category" }, 
                { "Count", new BsonDocument{ { "$sum", 1 } } } } } };

                operations.AddRange(new[] { groupCount, sortCount });
            }

            if (resourceTypes != null && resourceTypes.Count > 0)
            {
                var matchTypes = new List<BsonDocument>();
                foreach (var resourceType in resourceTypes)
                {
                    BsonDocument matchType;
                    if (resourceType == ResourceType.GeoSource || resourceType == ResourceType.Map)
                    {
                        matchType = new BsonDocument { { "Type", resourceType.ToString() }, { "State", "Active" } };
                    }
                    else
                    {
                        matchType = new BsonDocument { { "Type", resourceType.ToString() } };
                    }
                    matchTypes.Add(matchType);
                }
                var matchSpecifiedTypes = new BsonDocument { { "$match", new BsonDocument { { "$or", new BsonArray(matchTypes) } } } };
                operations.Insert(0, matchSpecifiedTypes);
            }

            if (resourceIds != null)
            {
                var matchIds = new BsonDocument { { "$match", new BsonDocument { { "Id", new BsonDocument { { "$in", new BsonArray(resourceIds) } } } } } };
                operations.Insert(0, matchIds);
            }
            if (m_authorizedUser == null || !m_authorizedUser.IsPyxisAdmin)
            {
                BsonDocument matchVisibility;
                if (m_authorizedUser != null)
                {
                    matchVisibility = new BsonDocument { { "$match", new BsonDocument { { "$or", new BsonArray { 
                        new BsonDocument  { { "Metadata.Visibility", VisibilityType.Public.ToString() } }, 
                        new BsonDocument { { "Metadata.User._id", m_authorizedUser.Id } } } } } } };
                }
                else
                {
                    matchVisibility = new BsonDocument { { "$match", new BsonDocument { { "Metadata.Visibility", VisibilityType.Public.ToString() } } } };
                }
                operations.Insert(0, matchVisibility);
            }
            // $text must be the first $match in aggregation
            if (search != null)
            {
                var matchIds = new BsonDocument { { "$match", new BsonDocument { { "$text", new BsonDocument { { "$search", search } } } } } };
                operations.Insert(0, matchIds);
            }

            var aggregateArgs = new AggregateArgs { Pipeline = operations };
            var result = Resources.Aggregate(aggregateArgs);
            return result.Select(g => BsonSerializer.Deserialize<ResourceGrouping>(g)).AsQueryable();
        }

        private enum VisibilityQueryOption
        {
            Default, // include only Public
            IncludeNonDiscoverable // include Public and NonDiscoverable
        }

        #endregion Resources

        #region Pipelines

        public IQueryable<PipelineTaggedInfoNoDefinitionDTO> GetPipelines()
        {
            var query = Query<Resource>.EQ(r => r.Type, ResourceType.GeoSource);

            var map =
                "function() {" +
                    "var imageUrl = null;" +
                    "for(var i = 0; i < this.Metadata.ExternalUrls.length; i++) {" +
                        "if(this.Metadata.ExternalUrls[i][\"Type\"] == \"Image\") {" +
                            "imageUrl = this.Metadata.ExternalUrls[i][\"Url\"];" +
                            "break;" +
                        "}" +
                    "}" +
                    "emit(this.ProcRef, { Category: this.Metadata.Category, Created: this.Metadata.Created, DataSize: this.DataSize, Description: this.Metadata.Description," +
                                        " Name: this.Metadata.Name, ProcRef: this.ProcRef, State: this.State, User: this.Metadata.User.Name, Tags: this.Metadata.Tags," +
                                        " ImageUrl: imageUrl });" +
                "}";

            var reduce =
                "function(key, values) {" +
                    "return values[0];" +
                "}";

            var mapReduceArgs = new MapReduceArgs { Query = query, MapFunction = map, ReduceFunction = reduce };
            var result = Resources.MapReduce(mapReduceArgs);
            return result.GetResults().Select(x => BsonSerializer.Deserialize<PipelineTaggedInfoNoDefinitionDTO>(x["value"].AsBsonDocument)).AsQueryable();
        }

        public PipelineTaggedInfoDTO GetPipeline(string procRef)
        {
            var pipeline = GetPipelineByProcRef(procRef);
            if (pipeline == null) { return null; }
            Pyxis.Contract.Publishing.GeoSource geoSource;
            if (pipeline.Type == ResourceType.GeoSource)
            {
                geoSource = pipeline as Pyxis.Contract.Publishing.GeoSource;
            }
            else
            {
                geoSource = (pipeline as Pyxis.Contract.Publishing.Pipeline).ToGeoSource();
            }
            return PipelineTaggedInfoFactory.Create(geoSource);
        }

        public PublishedPipelineDetailsDTO GetPipelineDetails(string procRef)
        {
            var geoSource = GetPipelineByProcRef(procRef) as Pyxis.Contract.Publishing.GeoSource;
            if (geoSource == null)
            {
                throw new DataLayerException("No GeoSource with specified ProcRef", DataLayerExceptionType.NotFound);
            }

            // Consider having Gwss transmit dictionaries instead of lists
            var map =
                "function() {" +
                    "var gwss = this;" +
                    "var id = gwss._id;" +
                    "gwss.Status.PipelinesStatuses.forEach(function (pipelineStatus) {" +
                        "if (pipelineStatus.ProcRef === \"" + procRef + "\") {" +
                            "var operation = null;" +
                            "if(gwss.Status.OperationsStatuses != undefined) {" +
                                "gwss.Status.OperationsStatuses.forEach(function (operationStatus) {" +
                                    "if(operationStatus.Operation.Parameters[\"ProcRef\"] === \"" + procRef + "\") {" +
                                        "operation = operationStatus;" +
                                    "}" +
                                "})" +
                            "}" +
                            "emit(\"" + procRef + "\", { PublishingServerStatus: [{ NodeId: id, ProcRef: \"" + procRef + "\", Status: pipelineStatus.StatusCode, OperationStatus: operation }]});" +
                        "}" +
                    "})" +
                "}";

            var reduce =
                "function(key, values) {" +
                    "var result = { PublishingServerStatus: [] };" +
                    "values.forEach(function (value) {" +
                        "result.PublishingServerStatus = result.PublishingServerStatus.concat(value.PublishingServerStatus);" +
                    "});" +
                    "return result;" +
                "}";

            var mapReduceArgs = new MapReduceArgs { MapFunction = map, ReduceFunction = reduce };
            var result = Gwsses.MapReduce(mapReduceArgs);
            var results = result.GetResults();
            var publishedPipelineDetails = results.Select(x => BsonSerializer.Deserialize<PublishedPipelineDetailsDTO>(x["value"].AsBsonDocument)).FirstOrDefault();
            if (publishedPipelineDetails == null)
            {
                publishedPipelineDetails = new PublishedPipelineDetailsDTO();
            }

            publishedPipelineDetails.DataSize = geoSource.DataSize.Value;
            publishedPipelineDetails.ProcRef = procRef;
            return publishedPipelineDetails;
        }

        #endregion Pipelines

        #region Activities

        public void InsertComment(Comment comment)
        {
            var resource = GetResourceById(comment.ResourceId);
            if (resource == null)
            {
                throw new DataLayerException("Not found - No resource found with the given ID", DataLayerExceptionType.NotFound);
            }

            var query = Query.And(Query<Resource>.EQ(r => r.Type, resource.Type),
                Query<Resource>.EQ(r => r.Id, comment.ResourceId));
            InsertCommentInHierarchy(resource.Metadata.Comments, comment);
            var result = Activities.Insert(comment, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
            result = Resources.Update(query, Update<Resource>.Set(r => r.Metadata.Comments, resource.Metadata.Comments), WriteConcern.Acknowledged);
            if (result.HasLastErrorMessage)
            {
                query = Query.And(Query<Activity>.EQ(a => a.Type, ActivityType.Comment),
                    Query<Comment>.Where(c => c.ResourceId == comment.ResourceId && c.Id == comment.Id));
                Activities.Remove(query);
                HandleWriteConcernResult(result);
            }
        }

        public void InsertRating(Rating rating)
        {
            var ratingQuery = Query.And(Query<Activity>.EQ(a => a.Type, ActivityType.Rating),
                Query<Rating>.EQ(r => r.ResourceId, rating.ResourceId),
                Query<Rating>.EQ(r => r.User.Id, rating.User.Id));
            var previousValue = 0;
            var previousRating = Activities.FindOneAs<Rating>(ratingQuery);
            if (previousRating != null)
            {
                previousValue = previousRating.Value;
                if (rating.Value == previousValue)
                {
                    throw new DataLayerException("Not Modified - New rating is the same as the previous rating", DataLayerExceptionType.NotModified);
                }
                rating.Id = previousRating.Id;
            }
            else if (rating.Value == 0)
            {
                throw new DataLayerException("Not Modified - Cannot reset because no previous rating exists", DataLayerExceptionType.NotModified);
            }
            int likeIncrement, dislikeIncrement;
            switch (rating.Value)
            {
                case 1:
                    likeIncrement = 1;
                    dislikeIncrement = previousValue < 0 ? -1 : 0;
                    break;
                case 0:
                    likeIncrement = previousValue > 0 ? -1 : 0;
                    dislikeIncrement = previousValue < 0 ? -1 : 0;
                    break;
                case -1:
                    likeIncrement = previousValue > 0 ? -1 : 0;
                    dislikeIncrement = 1;
                    break;
                default:
                    throw new DataLayerException("Unsupported rating value", DataLayerExceptionType.BadRequest);
            }

            var updates = Update.Combine(Update<Resource>.Inc(r => r.Metadata.Ratings.Likes, likeIncrement),
                Update<Resource>.Inc(r => r.Metadata.Ratings.Dislikes, dislikeIncrement));
            var result = Activities.Update(ratingQuery, Update.Replace(rating), UpdateFlags.Upsert, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
            var resourceQuery = Query<Resource>.EQ(r => r.Id, rating.ResourceId);
            result = Resources.Update(resourceQuery, updates, WriteConcern.Acknowledged);
            if (result.DocumentsAffected == 0)
            {
                Activities.Remove(ratingQuery);
                throw new DataLayerException("Invalid resource specified", DataLayerExceptionType.NotFound);
            }
        }

        public IQueryable<Agreement> GetAgreements(DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetAgreementsByLicenseId(Guid licenseId, DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.ResourceId == licenseId && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetAgreementsByLicenseVersionedId(Guid licenseId, Guid licenseVersion, DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.ResourceId == licenseId && a.LicenseVersion == licenseVersion
                && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetAgreementsByUser(Guid userId, DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.User.Id == userId && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetAgreementsByUserAndLicenseId(Guid userId, Guid licenseId, DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.ResourceId == licenseId && a.User.Id == userId && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetAgreementsByUserAndLicenseVersionedId(Guid userId, Guid licenseId, Guid licenseVersion, DateTime from, DateTime until)
        {
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.ResourceId == licenseId && a.LicenseVersion == licenseVersion && a.User.Id == userId
                && a.Created >= from && a.Created <= until);
        }

        public IQueryable<Agreement> GetActiveAgreementsByUserAndLicenseReferences(Guid userId, List<LicenseReference> licenseReferences)
        {
            // An agreement to an old license remains valid until it expires
            var licenseIds = licenseReferences.Select(l => l.Id).ToList();
            return Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.User.Id == userId && a.Decision == DecisionType.Agree
                && licenseIds.Contains(a.ResourceId) && a.Expiration > DateTime.UtcNow);
        }

        public LicensedAccess GetLicensedAccessToResource(Guid userId, Guid resourceId)
        {
            var resource = GetResourceById(resourceId);
            if (resource == null)
            {
                throw new DataLayerException("No resource found with the specified Resource Id", DataLayerExceptionType.NotFound);
            }
            var licensedAccess = new LicensedAccess { HasAccess = true, UsingLicenses = new List<VersionedLicenseReference>() };
            if (resource.Licenses.Any())
            {
                // An agreement to an old license remains valid until it expires
                var licenseIds = resource.Licenses.Select(l => l.Id).ToList();
                var agreements = Activities.AsQueryable<Agreement>().Where(a => a.Type == ActivityType.Agreement && a.User.Id == userId && a.Decision == DecisionType.Agree
                    && licenseIds.Contains(a.ResourceId) && a.Expiration > DateTime.UtcNow).ToList();
                if (!agreements.Any())
                {
                    licensedAccess.HasAccess = false;
                }
                else
                {
                    var agreement = agreements.Where(a => resource.Licenses.Any(l => l.LicenseType == LicenseType.Full && l.Id == a.ResourceId)).OrderByDescending(l => l.Created).FirstOrDefault();
                    if (agreement == null)
                    {
                        agreement = agreements.Where(a => resource.Licenses.Any(l => l.LicenseType == LicenseType.Trial && l.Id == a.ResourceId)).OrderByDescending(l => l.Created).FirstOrDefault();
                    }
                    if (agreement == null)
                    {
                        licensedAccess.HasAccess = false;
                    }
                    else
                    {
                        licensedAccess.UsingLicenses.Add(new VersionedLicenseReference(agreement.ResourceId, agreement.LicenseType, agreement.Limitations, agreement.LicenseVersion));
                    }
                }
            }
            return licensedAccess;
        }

        public void InsertAgreement(Agreement agreement)
        {
            var license = GetResourceByIdAndVersion<License>(agreement.ResourceId, agreement.LicenseVersion);
            if (license == null)
            {
                throw new DataLayerException("Not found - No license found with the given Id and Version", DataLayerExceptionType.NotFound);
            }

            var previousAgreement = GetAgreementsByUserAndLicenseVersionedId(agreement.User.Id, agreement.ResourceId, agreement.LicenseVersion, DateTime.MinValue, DateTime.MaxValue)
                .OrderByDescending(a => a.Created).FirstOrDefault();
            if (previousAgreement != null && agreement.Decision == previousAgreement.Decision)
            {
                throw new DataLayerException("Not Modified - New agreement is the same as the previous agreement", DataLayerExceptionType.NotModified);
            }

            var result = Activities.Insert(agreement, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
        }

        private static void InsertCommentInHierarchy(LinkedList<AggregateComment> comments, Comment comment)
        {
            if (!comments.Insert(comment))
            {
                throw new DataLayerException("Not found - No Comment with given ID to reply to", DataLayerExceptionType.NotFound);
            }
        }

        #endregion Activities

        #region Parameters
        // List of Site Parameters is in SiteParameters.txt in the root of LicenseServer.csproj

        // Serializing structs (KeyValuePair) is problematic, internally using a class
        internal class MongoKeyValuePair
        {
            public string Key { get; set; }
            public object Value { get; set; }

            public MongoKeyValuePair(KeyValuePair<string, object> pair)
            {
                Key = pair.Key;
                Value = pair.Value;
            }
        }

        public IQueryable<KeyValuePair<string, object>> GetParameters()
        {
            return Parameters.AsQueryable().Select(p => new KeyValuePair<string, object>(p.Key, p.Value));
        }

        public object GetParameter(string key)
        {
            var pair = Parameters.AsQueryable().FirstOrDefault(p => p.Key == key);
            return pair == null ? null : pair.Value;
        }

        public void UpdateParameter(KeyValuePair<string, object> pair)
        {
            var query = Query<MongoKeyValuePair>.EQ(p => p.Key, pair.Key);
            var update = Update<MongoKeyValuePair>.Replace(new MongoKeyValuePair(pair));
            var result = Parameters.Update(query, update, UpdateFlags.Upsert, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
        }

        #endregion Parameters

        #region Gwsses

        public IQueryable<Gwss> GetGwsses()
        {
            return Gwsses.FindAllAs<Gwss>().AsQueryable();
        }

        // GetGwsses without LS Response to reduce data transfer
        public IQueryable<Gwss> GetGwssesStatuses()
        {
            return Gwsses.FindAllAs<Gwss>().SetFields(Fields.Include("Id", "LastHeard", "Status")).AsQueryable();
        }

        public Gwss GetGwssById(Guid id)
        {
            var query = Query<Gwss>.EQ(g => g.Id, id);
            return Gwsses.FindOneAs<Gwss>(query);
        }

        // GetGwssById without LS Response to reduce data transfer
        public Gwss GetGwssStatusById(Guid id)
        {
            var query = Query<Gwss>.EQ(g => g.Id, id);
            return Gwsses.FindAs<Gwss>(query).SetFields(Fields.Include("Id", "LastHeard", "Status")).FirstOrDefault();
        }

        public void UpdateGwssStatus(Guid id, GwssStatus gwssStatus)
        {
            var query = Query<Gwss>.EQ(g => g.Id, id);
            var update = Update<Gwss>.Set(g => g.LastHeard, DateTime.UtcNow)
                .Set(g => g.Status, gwssStatus);
            var result = Gwsses.Update(query, update, UpdateFlags.Upsert, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
        }

        public void RemoveGwss(Guid id)
        {
            var query = Query<PipelineServerStatus>.EQ(p => p.ServerId, id);
            var result = Gwsses.Remove(query, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);

            query = Query<Gwss>.EQ(g => g.Id, id);
            result = Gwsses.Remove(query, WriteConcern.Acknowledged);
            HandleWriteConcernResult(result);
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

            var query = Query.And(Query<PyxisIdentityAffiliate>.NE(d => d.ResourceId, null),
                Query<PyxisIdentityAffiliate>.Matches(d => d.Email, new BsonRegularExpression(new Regex(atDomain + "$"))));
            return Identities.FindAs<PyxisIdentityAffiliate>(query)
                .SetFields(Fields
                    .Exclude("_id")
                    .Include("UserName", "FirstName", "LastName", "Email", "ResourceId"))
                    .AsQueryable();
        }

        #endregion Identities
    }
}