using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PyxisCLI.Server.WebConfig.AuthenticationProvider
{
    class GuestAuthenticationProvider : IAuthenticationProvider
    {
        public async Task<TokenValidationResults> ValidateToken(string token)
        {
            return new TokenValidationResults {UserId = "guest", Valid = true};
        }

        public async Task<UserProfile> GetUserProfile(string token)
        {
            return new UserProfile()
            {
                Name = "guest",
                Id = "guest"
            };
        }
    }
}
