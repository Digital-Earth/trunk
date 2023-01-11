using System;
using System.Security.Principal;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Http.Filters;
using PyxisCLI.Server.WebConfig.AuthenticationProvider;

namespace PyxisCLI.Server.WebConfig
{
    /// <summary>
    /// Generic IAuthenticationFilter that can be linked with IAuthenticationProvider 
    /// </summary>
    public class PlugableAuthenticationAttribute : Attribute, IAuthenticationFilter
    {
        private static IAuthenticationProvider s_provider;

        public bool AllowMultiple { get { return false; } }

        public PlugableAuthenticationAttribute(IAuthenticationProvider provider)
        {
            s_provider = provider;
        }

        public async Task AuthenticateAsync(HttpAuthenticationContext context, CancellationToken cancellationToken)
        {
            string token = null;
            if (context.Request.Headers.Authorization != null)
            {
                token = context.Request.Headers.Authorization.Parameter;
            }

            try
            {
                var result = await s_provider.ValidateToken(token);
                if (result != null && result.Valid)
                {
                    context.Principal = new GenericPrincipal(new GenericIdentity(result.UserId), new string[0]);
                }
            }
            catch (Exception)
            {
            }
        }

        public async Task ChallengeAsync(HttpAuthenticationChallengeContext context, CancellationToken cancellationToken)
        {
            return;
        }
    }
}