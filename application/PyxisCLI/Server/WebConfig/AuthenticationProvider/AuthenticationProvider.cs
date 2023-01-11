using System.Threading.Tasks;

namespace PyxisCLI.Server.WebConfig.AuthenticationProvider
{
    public class TokenValidationResults
    {
        public bool Valid { get; set; }
        public string UserId { get; set; }

        public static TokenValidationResults NotAuthorized
        {
            get
            {
                return new TokenValidationResults() { Valid = false };
            }
        }
    }

    public class UserProfile
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Email { get; set; }
    }

    public interface IAuthenticationProvider
    {
        Task<TokenValidationResults> ValidateToken(string token);
        Task<UserProfile> GetUserProfile(string token);
    }
}
