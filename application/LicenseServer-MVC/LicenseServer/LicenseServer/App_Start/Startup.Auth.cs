using System;
using System.Configuration;
using System.Security.Claims;
using System.Threading.Tasks;
using LicenseServer.App_Utilities;
using LicenseServer.Models.Mongo;
using LicenseServer.Providers;
using Microsoft.AspNet.Identity;
using Microsoft.AspNet.Identity.Owin;
using Microsoft.Owin;
using Microsoft.Owin.Cors;
using Microsoft.Owin.Security.Cookies;
using Microsoft.Owin.Security.DataProtection;
using Microsoft.Owin.Security.Facebook;
using Microsoft.Owin.Security.Google;
using Microsoft.Owin.Security.OAuth;
using Owin;
using Owin.Security.Providers.LinkedIn;

namespace LicenseServer
{
    public partial class Startup
    {
        public static OAuthAuthorizationServerOptions OAuthOptions { get; private set; }

        public static Func<UserManager<PyxisIdentityUser>> UserManagerFactory { get; set; }

        public static string PublicClientId { get; private set; }

        // For more information on configuring authentication, please visit http://go.microsoft.com/fwlink/?LinkId=301864
        public void ConfigureAuth(IAppBuilder app)
        {
            PublicClientId = "Pyxis.LicenseServer";
            
            var provider = app.GetDataProtectionProvider();
            UserManagerFactory = () =>
            {
                var userManager = new UserManager<PyxisIdentityUser>(new EmailUserStore<PyxisIdentityUser>("pyxis_licenseserverMongo"));
                
                userManager.EmailService = new SendEmailService();

                userManager.UserTokenProvider = new DataProtectorTokenProvider<PyxisIdentityUser>(provider.Create("PasswordReset"))
                {
                    TokenLifespan = TimeSpan.FromHours(1)
                };
                return userManager;
            };

            OAuthOptions = new OAuthAuthorizationServerOptions
            {
                TokenEndpointPath = new PathString("/api/v1/Token"),
                Provider = new ApplicationOAuthProvider(PublicClientId, UserManagerFactory),
                AuthorizeEndpointPath = new PathString("/api/v1/Account/ExternalLogin"),
                AccessTokenExpireTimeSpan = TimeSpan.FromDays(14),
                AllowInsecureHttp = true
            };

            app.UseCors(CorsOptions.AllowAll);
            
            // Enable the application to use a cookie to store information for the signed in user
            // and to use a cookie to temporarily store information about a user logging in with a third party login provider
            app.UseCookieAuthentication(new CookieAuthenticationOptions());
            app.UseExternalSignInCookie(DefaultAuthenticationTypes.ExternalCookie);

            // Enable the application to use bearer tokens to authenticate users
            app.UseOAuthBearerTokens(OAuthOptions);

            // Uncomment the following lines to enable logging in with third party login providers
            //app.UseMicrosoftAccountAuthentication(
            //    clientId: "",
            //    clientSecret: "");

            //app.UseTwitterAuthentication(
            //    consumerKey: ConfigurationManager.AppSettings["Twitter.ConsumerKey"],
            //    consumerSecret: ConfigurationManager.AppSettings["Twitter.ConsumerSecret"]);
            
            app.UseGoogleAuthentication(new GoogleOAuth2AuthenticationOptions
            {
                ClientId = ConfigurationManager.AppSettings["Google.ClientId"],
                ClientSecret = ConfigurationManager.AppSettings["Google.ClientSecret"],
                Provider = new GoogleOAuth2AuthenticationProvider
                {
                    OnAuthenticated = (context) =>
                    {
                        context.Identity.AddClaim(new Claim("urn:google:access_token", context.AccessToken, ClaimValueTypes.String, "Google"));
                        context.Identity.AddClaim(new Claim("urn:google:email", context.Identity.FindFirstValue(ClaimTypes.Email)));
                        context.Identity.AddClaim(new Claim("urn:google:first_name", context.Identity.FindFirstValue(ClaimTypes.GivenName)));
                        context.Identity.AddClaim(new Claim("urn:google:last_name", context.Identity.FindFirstValue(ClaimTypes.Surname)));
                        return Task.FromResult(0);
                    }
                },
                Scope = { "profile", "email" }
            });

            app.UseLinkedInAuthentication(new LinkedInAuthenticationOptions
            {
                ClientId = ConfigurationManager.AppSettings["LinkedIn.ClientId"],
                ClientSecret = ConfigurationManager.AppSettings["LinkedIn.ClientSecret"],
                Provider = new LinkedInAuthenticationProvider
                {
                    OnAuthenticated = (context) =>
                    {
                        context.Identity.AddClaim(new Claim("urn:linkedin:access_token", context.AccessToken, ClaimValueTypes.String, "LinkedIn"));

                        foreach (var claim in context.User)
                        {
                            var claimType = string.Format("urn:linkedin:{0}", claim.Key);
                            claimType = claimType.Replace("emailAddress", "email").Replace("firstName", "first_name").Replace("lastName", "last_name");
                            string claimValue = claim.Value.ToString();
                            if (!context.Identity.HasClaim(claimType, claimValue))
                            {
                                context.Identity.AddClaim(new Claim(claimType, claimValue, "XmlSchemaString", "LinkedIn"));
                            }
                        }
                        return Task.FromResult(0);
                    }
                }
            });

            app.UseFacebookAuthentication(new FacebookAuthenticationOptions
            {
                AppId = ConfigurationManager.AppSettings["Facebook.AppId"],
                AppSecret = ConfigurationManager.AppSettings["Facebook.AppSecret"],
                Provider = new FacebookAuthenticationProvider
                {
                    OnAuthenticated = (context) =>
                    {
                        context.Identity.AddClaim(new Claim("urn:facebook:access_token", context.AccessToken, ClaimValueTypes.String, "Facebook"));
                        foreach (var claim in context.User)
                        {
                            var claimType = string.Format("urn:facebook:{0}", claim.Key);
                            string claimValue = claim.Value.ToString();
                            if (!context.Identity.HasClaim(claimType, claimValue))
                            {
                                context.Identity.AddClaim(new Claim(claimType, claimValue, "XmlSchemaString", "Facebook"));
                            }
                        }
                        return Task.FromResult(0);
                    }
                },
                Scope = { "public_profile", "email" }
            });
        }
    }
}
