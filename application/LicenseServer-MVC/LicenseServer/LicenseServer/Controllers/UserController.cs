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
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Models.Mongo.Interface;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using Gallery = LicenseServer.Models.Mongo.Gallery;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;
using Map = LicenseServer.Models.Mongo.Map;
using User = LicenseServer.Models.Mongo.User;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/User")]
    public class UserController : BaseResourceController<User>
    {
        public UserController() 
        { }

        // Inject for test
        public UserController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/User/Available
        [HttpGet]
        [Route("Available")]
        public HttpResponseMessage Available(string userName)
        {
            if (db.GetResourcesByName<User>(userName, MongoStringCompareOptions.CaseInsensitive).FirstOrDefault() == null)
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            return Request.CreateResponse(HttpStatusCode.Conflict);
        }

        // GET api/v1/User/Delegates
        [Authorize(Roles = PyxisIdentityRoleGroups.AllAdmins)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("Delegates")]
        public HttpResponseMessage GetDelegates()
        {
            return Request.CreateResponse(db.GetAffiliates(CurrentUserIdentity.Email));
        }

        // Get api/v1/User/5/GeoSources
        [HttpGet]
        [Route("{id}/GeoSources")]
        public PageResult<dynamic> GetGeoSources(Guid id, ODataQueryOptions<GeoSource> options)
        {
            return db.GetResourcesByUserId<GeoSource>(id).ToPageResult(Request, options);
        }

        // Get api/v1/User/5/Maps
        [HttpGet]
        [Route("{id}/Maps")]
        public PageResult<dynamic> GetMaps(Guid id, ODataQueryOptions<Map> options)
        {
            return db.GetResourcesByUserId<Map>(id).ToPageResult(Request, options);
        }

        // Get api/v1/User/5/Galleries
        [HttpGet]
        [Route("{id}/Galleries")]
        public PageResult<dynamic> GetGalleries(Guid id, ODataQueryOptions<Gallery> options)
        {
            return db.GetResourcesByUserId<Gallery>(id).ToPageResult(Request, options);
        }

        // Get api/v1/User/5/Licenses
        [HttpGet]
        [Route("{id}/Licenses")]
        public PageResult<dynamic> GetLicenses(Guid id, ODataQueryOptions<License> options)
        {
            return db.GetResourcesByUserId<License>(id).ToPageResult(Request, options);
        }

        // Get api/v1/User/Licenses
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("Licenses")]
        public PageResult<dynamic> GetLicenses(ODataQueryOptions<License> options)
        {
            RequireCompletedProfile();

            return db.GetResourcesByUserId<License>(CurrentUserIdentity.ResourceId.Value).ToPageResult(Request, options);
        }

        // Get api/v1/User/5/Profile
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("{id}/Profile")]
        [ResponseType(typeof(UserProfile))]
        public HttpResponseMessage GetProfile(Guid id)
        {
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            if (CurrentUserIdentity.ResourceId.Value != id && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to access requested profile");
            }
            return GenerateProfileResponse(Request, CurrentUserIdentity);
        }

        // Get api/v1/User/Profile
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("Profile")]
        [ResponseType(typeof(UserProfile))]
        public HttpResponseMessage GetProfile()
        {
            // Added in order to recover an incomplete profile (account missing a User resource Id)
            if (!CurrentUserIdentity.ResourceId.HasValue && CurrentUserIdentity.ExternalLoginProvider == null)
            {
                var id = Guid.NewGuid();
                Post(new User
                {
                    Id = id,
                    Metadata = new Metadata
                    {
                        Name = CurrentUserIdentity.UserName
                    }
                });
            }
            return GenerateProfileResponse(Request, CurrentUserIdentity);
        }

        // Get api/v1/User/5/Quota
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("{id}/Quota")]
        [ResponseType(typeof(long))]
        public HttpResponseMessage GetQuota(Guid id)
        {
            RequireCompletedProfile();
            if (CurrentUserIdentity.ResourceId.Value != id && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to access requested storage amount");
            }
            return Request.CreateResponse(HttpStatusCode.OK, GetUserStorageLimit(id));
        }

        // Get api/v1/User/Quota
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("Quota")]
        [ResponseType(typeof(long))]
        public HttpResponseMessage GetQuota()
        {
            RequireCompletedProfile();
            return Request.CreateResponse(HttpStatusCode.OK, GetUserStorageLimit());
        }

        // Get api/v1/User/5/Storage
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("{id}/Storage")]
        [ResponseType(typeof(long))]
        public HttpResponseMessage GetStorage(Guid id)
        {
            RequireCompletedProfile();
            if (CurrentUserIdentity.ResourceId.Value != id && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to access requested storage amount");
            }
            return Request.CreateResponse(HttpStatusCode.OK, db.GetStorageByUserId(id));
        }

        // Get api/v1/User/Storage
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpGet]
        [Route("Storage")]
        [ResponseType(typeof(long))]
        public HttpResponseMessage GetStorage()
        {
            RequireCompletedProfile();
            return Request.CreateResponse(HttpStatusCode.OK, db.GetStorageByUserId(CurrentUserIdentity.ResourceId.Value));
        }

        // POST api/v1/User
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(User user)
        {
            if (CurrentUserIdentity == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Invalid access token, try acquiring a new token from Account/ExternalLogin");
            }
            if (CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "User resource has already been created for this account");
            }
            if (CurrentUserIdentity.Logins.Any())
            {
                user.Metadata = new Metadata { Name = CurrentUserIdentity.UserName };
            }
            if (user.Metadata == null || user.Metadata.Name == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Resource.Metadata.Name must be specified");
            }
            if (db.GetResourcesByName<User>(user.Metadata.Name).FirstOrDefault() != null)
            {
                return Request.CreateResponse(HttpStatusCode.Conflict, "UserName is already taken, please choose a different name");
            }
            if (user.Id == Guid.Empty)
            {
                user.Id = Guid.NewGuid();
            }
            user.UserType = UserType.Member;
            // Connect the user's identity to the resource
            CurrentUserIdentity.ResourceId = user.Id;
            CurrentUserIdentity.ProfileName = user.Metadata.Name;
            var updateSucceeded = false;
            Retry.Execute(() =>
            {
                UpdateUserIdentity(CurrentUserIdentity);
                updateSucceeded = true;
            });
            if (!updateSucceeded)
            {
                throw new Exception("Unable to update user account information");
            }
            user.Metadata.User = new UserInfo(user);

            return Retry.Execute(() => base.Post(user));
        }

        // PUT api/v1/User
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Put(Guid id, Guid version, User user)
        {
            if (user.Metadata != null && user.Metadata.Name != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "User Metadata.Name cannot be changed.");
            }
            if (user.Groups != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Use Groups API for adding and removing Groups");
            }
            return base.Put(id, version, user);
        }

        // DELETE api/v1/User/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Delete(Guid id)
        {
            if (!CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delete requested resource");
            }

            try
            {
                var userObject = db.GetResourceById<User>(id);

                if (userObject == null)
                {
                    return Request.CreateResponse(HttpStatusCode.NotFound);
                }

                if (db.GetResourcesByUserId<GeoSource>(id).Any() || db.GetResourcesByUserId<Map>(id).Any())
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest,"User has GeoSources/Maps and therefore can't be removed");
                }

                var accountControler = new AccountController();

                var userToDelete = accountControler.UserManager.FindByName(userObject.Metadata.Name);

                if (userToDelete != null)
                {
                    accountControler.UserManager.Delete(userToDelete);
                }

                //remove all galleries
                foreach (var gallery in userObject.Galleries)
                {
                    //remove all user galleries
                    if (db.GetResourceById<Gallery>(gallery) != null)
                    {
                        db.RemoveResource<Gallery>(gallery);    
                    }
                }

                //remove object
                db.RemoveResource<User>(id);
                
                
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        // PUT api/v1/User/5/Profile
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPut]
        [Route("{id}/Profile")]
        public virtual HttpResponseMessage UpdateProfile(Guid id, UserProfile userProfile)
        {
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            if (CurrentUserIdentity.ResourceId.Value != id && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to access requested profile");
            }
            if (userProfile.Metadata != null && userProfile.Metadata.Name != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "User Metadata.Name cannot be changed.");
            }

            return UpdateProfile(userProfile, CurrentUserIdentity);
        }

        // PUT api/v1/User/Profile
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPut]
        [Route("Profile")]
        public virtual HttpResponseMessage UpdateProfile(UserProfile userProfile)
        {
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }
            if (userProfile.Metadata != null && userProfile.Metadata.Name != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "User Metadata.Name cannot be changed.");
            }

            return UpdateProfile(userProfile, CurrentUserIdentity);
        }

        private HttpResponseMessage UpdateProfile(UserProfile userProfile, PyxisIdentityUser currentUserIdentity)
        {
            if (userProfile.Email != null 
                || userProfile.EmailConfirmed.HasValue 
                || userProfile.AcceptTerms.HasValue 
                || userProfile.AccountConsent.HasValue 
                || userProfile.PromotionConsent.HasValue)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Email, EmailConfirmed, AcceptTerms, AccountConsent, and PromotionConsent must be updated using the Account controller.");
            }
            var user = userProfile.ExtractUser();
            var identity = userProfile.ExtractIdentity();

            // Update User Identity
            MergeIdentityChanges(currentUserIdentity, identity);
            if (!UpdateUserIdentity(currentUserIdentity))
            {
                return Request.CreateResponse(HttpStatusCode.InternalServerError, "Unable to update user account information");
            }

            // Update User Resource
            try
            {
                var latestResource = db.GetResourceById<User>(currentUserIdentity.ResourceId.Value);
                db.UpdateResource(latestResource.Id, latestResource.Version, user);
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        private void MergeIdentityChanges(PyxisIdentityUser currentUserIdentity, PyxisIdentityUser identity)
        {
            if (identity.Country != null) { currentUserIdentity.Country = identity.Country; }
            if (identity.City != null) { currentUserIdentity.City = identity.City; }
            if (identity.ProfileName != null) { currentUserIdentity.ProfileName = identity.ProfileName; }
            if (identity.BusinessName != null) { currentUserIdentity.BusinessName = identity.BusinessName; }
            if (identity.CollectTax.HasValue) { currentUserIdentity.CollectTax = identity.CollectTax; }
            if (identity.PayPalEmail != null) { currentUserIdentity.PayPalEmail = identity.PayPalEmail; }
        }

        private HttpResponseMessage GenerateProfileResponse(HttpRequestMessage request, PyxisIdentityUser identity)
        {
            if (identity.ResourceId == null)
            {
                return request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }

            var userResource = Retry.Execute(() => db.GetResourceById<User>(identity.ResourceId.Value));
            if (userResource == null)
            {
                return request.CreateResponse(HttpStatusCode.BadRequest, "Complete user profile before requesting");
            }

            var userProfile = UserProfileFactory.Create(userResource, identity);
            return request.CreateResponse(HttpStatusCode.OK, userProfile);
        }
    }
}