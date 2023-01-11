using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http.OData.Query;
using Microsoft.AspNet.Identity;
using LicenseServer.Models.Mongo;
using System.Net;
using System.Threading.Tasks;
using LicenseServer.Models.Mongo.Interface;

namespace LicenseServer.Controllers
{
    // Cross-Origin Resource Sharing standard compliant controller
    public class CORSMongoApiController : TestableMongoApiController
    {
        private class DataLayerAuthorizedUser : IAuthorizedUser
        {
            public Guid Id { get; set; }
            public bool IsAdmin { get; set; }
            public bool IsPyxisAdmin { get; set; }
        }

        public PyxisIdentityUser CurrentUserIdentity { get; private set; }

        public CORSMongoApiController() 
        {
            if (User.Identity.IsAuthenticated)
            {
                var currentUserIdentityTask = GetUserIdentityAsync();
                currentUserIdentityTask.Wait();
                CurrentUserIdentity = currentUserIdentityTask.Result;
                if (CurrentUserIdentity.ResourceId.HasValue)
                {
                    db.SetAuthorizedUser(new DataLayerAuthorizedUser
                    {
                        Id = CurrentUserIdentity.ResourceId.Value,
                        IsAdmin = User.IsInRole(PyxisIdentityRoles.ChannelAdmin) || User.IsInRole(PyxisIdentityRoles.Admin) || User.IsInRole(PyxisIdentityRoles.SiteAdmin),
                        IsPyxisAdmin = User.IsInRole(PyxisIdentityRoles.Admin) || User.IsInRole(PyxisIdentityRoles.SiteAdmin)
                    });
                }
            }
        }

        // Inject for test
        public CORSMongoApiController(TestMongoSetup setup) :
            base(setup)
        {
            CurrentUserIdentity = setup.CurrentUserIdentity;
        }

        // Handle the OPTIONS method (http://www.w3.org/TR/cors/#preflight-request)
        // The cross site attribute will add necessary headers
        public string OptionsHandler()
        {
            return null; // HTTP 200 response with empty body
        }

        protected bool UpdateUserIdentity(PyxisIdentityUser userIdentity)
        {
            var manager = Startup.UserManagerFactory();
            var updateTask = manager.UpdateAsync(userIdentity);
            updateTask.Wait();
            return updateTask.Result.Succeeded;
        }

        protected void RequireCompletedProfile()
        {
            if (!CurrentUserIdentity.ResourceId.HasValue)
            {
                throw new HttpException((int)HttpStatusCode.BadRequest, "Completion of this request requires a complete user profile for authorization.  " +
                    "Please complete your user profile to activate this feature.");
            }
        }

        protected object GetSiteParameter(string key)
        {
            var value = db.GetParameter(key);
            if(value == null)
            {
                throw new HttpException((int)HttpStatusCode.InternalServerError, "Site parameter " + key + " was not found.");
            }
            return value;
        }

        protected long GetUserStorageLimit()
        {
            return GetUserStorageLimit(CurrentUserIdentity.ResourceId.Value);
        }

        protected long GetUserStorageLimit(Guid userId)
        {
            var user = db.GetResourceById<User>(userId);
            return (long)GetSiteParameter(user.UserType + "-StorageLimit");
        }

        private async Task<PyxisIdentityUser> GetUserIdentityAsync()
        {
            var manager = Startup.UserManagerFactory();
            return await RetryAsync.ExecuteAsync(() => manager.FindByIdAsync(User.Identity.GetUserId()));
        }
    }
}