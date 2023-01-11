using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using Microsoft.AspNet.Identity;
using LicenseServer.App_Utilities;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.Ajax.Utilities;
using Pyxis.Contract.Publishing;
using Gallery = LicenseServer.Models.Mongo.Gallery;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;
using Map = LicenseServer.Models.Mongo.Map;
using Product = LicenseServer.Models.Mongo.Product;
using Resource = Pyxis.Contract.Publishing.Resource;
using User = LicenseServer.Models.Mongo.User;
using Group = LicenseServer.Models.Mongo.Group;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Gallery")]
    public class GalleryController : BaseResourceController<Gallery>
    {
        public GalleryController() 
        { }

        // Inject for test
        public GalleryController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Gallery/Available
        [HttpGet]
        [Route("Available")]
        public HttpResponseMessage Available(string name)
        {
            if (!NameBlacklist.Contains(name) && db.GetResourcesByName<Gallery>(name).FirstOrDefault() == null)
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            return Request.CreateResponse(HttpStatusCode.Conflict);
        }

        // GET api/v1/Gallery/Available
        [HttpGet]
        [Route("Available")]
        public HttpResponseMessage Available(Guid userId, string name)
        {
            if (db.GetResourcesByName<Gallery>(userId, name).FirstOrDefault() == null)
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.Conflict);
            }
        }
        
        // GET api/v1/Gallery/{id}/Expanded
        [HttpGet]
        [Route("{id}/Expanded")]
        [ResponseType(typeof(GalleryExpandedDTO))]
        public HttpResponseMessage GetExpandedGallery(Guid id)
        {
            return GetExpandedGallery(id, ResultFormat.Basic);
        }

        // GET api/v1/Gallery/{id}/Expanded?Format=(Full|Basic|View}
        [HttpGet]
        [Route("{id}/Expanded")]
        [ResponseType(typeof(GalleryExpandedDTO))]
        public HttpResponseMessage GetExpandedGallery(Guid id, ResultFormat format)
        {
            var galleryResponse = base.Get(id);
            Gallery gallery;
            if (galleryResponse.TryGetContentValue(out gallery))
            {
                var containedResources = db.GetResourcesByIds(gallery.Resources.Select(r => r.ResourceId).ToList()).FormatResources(format);
                var galleryExpandedDto = GalleryExpandedFactory.Create(gallery, containedResources);
                return Request.CreateResponse(HttpStatusCode.OK, galleryExpandedDto);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }
        }

        // GET api/v1/Gallery/{id}/Grouping?field={field}
        [HttpGet]
        [Route("{id}/Grouping")]
        public PageResult<dynamic> GetGalleryGroupings(Guid id, string field, ODataQueryOptions<Pyxis.Contract.Publishing.ResourceGrouping> options)
        {
            return db.GetGalleryGroupings(id, field).ToPageResult(Request, options);
        }

        // GET api/v1/Gallery/{id}/Grouping?search={search}&field={field}
        [HttpGet]
        [Route("{id}/Grouping")]
        public PageResult<dynamic> GetGallerySearchGroupings(Guid id, string search, string field, ODataQueryOptions<Pyxis.Contract.Publishing.ResourceGrouping> options)
        {
            return db.GetGalleryGroupings(id, search, field).ToPageResult(Request, options);
        }

        // GET api/v1/Gallery/{id}/Search?search={search}
        [HttpGet]
        [Route("{id}/Search")]
        public PageResult<dynamic> SearchGallery(Guid id, string search, ODataQueryOptions<Resource> options)
        {
            return db.SearchGallery(id, search).ToPageResult(Request, options);
        }
        
        // POST api/v1/Gallery
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)] // allow unconfirmed token to enter (required to use a token issued before switching Roles to member from unconfirmed)
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(Gallery gallery)
        {
            User publisher = null;
            if (gallery.Metadata == null || gallery.Metadata.Name == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Resource.Metadata.Name must be specified");
            }
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            HttpResponseMessage groupResponse;
            if (DisallowGroup(gallery, out groupResponse))
            {
                return groupResponse;
            }
            var existingGallery = Retry.Execute(() => db.GetResourcesByName<Gallery>(gallery.Metadata.Name)).FirstOrDefault();
            if (existingGallery != null)
            {
                // Ensure the gallery is in the User's Galleries list if they are the owner (handle multiple POSTs)
                if (existingGallery.Metadata.User.Id == CurrentUserIdentity.ResourceId.Value)
                {
                    publisher = Retry.Execute(() => db.GetResourceById<User>(CurrentUserIdentity.ResourceId.Value));
                    if (publisher.Galleries.Contains(existingGallery.Id))
                    {
                        // gallery is already in the User gallery list
                        return Request.CreateResponse(HttpStatusCode.Conflict, "A Gallery with that name already exists, please choose a different name");
                    }
                }
                else
                {
                    return Request.CreateResponse(HttpStatusCode.Conflict, "A Gallery with that name already exists, please choose a different name");
                }
            }
            if (gallery.Resources != null)
            {
                gallery.Resources = Compare.DistinctBy(gallery.Resources, r => r.ResourceId).ToList();
            }
            HttpResponseMessage response = null;
            if (existingGallery == null)
            {
                if (gallery.Admin != null)
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "Gallery Admin cannot be specified");
                }
                if (CurrentUserIdentity.IsInChannelAdminRole())
                {
                    gallery.Admin = CurrentUserIdentity.UserInfo;

                    if (gallery.Metadata.User != null)
                    {
                        var galleryAdmin = db.GetResourceById<User>(gallery.Metadata.User.Id);
                        if (galleryAdmin == null || !db.GetAffiliates(CurrentUserIdentity.Email).Select(a => a.ResourceId).Contains(galleryAdmin.Id))
                        {
                            return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delegate Gallery administration to the specified Admin");
                        }
                        gallery.Metadata.User = new UserInfo(galleryAdmin.Id, galleryAdmin.Metadata.Name);
                    }
                }

                response = Retry.Execute(() => base.Post(gallery));

                // Add the Gallery to the User's list of Galleries
                var galleryWithUser = response.Content.ReadAsAsync<Gallery>();
                galleryWithUser.Wait();
                publisher = Retry.Execute(() => db.GetResourceById<User>(galleryWithUser.Result.Metadata.User.Id));
                publisher.Galleries.Add(galleryWithUser.Result.Id);
            }
            else
            {
                publisher.Galleries.Add(existingGallery.Id);
            }
            try
            {
                Retry.Execute(() => db.UpdateResource(publisher.Id, publisher.Version, new User { Galleries = publisher.Galleries }));
                if (response != null)
                {
                    return response;
                }
                // recreate the response if the gallery has only been added to the User's Galleries list
                response = Request.CreateResponse(HttpStatusCode.Created, existingGallery);
                var apiLink = Url.Link("DefaultApi", new { id = existingGallery.Id });
                if (apiLink != null)
                {
                    response.Headers.Location = new Uri(apiLink);
                }
                return response;
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // PUT api/v1/Gallery
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Put(Guid id, Guid version, Gallery gallery)
        {
            Guid? previousOwner = null;
            if (gallery.Metadata != null)
            {
                if (gallery.Metadata.Name != null)
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "Gallery Metadata.Name cannot be changed");
                }
                if (gallery.Metadata.User != null)
                {
                    var existingGallery = db.GetResourceByIdAndVersion<Gallery>(id, version);
                    if (existingGallery == null)
                    {
                        return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified Gallery does not exist");
                    }
                    if (!CurrentUserIdentity.IsInPyxisAdminRole() 
                        && !(CurrentUserIdentity.IsInAdminRole() && existingGallery.Admin != null && existingGallery.Admin.Id == CurrentUserIdentity.ResourceId))
                    {
                        return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not Authorized to change User for the specified Gallery");
                    }
                    if (CurrentUserIdentity.IsInChannelAdminRole() && !db.GetAffiliates(CurrentUserIdentity.Email).Select(a => a.ResourceId).Contains(gallery.Metadata.User.Id))
                    {
                        return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delegate Gallery administration to the specified Admin");
                    }
                    previousOwner = existingGallery.Metadata.User.Id;
                    var galleryAdmin = db.GetResourceById<User>(gallery.Metadata.User.Id);
                    gallery.Metadata.User = new UserInfo(galleryAdmin.Id, galleryAdmin.Metadata.Name);
                }
            }
            HttpResponseMessage groupResponse;
            if (DisallowGroup(gallery, out groupResponse))
            {
                return groupResponse;
            }
            var response = base.Put(id, version, gallery);
            if (response.StatusCode != HttpStatusCode.OK || previousOwner == null || gallery.Metadata.User.Id == previousOwner.Value)
            {
                return response;
            }
            AddToUserGalleries(Request, gallery.Metadata.User.Id, id, response);
            return RemoveFromUserGalleries(Request, previousOwner.Value, id, response);
        }

        // PUT api/v1/Gallery/{id}/Resource
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPut]
        [Route("{id}/Resource")]
        public HttpResponseMessage AddGalleryResource(Guid id, Guid resourceId)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified gallery");
            }
            if (gallery.Resources.Any(r => r.ResourceId == resourceId))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery already contains the specified resource");
            }

            ChangeResourceGalleryProvider(resourceId, gallery);

            gallery.Resources.Add(new Pyxis.Contract.Publishing.GalleryResource { ResourceId = resourceId });
            return base.Put(id, gallery.Version, new Gallery() { Resources = gallery.Resources });
        }

        // DELETE api/v1/Gallery/{id}/Resource
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpDelete]
        [Route("{id}/Resource")]
        public HttpResponseMessage RemoveGalleryResource(Guid id, Guid resourceId)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified gallery");
            }
            if (!gallery.Resources.Any(r => r.ResourceId == resourceId))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery does not contain the specified resource");
            }

            RemoveResourceGalleryProvider(resourceId);

            gallery.Resources.RemoveAll(r => r.ResourceId == resourceId);
            return base.Put(id, gallery.Version, new Gallery() { Resources = gallery.Resources });
        }

        // PUT api/v1/Gallery/{id}/Feature
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPut]
        [Route("{id}/Feature")]
        public HttpResponseMessage AddGalleryFeature(Guid id, Guid resourceId)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified gallery");
            }
            var item = gallery.Resources.FirstOrDefault(r => r.ResourceId == resourceId);
            if (item == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery does not contain the specified resource");
            }
            if (item.Featured)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery already features the specified resource");
            }

            item.Featured = true;
            return base.Put(id, gallery.Version, new Gallery() { Resources = gallery.Resources });
        }

        // DELETE api/v1/Gallery/{id}/Feature
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpDelete]
        [Route("{id}/Feature")]
        public HttpResponseMessage RemoveGalleryFeature(Guid id, Guid resourceId)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified gallery");
            }
            var item = gallery.Resources.FirstOrDefault(r => r.ResourceId == resourceId);
            if (item == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery does not contain the specified resource");
            }
            if (!item.Featured)
            {
                return Request.CreateResponse(HttpStatusCode.Conflict, "The gallery does not features the specified resource");
            }

            item.Featured = false;
            return base.Put(id, gallery.Version, new Gallery() { Resources = gallery.Resources });
        }

        // DELETE api/v1/Gallery/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Delete(Guid id)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (gallery == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The specified gallery does not exist.");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(gallery))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delete requested resource");
            }
            if (gallery.Resources.Any())
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The gallery must be empty before it can be deleted.");
            }

            try
            {
                db.RemoveResource<Gallery>(id);
                if (gallery.Admin != null)
                {
                    RemoveFromUserGalleries(Request, gallery.Admin.Id, id);
                }
                return RemoveFromUserGalleries(Request, gallery.Metadata.User.Id, id);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // DELETE api/v1/Gallery/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public HttpResponseMessage DeleteAndChangeGalleryProvider(Guid id, Guid newGalleryProvider)
        {
            var gallery = db.GetResourceById<Gallery>(id);
            if (gallery == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The specified gallery does not exist.");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delete requested resource");
            }
            var newProvider = db.GetResourceById<Gallery>(newGalleryProvider);
            if (newProvider == null)
            {
                Request.CreateResponse(HttpStatusCode.BadRequest, "Specified destination gallery does not exist");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() || !ValidateOwnership(newProvider))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to change to the specified new gallery provider");
            }

            try
            {
                foreach (var galleryResource in gallery.Resources)
                {
                    ChangeResourceGalleryProvider(galleryResource.ResourceId, newProvider);
                    newProvider.Resources.Add(new Pyxis.Contract.Publishing.GalleryResource { ResourceId = galleryResource.ResourceId });
                }

                var response = base.Put(newProvider.Id, newProvider.Version, new Gallery() { Resources = newProvider.Resources });
                if (response.StatusCode != HttpStatusCode.OK)
                {
                    return response;
                }
                db.RemoveResource<Gallery>(id);
                if (gallery.Admin != null)
                {
                    RemoveFromUserGalleries(Request, gallery.Admin.Id, id);
                }
                return RemoveFromUserGalleries(Request, gallery.Metadata.User.Id, id);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        private bool DisallowGroup(Gallery gallery, out HttpResponseMessage groupResponse)
        {
            groupResponse = null;
            if (gallery.Groups == null || !gallery.Groups.Any())
            {
                return false;
            }
            var groups = db.GetResourcesByIds<Group>(gallery.Groups.Select(g => g.Id).ToList());
            for (var i = 0; i < gallery.Groups.Count; i++)
            {
                var group = gallery.Groups[i];
                var foundGroup = groups.FirstOrDefault(g => g.Id == group.Id);
                if (foundGroup == null)
                {
                    groupResponse = Request.CreateResponse(HttpStatusCode.BadRequest, "Specified Group with Id " + group.Id  + " does not exist.");
                    return true;
                }
                if (foundGroup.Metadata.User.Id != CurrentUserIdentity.ResourceId.Value)
                {
                    groupResponse = Request.CreateResponse(HttpStatusCode.BadRequest, "Not authorized to assign Gallery Group " + foundGroup.Metadata.Name + " to the specified Gallery.");
                    return true;
                }
                // recreate GroupInfo to overwrite user-specified non-Id fields 
                gallery.Groups[i] = new GroupPermissionInfo(foundGroup, gallery.Groups[i].Permission);
            }
            return false;
        }

        private void ChangeResourceGalleryProvider(Guid resourceId, Gallery gallery)
        {
            var resource = db.GetResourceById(resourceId);
            resource.Metadata.Providers.RemoveAll(x => x.Type == Pyxis.Contract.Publishing.ResourceType.Gallery);
            resource.Metadata.Providers.Add(new Pyxis.Contract.Publishing.Provider { Type = Pyxis.Contract.Publishing.ResourceType.Gallery, Name = gallery.Metadata.Name, Id = gallery.Id });
            UpdateResourceGalleryProvider(resource);
        }

        private void RemoveResourceGalleryProvider(Guid resourceId)
        {
            var resource = db.GetResourceById(resourceId);
            resource.Metadata.Providers.RemoveAll(x => x.Type == Pyxis.Contract.Publishing.ResourceType.Gallery);
            UpdateResourceGalleryProvider(resource);
        }

        private HttpResponseMessage AddToUserGalleries(HttpRequestMessage request, Guid userId, Guid galleryId, HttpResponseMessage response = null)
        {
            var userResource = Retry.Execute(() => db.GetResourceById<User>(userId));
            var userUpdates = new User();
            var numGalleries = userResource.Galleries.Count;
            userUpdates.Galleries = new List<Guid>(userResource.Galleries);
            userUpdates.Galleries.Add(galleryId);
            userUpdates.Galleries = userUpdates.Galleries.Distinct().ToList();
            if (numGalleries != userUpdates.Galleries.Count)
            {
                Retry.Execute(() => db.UpdateResource(userResource.Id, userResource.Version, userUpdates));
            }
            return response ?? request.CreateResponse(HttpStatusCode.OK);
        }

        private HttpResponseMessage RemoveFromUserGalleries(HttpRequestMessage request, Guid userId, Guid galleryId, HttpResponseMessage response = null)
        {
            var userResource = Retry.Execute(() => db.GetResourceById<User>(userId));
            if (userResource == null)
            {
                return request.CreateResponse(HttpStatusCode.BadRequest, "Unable to remove Gallery from User's Galleries - specified User does not exist");
            }

            var userUpdates = new User();
            userUpdates.Galleries = new List<Guid>(userResource.Galleries);
            var numRemoved = userUpdates.Galleries.RemoveAll(g => g == galleryId);
            if (numRemoved > 0)
            {
                Retry.Execute(() => db.UpdateResource(userResource.Id, userResource.Version, userUpdates));
            }
            return response ?? request.CreateResponse(HttpStatusCode.OK);
        }

        private void UpdateResourceGalleryProvider(Resource resource)
        {
            dynamic updates;
            switch (resource.Type)
            {
                case Pyxis.Contract.Publishing.ResourceType.GeoSource:
                    updates = new GeoSource();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.Map:
                    updates = new Map();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.User:
                    updates = new User();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.Group:
                    updates = new Group();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.License:
                    updates = new License();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.Gallery:
                    updates = new Gallery();
                    break;
                case Pyxis.Contract.Publishing.ResourceType.Product:
                    updates = new Product();
                    break;
                default:
                    throw new NotImplementedException("Changing the gallery provider for a " + resource.Type.ToString() + " is not implemented");
            }
            updates.Metadata = new Pyxis.Contract.Publishing.Metadata
                {
                    Providers = resource.Metadata.Providers
                };
            Retry.Execute(db.UpdateResource(resource.Id, resource.Version, updates));
        }
    }
}