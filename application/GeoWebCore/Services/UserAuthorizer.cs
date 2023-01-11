using Pyxis.Publishing.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Services
{
    /// <summary>
    /// Authorizes Http requests based on the Bearer token in the Authorization header.
    /// </summary>
    public static class UserAuthorizer
    {
        /// <summary>
        /// The URL to the license server.
        /// </summary>
        public static string LicenseServerUrl { get; set; }

        /// <summary>
        /// Verify that a user is authorized for an operation based on an HttpRequestMessage authorization token.
        /// </summary>
        /// <param name="headers">Headers from the incoming Http request.</param>
        /// <returns>The user id or null if the user cannot be identified from the authorization token.</returns>
        public static string Authorize(HttpRequestHeaders headers)
        {
            var userClient = new UserClient(LicenseServerUrl, headers);
            var guid = userClient.GetUserId();

            if (guid == Guid.Empty)
            {
                return null;
            }

            return guid.ToString();
        }
    }
}
