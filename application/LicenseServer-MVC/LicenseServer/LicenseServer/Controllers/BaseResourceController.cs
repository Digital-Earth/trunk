using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.App_Utilities;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using Gallery = LicenseServer.Models.Mongo.Gallery;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;
using Map = LicenseServer.Models.Mongo.Map;
using User = LicenseServer.Models.Mongo.User;
using Group = LicenseServer.Models.Mongo.Group;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;

namespace LicenseServer.Controllers
{
    public abstract class BaseResourceController<T> : CORSMongoApiController where T : Resource 
    {
        public BaseResourceController() 
        { }

        // Inject for test
        protected BaseResourceController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Resource
        public virtual PageResult<dynamic> Get(ODataQueryOptions<T> options)
        {
            return db.GetResources<T>().ToPageResult(Request, options);
        }

        // GET api/v1/Resource?search={search string}
        public virtual PageResult<dynamic> Get(string search, ODataQueryOptions<T> options)
        {
            return db.SearchResources<T>(search).ToPageResult(Request, options);
        }

        // GET api/v1/Resource?search={search string}&Grouping={field}
        public virtual PageResult<dynamic> GetSearchGroupings(string search, string grouping, ODataQueryOptions<ResourceGrouping> options)
        {
            return db.GetResourceGroupings<T>(search, grouping).ToPageResult(Request, options);
        }

        // GET api/v1/Resource/5
        public virtual HttpResponseMessage Get(Guid id)
        {
            var result = db.GetResourceById<T>(id);
            
            return CreateResponse(result);
        }

        // GET api/v1/Resource?name={name}
        public virtual HttpResponseMessage Get(string name)
        {
            var result = db.GetResourcesByName<T>(name);

            return CreateResponse(result.FirstOrDefault());
        }

        // GET api/v1/Resource/5/Versions
        [ActionName("Versions")]
        public virtual PageResult<dynamic> GetVersions(Guid resourceId, ODataQueryOptions<T> options)
        {
            return db.GetResourceVersionsById<T>(resourceId).ToPageResult(Request, options);
        }

        // GET api/v1/Resource/5?Version={version}
        public virtual HttpResponseMessage Get(Guid id, Guid version)
        {
            var result = db.GetResourceByIdAndVersion<T>(id, version);
            
            return CreateResponse(result);
        }

        // GET api/v1/Resource?Grouping={field}
        public virtual PageResult<dynamic> GetGroupings(string grouping, ODataQueryOptions<ResourceGrouping> options)
        {
            return db.GetResourceGroupings<T>(grouping).ToPageResult(Request, options);
        }

