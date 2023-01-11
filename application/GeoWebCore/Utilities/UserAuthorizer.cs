using System;
using System.Net.Http.Headers;
using System.Web.Http;
using System.Web.Http.Controllers;
using Pyxis.Publishing.Protocol;
using GeoWebCore.Services;
using GeoWebCore.WebConfig;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Custom authorize attribute [LsAuthorizeUser] for authorizing Http requests. Returns the user id
    /// in ActionArguments["UserId"].
    /// </summary>
    public class LsAuthorizeUserAttribute : AuthorizeAttribute
    {
        /// <summary>
        /// Verify that usr is authrized for this request. if user is authrize, the user Id is stored at actionContext.ActionArguments["UserId"]
        /// </summary>
        /// <param name="actionContext">actionContext to verify</param>
        /// <returns>return true if user autherized</returns>
        protected override bool IsAuthorized(HttpActionContext actionContext)
        {
            var userId = UserAuthorizer.Authorize(actionContext.Request.Headers);
            if (userId != null)
            {
                actionContext.ActionArguments[GlobalActionArgument.UserId] = userId;
                return true;
            }

            return false;
        }
    }

}
