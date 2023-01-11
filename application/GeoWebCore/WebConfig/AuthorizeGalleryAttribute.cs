using GeoWebCore.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using System.Web.Http.Controllers;

namespace GeoWebCore.WebConfig
{
    /// <summary>
    /// Custom authorize attribute [LsAuthorizeGallery] for authorizing Http requests. Returns the user and gallery id
    /// in ActionArguments["UserId"] and ActionArguments["GalleryId"].
    /// </summary>
    public class AuthorizeGalleryAttribute : AuthorizeAttribute
    {
        /// <summary>
        /// Returns true if the provided Authorization header can be validated by the issuing authority.
        /// The authorized gallery is specified in the GalleryId ActionArguments and should be verified against user-specified gallery Ids.
        /// </summary>
        /// <param name="actionContext">Current action context.</param>
        /// <returns>True if the Authorization token can be validated, otherwise false.</returns>
        protected override bool IsAuthorized(HttpActionContext actionContext)
        {
            var token = actionContext.Request.Headers.Authorization.Parameter;
            try
            {
                var payload = GalleryGrantingAuthority.Validate(token);
                actionContext.ActionArguments[GlobalActionArgument.UserId] = payload["userId"];
                actionContext.ActionArguments[GlobalActionArgument.GalleryId] = payload["galleryId"];
                actionContext.ActionArguments[GlobalActionArgument.UserToken] = payload["userToken"];
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}
