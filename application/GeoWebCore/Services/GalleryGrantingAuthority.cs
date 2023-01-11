using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Jose;
using Newtonsoft.Json;
using Pyxis.Publishing;
using Pyxis.Publishing.Permits;

namespace GeoWebCore.Services
{
    /// <summary>
    /// Authority for issuing and validating tokens for granting access to gallery files.
    /// </summary>
    public static class GalleryGrantingAuthority
    {
        // 256 bit secret
        private static readonly byte[] s_secretKey =
        {
            163, 86, 207, 7, 22, 92, 75, 143, 140, 37, 13, 221, 115, 66, 69,
            181, 51, 174, 50, 216, 233, 192, 99, 253, 45, 153, 97, 185, 243, 226, 188, 3
        };

        /// <summary>
        /// Issue a token for the provided user to the specified gallery.
        /// Authorization to the gallery is verified.
        /// The user's permission to publish to the Gallery granted authorization to may change before expiration of the issued token. 
        /// The issued token would continue to grant permission until expired in that case, even though they lost permission on the license server. 
        /// </summary>
        /// <param name="user">Authorized user.</param>
        /// <param name="userId">Id of the requesting User.</param>
        /// <param name="galleryId">Id of the Gallery requesting authorization to.</param>
        /// <returns>An AccessToken.TokenDetails object authorizing all claims to the gallery specified by <paramref name="galleryId"/>.</returns>
        /// <exception cref="Exception">If the user is not authorized to publish to the gallery <paramref name="galleryId"/>.</exception>
        public static AccessToken.TokenDetails Issue(User user, string userId, string galleryId)
        {
            if (!IsAuthorized(user, galleryId))
            {
                throw new Exception("Not authorized to publish to the specified Gallery.");
            }

            // use the same expiration as the LS token (may be desirable to have shorter expiration to reflect the ability of the underlying permission to publish to a gallery changing)
            var tokenDetails = user.GetTokenDetails();
            var payload = new Dictionary<string, object>
            {
                {"userId", userId},
                {"galleryId", galleryId},
                {"userToken", tokenDetails.Token}
            };
            string token = JWT.Encode(payload, s_secretKey, JweAlgorithm.A256GCMKW, JweEncryption.A256CBC_HS512);
            tokenDetails.Token = token;

            return tokenDetails;
        }

        /// <summary>
        /// Validate the provided token was issued by the token authority.
        /// </summary>
        /// <param name="token">Token to validate.</param>
        /// <returns>Dictionary of properties in the token.</returns>
        /// <exception cref="Exception"><paramref name="token"/> cannot be validated.</exception>
        public static Dictionary<string, object> Validate(string token)
        {
            string json = JWT.Decode(token, s_secretKey);
            return JsonConvert.DeserializeObject<Dictionary<string, object>>(json);
        }

        private static bool IsAuthorized(User user, string galleryId)
        {
            return user.GetGalleries().Any(g => g.Id == new Guid(galleryId));
        }
    }
}