        // POST api/v1/Resource
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public virtual HttpResponseMessage Post(T resource)
        {
            if (!(resource is Pyxis.Contract.Publishing.User || resource is Pyxis.Contract.Publishing.Map) 
                && CurrentUserIdentity.IsInRole(PyxisIdentityRoles.Unconfirmed))
            {
                return Request.CreateResponse(HttpStatusCode.Forbidden, "Confirm your email address to complete the request.");
            }
            if (resource.Metadata == null || resource.Metadata.Name == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Resource.Metadata.Name must be specified");
            }
            Gallery gallery = null; 
            if (resource.Metadata.Providers != null)
            {
                if (resource.Metadata.Providers.Count > 1)
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "Resource can have at most one provider");
                }
                if (resource.Metadata.Providers.Any())
                {
                    var provider = resource.Metadata.Providers.First();
                    gallery = db.GetResourceById<Gallery>(provider.Id);
                    if (gallery == null)
                    {
                        return Request.CreateResponse(HttpStatusCode.BadRequest, "No Provider Gallery found with the specified Id.");
                    }
                    if (!(CurrentUserIdentity == null && resource.Metadata.Providers[0].Id == Properties.Settings.Default.AnonymousGalleryId) 
                        && !CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery) && !ValidateGalleryPermission(gallery))
                    {
                        return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified gallery");
                    }
                    provider.Name = gallery.Metadata.Name;
                    provider.Type = ResourceType.Gallery;
                }
            }
            if (!AnonymousOrCompleteProfile())
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            if (resource.Metadata.User == null || !AnonymousOrAdmin())
            {
                resource.Metadata.User = CurrentUserIdentity.UserInfo;
            }
            if (resource.Id == Guid.Empty)
            {
                resource.Id = Guid.NewGuid();
            }
            if (resource.Version == Guid.Empty)
            {
                resource.Version = Guid.NewGuid();
            }
            resource.Type = ResourceTypeResolver.Resolve<T>().Value;
            resource.Metadata.Created = DateTime.UtcNow;
            resource.Metadata.Updated = resource.Metadata.Created;

            FillReferenceTypeNulls(resource);

            HttpResponseMessage response;
            try
            {
                db.InsertResource(resource);
                if (gallery != null)
                {
                    gallery.Resources.Add(new GalleryResource { ResourceId = resource.Id });
                    UpdateResource(gallery.Id, gallery.Version, new Gallery() { Resources = gallery.Resources });
                }
                response = Request.CreateResponse(HttpStatusCode.Created, resource);
                var apiLink = Url.Link("DefaultApi", new {id = resource.Id});
                if (apiLink != null)
                {
                    response.Headers.Location = new Uri(apiLink);
                }
            }
            catch (DataLayerException exception)
            {
                response = exception.ToHttpResponse(Request);
            }
            return response;
        }

        // POST api/v1/Resource
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPost]
        public PageResult<dynamic> ResourceUpdates(List<Guid> resources, DateTime lastUpdated, ODataQueryOptions<T> options)
        {
            return db.GetUpdates<T>(resources, lastUpdated).ToPageResult(Request, options);
        }

        // PUT api/v1/Resource/5?Version={version}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public virtual HttpResponseMessage Put(Guid id, Guid version, T resource)
        {
            var existingResource = db.GetResourceById<T>(id);
            if (existingResource == null)
            {
                return Request.CreateResponse(HttpStatusCode.NotFound, "No resource with the specified ID");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(existingResource, CurrentUserIdentity))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to update requested resource");
            }
            if (resource.Licenses != null && resource.Licenses.Any())
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Licenses must be attached using the License API");
            }
            if (resource.Metadata != null && resource.Metadata.SystemTags != null && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to set SystemTags");
            }
            return UpdateResource(id, version, resource);
        }

        // DELETE api/v1/Resource/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public virtual HttpResponseMessage Delete(Guid id)
        {
            if (!CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delete requested resource");
            }

            try
            {
                db.RemoveResource<T>(id);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // Determining if the currently logged in user is the owner or has ownership rights to the resource.
        // For [Authorize]d requests.
        protected virtual bool ValidateOwnership(Resource resource)
        {
            return ValidateOwnership(resource, CurrentUserIdentity);
        }

        protected virtual bool ValidateOwnership(Resource resource, PyxisIdentityUser currentUserIdentity)
        {
            // Incapable of ownership
            if (!currentUserIdentity.ResourceId.HasValue)
            {
                return false;
            }
            // Is the Resource owner
            if (resource.Metadata.User.Id == currentUserIdentity.ResourceId.Value)
            {
                return true;
            }
            // Is the channel admin of Gallery
            var gallery = resource as Gallery;
            if (gallery != null && gallery.Admin != null && gallery.Admin.Id == currentUserIdentity.ResourceId.Value)
            {
                return true;
            }
            // Resource in current User's Gallery
            if (resource.Metadata.Providers.Any(p => p.Type == ResourceType.Gallery))
            {
                var userResource = db.GetResourceById<User>(currentUserIdentity.ResourceId.Value);
                if (resource.Metadata.Providers.Any(p => p.Type == ResourceType.Gallery && userResource.Galleries.Contains(p.Id)))
                {
                    return true;
                }
            }
            return false;
        }

        // Determining if the currently logged in user has rights to publish to a Gallery.
        // For [Authorize]d requests.
        protected virtual bool ValidateGalleryPermission(Gallery gallery)
        {
            return ValidateGalleryPermission(gallery, CurrentUserIdentity);
        }

        protected virtual bool ValidateGalleryPermission(Gallery gallery, PyxisIdentityUser currentUserIdentity)
        {
            // Incapable of ownership
            if (!currentUserIdentity.ResourceId.HasValue)
            {
                return false;
            }
            // See if the Gallery has any Groups with sufficient permission
            var permittedGroups = gallery.Groups.Where(g => g.Permission == GroupPermission.Publish).Select(g => g.Id).ToArray();
            if (!permittedGroups.Any())
            {
                return false;
            }
            // See if the Group is a member of any of the permitted Groups
            var userGroups = db.GetResourceById<User>(currentUserIdentity.ResourceId.Value).Groups.Select(g => g.Id);
            if (userGroups.Intersect(permittedGroups).Any())
            {
                return true;
            }
            return false;
        }

        protected List<Guid> RemoveGalleryProviders(T existingResource, T newResource)
        {
            var providerGalleries = existingResource.Metadata.Providers.Where(x => x.Type == ResourceType.Gallery).Select(p => p.Id).ToList();
            
            existingResource.Metadata.Providers.RemoveAll(x => x.Type == ResourceType.Gallery);
            if (existingResource.Metadata.Providers.Any())
            {
                if (newResource.Metadata == null)
                {
                    newResource.Metadata = new Metadata();
                }
                newResource.Metadata.Providers = existingResource.Metadata.Providers;
            }

            return providerGalleries;
        }

        protected HttpResponseMessage RemoveResourceFromProviderGalleries(List<Guid> providerGalleryIds, Guid resourceId)
        {
            var response = new HttpResponseMessage(HttpStatusCode.OK);
            foreach (var providerGalleryId in providerGalleryIds)
            {
                var gallery = db.GetResourceById<Gallery>(providerGalleryId);
                if (gallery.Resources.RemoveAll(r => r.ResourceId == resourceId) > 0)
                {
                    try
                    {
                        db.UpdateResource(gallery.Id, gallery.Version, new Gallery() { Resources = gallery.Resources });
                    }
                    catch (DataLayerException exception)
                    {
                        if (response.StatusCode != HttpStatusCode.OK)
                        {
                            response = exception.ToHttpResponse(Request);
                        }
                    }
                }
            }
            return response;
        }
        
        protected HttpResponseMessage CreateResponse(T result)
        {
            if (result != null)
            {
                return Request.CreateResponse(HttpStatusCode.OK, result);
            }
            return Request.CreateResponse(HttpStatusCode.NotFound);
        }

        private HttpResponseMessage UpdateResource<TR>(Guid id, Guid version, TR resource) where TR : Resource
        {
            try
            {
                db.UpdateResource(id, version, resource);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        private static void FillReferenceTypeNulls(T resource)
        {
            // prevent nulls from being inserted (can cause OData errors)
            if (resource.Licenses == null) { resource.Licenses = new List<LicenseReference>(); }
            if (resource.Metadata.Providers == null) { resource.Metadata.Providers = new List<Provider>(); }
            if (resource.Metadata.Comments == null) { resource.Metadata.Comments = new LinkedList<AggregateComment>(); }
            if (resource.Metadata.Ratings == null) { resource.Metadata.Ratings = new AggregateRatings(); }
            if (resource.Metadata.ExternalUrls == null) { resource.Metadata.ExternalUrls = new List<ExternalUrl>(); }
            if (resource.Metadata.Tags == null) { resource.Metadata.Tags = new List<string>(); }
            if (resource.Metadata.SystemTags == null) { resource.Metadata.SystemTags = new List<string>(); }
            if (resource.Metadata.Visibility == null) { resource.Metadata.Visibility = VisibilityType.Public; }
            switch (resource.Type)
            {
                case ResourceType.GeoSource:
                    var geoSource = resource as MultiDomainGeoSource;
                    if (geoSource.BasedOn == null) { geoSource.BasedOn = new List<ResourceReference>(); }
                    if (geoSource.Styles == null) { geoSource.Styles = new List<ResourceReference>(); }
                    if (geoSource.UsedBy == null) { geoSource.UsedBy = new List<Guid>(); }
                    if (geoSource.Related == null) { geoSource.Related = new List<Guid>(); }
                    if (geoSource.Specification == null) { geoSource.Specification = new PipelineSpecification(); }
                    break;
                case ResourceType.Map:
                    var map = resource as Map;
                    if (map.BasedOn == null) { map.BasedOn = new List<ResourceReference>(); }
                    if (map.Related == null) { map.Related = new List<Guid>(); }
                    if (map.Specification == null) { map.Specification = new PipelineSpecification(); }
                    if (map.Dashboards == null) { map.Dashboards = new List<Dashboard>(); }
                    if (map.Groups == null) { map.Groups = new List<Pyxis.Contract.Publishing.Map.Group>(); }
                    break;
                case ResourceType.User:
                    var user = resource as User;
                    if (user.Galleries == null) { user.Galleries = new List<Guid>(); }
                    if (user.Subscribed == null) { user.Subscribed = new List<Guid>(); }
                    if (user.Groups == null) { user.Groups = new List<GroupInfo>(); }
                    break;
                case ResourceType.Group:
                    var group = resource as Group;
                    if (group.Members == null) { group.Members = new List<UserInfo>(); }
                    break;
                case ResourceType.License:
                    var license = resource as License;
                    if (license.Terms == null) { license.Terms = new LicenseTerms(); }
                    if (license.ExportOptions == null) { license.ExportOptions = new LicenseExportOptions { AllowReport = false }; }
                    if (license.ExportOptions.Formats == null) { license.ExportOptions.Formats = new List<string>(); }
                    if (license.Limitations == null) { license.Limitations = new LicenseTrialLimitations(); }
                    break;
                case ResourceType.Gallery:
                    var gallery = resource as Gallery;
                    if (gallery.Resources == null) { gallery.Resources = new List<GalleryResource>(); }
                    if (gallery.Groups == null) { gallery.Groups = new List<GroupPermissionInfo>(); }
                    break;
                case ResourceType.File:
                    break;
                case ResourceType.Product:
                    break;
                default:
                    throw new NotImplementedException("ResourceType " + resource.Type.ToString() + " is not yet supported.");
            }
        }

        private bool AnonymousOrCompleteProfile()
        {
            return CurrentUserIdentity == null || CurrentUserIdentity.ResourceId.HasValue;
        }

        private bool AnonymousOrAdmin()
        {
            return CurrentUserIdentity == null || CurrentUserIdentity.IsInAdminRole();
        }
    }
}
