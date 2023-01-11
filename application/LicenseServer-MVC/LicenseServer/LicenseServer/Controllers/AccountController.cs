using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Security.Claims;
using System.Security.Cryptography;
using System.Threading.Tasks;
using System.Web;
using System.Web.Http;
using System.Web.Http.ModelBinding;
using Microsoft.AspNet.Identity;
using Microsoft.Owin.Security;
using Microsoft.Owin.Security.Cookies;
using Microsoft.Owin.Security.OAuth;
using LicenseServer.Models;
using LicenseServer.Providers;
using LicenseServer.Results;
using MongoDB.AspNet.Identity;
using LicenseServer.Models.Mongo;
using System.Net;
using System.Net.Sockets;
using System.Security.Principal;
using System.Text;
using AutoMapper;
using LicenseServer.App_Utilities;
using Microsoft.AspNet.Identity.Owin;
using System.Text.RegularExpressions;
using System.Web.Hosting;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using System.Web.Http.Results;
using LicenseServer.Extensions;
using MongoDB.Driver;
using Newtonsoft.Json;
using Polly;

namespace LicenseServer.Controllers
{
    [Authorize]
    [RoutePrefix("api/v1/Account")]
    public class AccountController : ApiController
    {
        private MongoDBEntities db;

        private const string LocalLoginProvider = "Local";
        private static readonly TimeSpan s_resetExpirationTime = TimeSpan.FromHours(1);
        private static readonly ReadOnlyDictionary<string, string> s_emailTemplates;
        private static readonly ReadOnlyDictionary<string, JwtSettings> s_jwtSettings;

        private Policy CreateRetryPolicy()
        {
            var log = Elmah.ErrorSignal.FromCurrentContext();
            return Policy.Handle<Exception>()
                .WaitAndRetryAsync(3, retryAttempt => TimeSpan.FromSeconds(Math.Pow(2, retryAttempt)),
                    (exception, retrySleep) =>
                    {
                        var exceptionToLog = new Exception("Retrying at " + DateTime.UtcNow + " sleep: " + retrySleep,
                            exception);
                        log.Raise(exceptionToLog);
                    });
        }

        protected Policy Retry
        {
            get
            {
                //we create a new policy every time to make sure HttpContext is setup correctly
                return CreateRetryPolicy();
            }
        }

        static AccountController()
        {
            Mapper.CreateMap<RegisterBindingModel, PyxisIdentityUser>();
            Mapper.CreateMap<ExternalLoginData, PyxisIdentityUser>()
                .ForMember(x => x.Claims, opt => opt.Ignore());
            Mapper.CreateMap<RegisterExternalBindingModel, PyxisIdentityUser>();
            var emailTemplates = new Dictionary<string, string>();
            emailTemplates["confirm-email"] =
                System.IO.File.ReadAllText(HostingEnvironment.MapPath(@"~\Content\email_templates\confirm-email.html"));
            emailTemplates["forgot-password"] =
                System.IO.File.ReadAllText(HostingEnvironment.MapPath(@"~\Content\email_templates\forgot-password.html"));
            s_emailTemplates = new ReadOnlyDictionary<string, string>(emailTemplates);
            var jwtSettings = new Dictionary<string, JwtSettings>
            {
                {
                    "zendesk",
                    new JwtSettings
                    {
                        Key = Properties.Settings.Default.ZendeskJwtKey,
                        Url = Properties.Settings.Default.ZendeskJwtAccessUrl
                    }
                }
            };
            s_jwtSettings = new ReadOnlyDictionary<string, JwtSettings>(jwtSettings);
        }

        public AccountController()
            : this(Startup.UserManagerFactory(), Startup.OAuthOptions.AccessTokenFormat)
        {
        }

        public AccountController(UserManager<PyxisIdentityUser> userManager,
            ISecureDataFormat<AuthenticationTicket> accessTokenFormat)
        {
            db = new MongoDBEntities();
            UserManager = userManager;
            AccessTokenFormat = accessTokenFormat;
        }

        public UserManager<PyxisIdentityUser> UserManager { get; private set; }
        public ISecureDataFormat<AuthenticationTicket> AccessTokenFormat { get; private set; }

        // GET api/v1/Account/UserInfo
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [Route("UserInfo")]
        public UserInfoViewModel GetUserInfo()
        {
            ExternalLoginData externalLogin = ExternalLoginData.FromIdentity(User.Identity as ClaimsIdentity);

            var isExternalLogin = externalLogin != null;
            // set email claim to null for local login, true for external login with a claim to email, false for external login with no email claim
            bool? emailClaim = null;
            if (isExternalLogin)
            {
                emailClaim = externalLogin.Email != null;
            }
            return new UserInfoViewModel
            {
                UserName = User.Identity.GetUserName(),
                HasRegistered = !isExternalLogin,
                LoginProvider = isExternalLogin ? externalLogin.LoginProvider : null,
                EmailClaim = emailClaim
            };
        }

        // GET api/v1/Account/Jwt?application={application}
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [Route("Jwt")]
        public async Task<HttpResponseMessage> GetJwt(string application)
        {
            application = application.ToLower();
            JwtSettings jwtSettings;
            if (!s_jwtSettings.TryGetValue(application, out jwtSettings))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Provided application is not supported");
            }

            ExternalLoginData externalLogin = ExternalLoginData.FromIdentity(User.Identity as ClaimsIdentity);
            if (externalLogin != null)
            {
                return Request.CreateErrorResponse(HttpStatusCode.Forbidden, "Complete registration before requesting.");
            }

            var identity = await UserManager.FindByIdAsync(User.Identity.GetUserId());

            var t = (DateTime.UtcNow - new DateTime(1970, 1, 1));
            int timestamp = (int) t.TotalSeconds;

            // Jwt payload defined here: https://support.zendesk.com/hc/en-us/articles/203663816-Setting-up-single-sign-on-with-JWT-JSON-Web-Token-
            var payload = new Dictionary<string, object>
            {
                {"iat", timestamp},
                {"jti", Guid.NewGuid().ToString()},
                {"name", identity.UserName},
                {"email", identity.Email},
                {"external_id", identity.Id}
            };
            if (identity.ResourceId.HasValue)
            {
                payload["remote_photo_url"] = "https://www.pyxisinnovation.com/images/avatars/thumbnails/" +
                                              identity.ResourceId.Value + ".jpg";
            }

            var jwt = JWT.JsonWebToken.Encode(payload, jwtSettings.Key, JWT.JwtHashAlgorithm.HS256);
            return Request.CreateResponse(HttpStatusCode.OK, new {jwt, url = jwtSettings.Url});
        }

        // POST api/v1/Account/Login
        [Route("Login")]
        [AllowAnonymous]
        public async Task<HttpResponseMessage> Login(LoginBindingModel loginBindingModel)
        {
            var identity = await Retry.ExecuteAsync(() => UserManager.FindByNameAsync(loginBindingModel.Username));
            if (identity == null && ValidateEmail(loginBindingModel.Username))
            {
                // Attempt to get their UserName with their email
                identity = await Retry.ExecuteAsync(() => UserManager.FindByEmailAsync(loginBindingModel.Username));
                if (identity != null)
                {
                    loginBindingModel.Username = identity.UserName;
                }
            }
            if (identity != null)
            {
                identity.LastLogin = DateTime.UtcNow;
                await Retry.ExecuteAsync(() => UserManager.UpdateAsync(identity));
            }

            // OAuth Token endpoint is buried deep in OWIN, so issue a Server POST to request a token.
            var requestUrl = HttpContext.Current.Request.Url.ToString();
            var tokenServiceUrl =
                requestUrl.Substring(0, requestUrl.IndexOf("/Account/Login", StringComparison.CurrentCultureIgnoreCase)) +
                "/Token";
            using (var client = new HttpClient())
            {
                var requestParams = new List<KeyValuePair<string, string>>
                {
                    new KeyValuePair<string, string>("grant_type", "password"),
                    new KeyValuePair<string, string>("username", loginBindingModel.Username),
                    new KeyValuePair<string, string>("password", loginBindingModel.Password)
                };
                var requestParamsFormUrlEncoded = new FormUrlEncodedContent(requestParams);
                var tokenServiceResponse =
                    await Retry.ExecuteAsync(() => client.PostAsync(tokenServiceUrl, requestParamsFormUrlEncoded));
                var responseString = await tokenServiceResponse.Content.ReadAsStringAsync();
                var responseCode = tokenServiceResponse.StatusCode;
                var responseMsg = new HttpResponseMessage(responseCode)
                {
                    Content = new StringContent(responseString, Encoding.UTF8, "application/json")
                };
                return responseMsg;
            }
        }

        // POST api/v1/Account/Logout
        [Route("Logout")]
        public IHttpActionResult Logout()
        {
            Authentication.SignOut(CookieAuthenticationDefaults.AuthenticationType);
            return Ok();
        }

        // GET api/v1/Account/ManageInfo?returnUrl=%2F&generateState=true
        [Route("ManageInfo")]
        public async Task<ManageInfoViewModel> GetManageInfo(string returnUrl, bool generateState = false)
        {
            IdentityUser user = await UserManager.FindByIdAsync(User.Identity.GetUserId());

            if (user == null)
            {
                return null;
            }

            List<UserLoginInfoViewModel> logins = new List<UserLoginInfoViewModel>();

            foreach (UserLoginInfo linkedAccount in user.Logins)
            {
                logins.Add(new UserLoginInfoViewModel
                {
                    LoginProvider = linkedAccount.LoginProvider,
                    ProviderKey = linkedAccount.ProviderKey
                });
            }

            if (user.PasswordHash != null)
            {
                logins.Add(new UserLoginInfoViewModel
                {
                    LoginProvider = LocalLoginProvider,
                    ProviderKey = user.UserName,
                });
            }

            return new ManageInfoViewModel
            {
                LocalLoginProvider = LocalLoginProvider,
                UserName = user.UserName,
                Logins = logins,
                ExternalLoginProviders = GetExternalLogins(returnUrl, generateState)
            };
        }

        // POST api/v1/Account/ForgotPassword
        [AllowAnonymous]
        [Route("ForgotPassword")]
        public async Task<IHttpActionResult> ForgotPassword(ForgotPasswordBindingModel model)
        {
            if (model == null || !ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            var identity = await FindByNameOrEmailAsync(model.UserName);
            if (identity == null || identity.Email == null
                ||
                (identity.ResetExpiration.HasValue &&
                 DateTime.UtcNow - identity.ResetExpiration.Value <= s_resetExpirationTime))
            {
                // Don't acknowledge the user doesn't exist or previous forgot attempt exists
                return Ok();
            }

            var resetToken = await UserManager.GeneratePasswordResetTokenAsync(identity.Id);
            identity.ResetPasswordHash = CreateHash(resetToken);
            identity.ResetExpiration = DateTime.UtcNow + s_resetExpirationTime;
            var result = await UserManager.UpdateAsync(identity);
            IHttpActionResult errorResult = GetErrorResult(result);
            if (errorResult != null)
            {
                return errorResult;
            }

            var frontEndUrl = GetReferrerBaseUrl();
            var bodyStringBuilder = new StringBuilder(s_emailTemplates["forgot-password"]);
            bodyStringBuilder.Replace("{{Url}}", frontEndUrl)
                .Replace("{{UserName}}", identity.UserName)
                .Replace("{{Token}}", resetToken);
            await UserManager.EmailService.SendAsync(new IdentityMessage
            {
                Destination = identity.Email,
                Subject = "WorldView Forgotten Password",
                Body = bodyStringBuilder.ToString()
            });

            return Ok();
        }

        // POST api/v1/Account/ResetForgottenPassword
        [AllowAnonymous]
        [Route("ResetForgottenPassword")]
        public async Task<IHttpActionResult> ResetForgottenPassword(ResetForgottenPasswordBindingModel model)
        {
            if (model == null || !ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            var identity = await FindByNameOrEmailAsync(model.UserName);
            var resetTokenHash = CreateHash(model.ResetToken);
            if (identity == null || !identity.ResetExpiration.HasValue || identity.ResetPasswordHash == null
                || identity.ResetExpiration.Value <= DateTime.UtcNow ||
                !resetTokenHash.SequenceEqual(identity.ResetPasswordHash))
            {
                return BadRequest("Request failed security measures.");
            }

            IdentityResult result =
                await UserManager.ResetPasswordAsync(identity.Id, model.ResetToken, model.NewPassword);
            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            // ResetToken is invalid after a successful reset.  Clear the reset values to allow a new reset token before expiration.
            identity = await UserManager.FindByNameAsync(model.UserName);
            identity.ResetPasswordHash = null;
            identity.ResetExpiration = null;
            // Ignoring result, failure to clear the expiration shouldn't fail the request.
            await UserManager.UpdateAsync(identity);

            return Ok();
        }

        // POST api/v1/Account/ChangePassword
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [Route("ChangePassword")]
        public async Task<IHttpActionResult> ChangePassword(ChangePasswordBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            IdentityResult result = await UserManager.ChangePasswordAsync(User.Identity.GetUserId(), model.OldPassword,
                model.NewPassword);
            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            return Ok();
        }

        // POST api/v1/Account/SetPassword
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [Route("SetPassword")]
        public async Task<IHttpActionResult> SetPassword(SetPasswordBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            IdentityResult result = await UserManager.AddPasswordAsync(User.Identity.GetUserId(), model.NewPassword);
            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            return Ok();
        }

        // POST api/v1/Account/SetConsent
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [Route("SetConsent")]
        public async Task<IHttpActionResult> SetConsent(ChangeConsentModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            var identity = await UserManager.FindByIdAsync(User.Identity.GetUserId());
            if ((!model.AcceptTerms.HasValue || model.AcceptTerms.Value == identity.AcceptTerms.Value)
                && (!model.AccountConsent.HasValue || model.AccountConsent.Value == identity.AccountConsent.Value)
                && (!model.PromotionConsent.HasValue || model.PromotionConsent.Value == identity.PromotionConsent.Value))
            {
                return Ok();
            }
            if (model.AcceptTerms.HasValue)
            {
                identity.AcceptTerms = model.AcceptTerms.Value;
            }
            if (model.AccountConsent.HasValue)
            {
                identity.AccountConsent = model.AccountConsent.Value;
            }
            if (model.PromotionConsent.HasValue)
            {
                identity.PromotionConsent = model.PromotionConsent.Value;
            }
            var result = await UserManager.UpdateAsync(identity);
            var errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            if (model.PromotionConsent.HasValue)
            {
                CrmManager.SetConsent(identity, model.PromotionConsent.Value);
            }

            return Ok();
        }

        // POST api/v1/Account/AddExternalLogin
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [Route("AddExternalLogin")]
        public async Task<IHttpActionResult> AddExternalLogin(AddExternalLoginBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            Authentication.SignOut(DefaultAuthenticationTypes.ExternalCookie);

            AuthenticationTicket ticket = AccessTokenFormat.Unprotect(model.ExternalAccessToken);

            if (ticket == null || ticket.Identity == null || (ticket.Properties != null
                                                              && ticket.Properties.ExpiresUtc.HasValue
                                                              &&
                                                              ticket.Properties.ExpiresUtc.Value < DateTimeOffset.UtcNow))
            {
                return BadRequest("External login failure.");
            }

            ExternalLoginData externalData = ExternalLoginData.FromIdentity(ticket.Identity);

            if (externalData == null)
            {
                return BadRequest("The external login is already associated with an account.");
            }

            IdentityResult result = await UserManager.AddLoginAsync(User.Identity.GetUserId(),
                new UserLoginInfo(externalData.LoginProvider, externalData.ProviderKey));

            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            return Ok();
        }

        // POST api/v1/Account/RemoveLogin
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [Route("RemoveLogin")]
        public async Task<IHttpActionResult> RemoveLogin(RemoveLoginBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            IdentityResult result;

            if (model.LoginProvider == LocalLoginProvider)
            {
                result = await UserManager.RemovePasswordAsync(User.Identity.GetUserId());
            }
            else
            {
                result = await UserManager.RemoveLoginAsync(User.Identity.GetUserId(),
                    new UserLoginInfo(model.LoginProvider, model.ProviderKey));
            }

            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            return Ok();
        }

        // GET api/v1/Account/ExternalLogin
        [OverrideAuthentication]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalCookie)]
        [AllowAnonymous]
        [Route("ExternalLogin", Name = "ExternalLogin")]
        public async Task<IHttpActionResult> GetExternalLogin(string provider, string error = null)
        {
            if (error != null)
            {
                var redirectUri = HttpUtility.ParseQueryString(Request.RequestUri.Query)["redirect_uri"];
                if (redirectUri != null)
                {
                    return Redirect(redirectUri + "#error=" + Uri.EscapeDataString(error));
                }
                return Redirect(Url.Content("~/") + "#error=" + Uri.EscapeDataString(error));
            }

            if (!User.Identity.IsAuthenticated)
            {
                return new ChallengeResult(provider, this);
            }

            ExternalLoginData externalLogin = ExternalLoginData.FromIdentity(User.Identity as ClaimsIdentity);

            if (externalLogin == null)
            {
                // LOCAL_AUTHORITY provider Cookie has been stored, redirect to the external provider to login again
                return new ChallengeResult(provider, this);
            }

            if (externalLogin.LoginProvider != provider)
            {
                Authentication.SignOut(DefaultAuthenticationTypes.ExternalCookie);
                return new ChallengeResult(provider, this);
            }

            PyxisIdentityUser user =
                await
                    Retry.ExecuteAsync(
                        () =>
                            UserManager.FindAsync(new UserLoginInfo(externalLogin.LoginProvider,
                                externalLogin.ProviderKey)));

            bool hasRegistered = user != null;

            if (hasRegistered)
            {
                Authentication.SignOut(DefaultAuthenticationTypes.ExternalCookie);
                ClaimsIdentity oAuthIdentity = await UserManager.CreateIdentityAsync(user,
                    OAuthDefaults.AuthenticationType);
                ClaimsIdentity cookieIdentity = await UserManager.CreateIdentityAsync(user,
                    CookieAuthenticationDefaults.AuthenticationType);
                AuthenticationProperties properties = ApplicationOAuthProvider.CreateProperties(user.UserName);
                Authentication.SignIn(properties, oAuthIdentity, cookieIdentity);
                user.LastLogin = DateTime.UtcNow;
                await Retry.ExecuteAsync(() => UserManager.UpdateAsync(user));
            }
            else
            {
                IEnumerable<Claim> claims = externalLogin.GetClaims();
                ClaimsIdentity identity = new ClaimsIdentity(claims, OAuthDefaults.AuthenticationType);
                Authentication.SignIn(identity);
            }

            return Ok();
        }

        // GET api/v1/Account/ExternalLogins?returnUrl=%2F&generateState=true
        [AllowAnonymous]
        [Route("ExternalLogins")]
        public IEnumerable<ExternalLoginViewModel> GetExternalLogins(string returnUrl, bool generateState = false)
        {
            IEnumerable<AuthenticationDescription> descriptions = Authentication.GetExternalAuthenticationTypes();
            List<ExternalLoginViewModel> logins = new List<ExternalLoginViewModel>();

            string state;

            if (generateState)
            {
                const int strengthInBits = 256;
                state = RandomOAuthStateGenerator.Generate(strengthInBits);
            }
            else
            {
                state = null;
            }

            foreach (AuthenticationDescription description in descriptions)
            {
                ExternalLoginViewModel login = new ExternalLoginViewModel
                {
                    Name = description.Caption,
                    Url = Url.Route("ExternalLogin", new
                    {
                        provider = description.AuthenticationType,
                        response_type = "token",
                        client_id = Startup.PublicClientId,
                        redirect_uri = new Uri(Request.RequestUri, returnUrl).AbsoluteUri,
                        state = state
                    }),
                    State = state
                };
                logins.Add(login);
            }

            return logins;
        }

        // POST api/v1/Account/Register
        [AllowAnonymous]
        [Route("Register")]
        public async Task<IHttpActionResult> Register(RegisterBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            var user = Mapper.Map<PyxisIdentityUser>(model);
            user.EmailConfirmed = false;

            var unavailableResponse = await ConfirmNameAndEmailAvailable(user);
            if (unavailableResponse != null)
            {
                return unavailableResponse;
            }

            if (String.IsNullOrWhiteSpace(user.Country) || String.IsNullOrWhiteSpace(user.City))
            {
                SetUserLocationFromIp(user);
            }

            user.Roles.Add(PyxisIdentityRoles.Unconfirmed);
            user.CrmId = CrmManager.AddContact(user);
            var result = await Retry.ExecuteAsync(() => UserManager.CreateAsync(user, model.Password));
            var errorResult = GetErrorResult(result);
            if (errorResult != null)
            {
                return errorResult;
            }

            try
            {
                var emailResult = await SendConfirmationEmail(user.Id);
                if (!(emailResult is OkResult))
                {
                    UserManager.Delete(user);
                }

                return emailResult;
            }
            catch (Exception)
            {
                //clean up.
                UserManager.Delete(user);
                throw;
            }
        }

        // POST api/v1/Account/RegisterExternal
        [OverrideAuthentication]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [Route("RegisterExternal")]
        public async Task<IHttpActionResult> RegisterExternal(RegisterExternalBindingModel model)
        {
            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            ExternalLoginData externalLogin = ExternalLoginData.FromIdentity(User.Identity as ClaimsIdentity);
            
            if (externalLogin == null)
            {
                return InternalServerError();
            }
            if (externalLogin.Email == null)
            {
                return ResponseMessage(Request.CreateResponse(HttpStatusCode.Forbidden, "Email must be included to register."));
            }

            var user = Mapper.Map<PyxisIdentityUser>(externalLogin);
            Mapper.Map(model, user, typeof(RegisterExternalBindingModel), typeof(PyxisIdentityUser));

            var unavailableResponse = await ConfirmNameAndEmailAvailable(user);
            if (unavailableResponse != null)
            {
                return unavailableResponse;
            }

            user.EmailConfirmed = true;
            user.Roles.Add(PyxisIdentityRoles.Member);
            user.ExternalLoginProvider = externalLogin.LoginProvider;
            if (String.IsNullOrWhiteSpace(user.Country) || String.IsNullOrWhiteSpace(user.City))
            {
                SetUserLocationFromIp(user);
            }

            user.Logins.Add(new UserLoginInfo(externalLogin.LoginProvider, externalLogin.ProviderKey));
            var accessTokenClaim = externalLogin.GetClaims().First(c => c.Type.Contains("access_token"));
            user.Claims.Add(new IdentityUserClaim
            {
                ClaimType = accessTokenClaim.Type, 
                ClaimValue = accessTokenClaim.Value
            });
            user.CrmId = CrmManager.AddContact(user);

            IdentityResult result = await Retry.ExecuteAsync(() => UserManager.CreateAsync(user));
            IHttpActionResult errorResult = GetErrorResult(result);

            if (errorResult != null)
            {
                return errorResult;
            }

            return Ok();
        }

        // GET api/v1/Account/Available
        [HttpGet]
        [AllowAnonymous]
        [Route("Available")]
        public HttpResponseMessage Available(string userName)
        {
            // allow email usernames if they aren't active email addresses
            var isEmail = ValidateEmail(userName);
            if (!NameBlacklist.Contains(userName)
                && db.GetIdentityNameAvailability(userName) 
                && (!isEmail || db.GetEmailAvailability(userName))
                )
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            return Request.CreateResponse(HttpStatusCode.Conflict);
        }

        // GET api/v1/Account/Available
        [HttpGet]
        [AllowAnonymous]
        [Route("Available")]
        public HttpResponseMessage EmailAvailable(string email)
        {
            if (!ValidateEmail(email))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified email is not valid");
            }
            if (db.GetEmailAvailability(email))
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            return Request.CreateResponse(HttpStatusCode.Conflict);
        }

        // GET api/v1/Account/SendConfirmationEmail
        [HttpGet]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [Route("SendConfirmationEmail")]
        public async Task<IHttpActionResult> SendConfirmationEmail()
        {
            return await SendConfirmationEmail(User.Identity.GetUserId());
        }

        // POST api/v1/Account/ConfirmEmail
        [AllowAnonymous]
        [Route("ConfirmEmail")]
        public async Task<IHttpActionResult> ConfirmEmail(ConfirmEmailBindingModel model)
        {
            var identity = await Retry.ExecuteAsync(() => UserManager.FindByEmailAsync(model.Email));
            if (identity == null)
            {
                return BadRequest("Provided email address was not found.");
            }
            var confirmationTokenHash = CreateHash(model.ConfirmationToken);
            if (identity.EmailConfirmedHash == null || !confirmationTokenHash.SequenceEqual(identity.EmailConfirmedHash))
            {
                return BadRequest("Request failed security measures.");
            }
            if (identity.EmailConfirmed.HasValue && identity.EmailConfirmed.Value)
            {
                return Ok();
            }

            identity.EmailConfirmed = true;
            identity.Roles.Clear();
            identity.Roles.Add(PyxisIdentityRoles.Member);
            CrmManager.SetAccountConfirmation(identity);
            var result = await Retry.ExecuteAsync(() => UserManager.UpdateAsync(identity));
            var errorResult = GetErrorResult(result);
            return errorResult ?? Ok();
        }

        // GET api/v1/Account/TokenDetails
        [HttpGet]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [Route("TokenDetails")]
        [ApiExplorerSettings(IgnoreApi = true)]
        public HttpResponseMessage TokenDetails()
        {
            var token = Request.Headers.Authorization.Parameter;
            var ticket = AccessTokenFormat.Unprotect(token);
            return Request.CreateResponse(HttpStatusCode.OK, ticket);
        }

        // GET api/v1/Account/Broken
        [HttpGet]
        [Authorize(Roles = PyxisIdentityRoles.SiteAdmin)]
        [Route("Broken")]
        public List<string> Broken()
        {
            return UserManager.Users.Where(user => user.ResourceId == null).Select(user => user.UserName).ToList();
        }

        // GET api/v1/Account/LastLogin
        [HttpGet]
        [Authorize(Roles = PyxisIdentityRoles.SiteAdmin)]
        [Route("LastLogin")]
        public PageResult<dynamic> LastLogin(ODataQueryOptions<UserLastLoginModel> options)
        {
            return UserManager.Users.Select(user => new UserLastLoginModel(user)).ToPageResult(Request, options);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                UserManager.Dispose();
            }

            base.Dispose(disposing);
        }

        #region Helpers

        private IAuthenticationManager Authentication
        {
            get { return Request.GetOwinContext().Authentication; }
        }

        private IHttpActionResult GetErrorResult(IdentityResult result)
        {
            if (result == null)
            {
                return InternalServerError();
            }

            if (!result.Succeeded)
            {
                if (result.Errors != null)
                {
                    foreach (string error in result.Errors)
                    {
                        ModelState.AddModelError("", error);
                    }
                }

                if (ModelState.IsValid)
                {
                    // No ModelState errors are available to send, so just return an empty BadRequest.
                    return BadRequest();
                }

                return BadRequest(ModelState);
            }

            return null;
        }

        private class ExternalLoginData
        {
            public string LoginProvider { get; set; }
            public string ProviderKey { get; set; }
            public string UserName { get; set; }
            public string Email { get; set; }
            public string FirstName { get; set; }
            public string LastName { get; set; }
            public string AccessToken { get; set; }

            public IList<Claim> GetClaims()
            {
                IList<Claim> claims = new List<Claim>();
                claims.Add(new Claim(ClaimTypes.NameIdentifier, ProviderKey, null, LoginProvider));

                if (UserName != null)
                {
                    claims.Add(new Claim(ClaimTypes.Name, UserName, null, LoginProvider));
                }

                if (String.Compare(LoginProvider, "google", StringComparison.InvariantCultureIgnoreCase) == 0)
                {
                    claims.Add(new Claim("urn:google:access_token", AccessToken, ClaimValueTypes.String, LoginProvider));
                }
                else if (String.Compare(LoginProvider, "facebook", StringComparison.InvariantCultureIgnoreCase) == 0)
                {
                    claims.Add(new Claim("urn:facebook:access_token", AccessToken, ClaimValueTypes.String, LoginProvider));
                }
                else if (String.Compare(LoginProvider, "linkedin", StringComparison.InvariantCultureIgnoreCase) == 0)
                {
                    claims.Add(new Claim("urn:linkedin:access_token", AccessToken, ClaimValueTypes.String, LoginProvider));
                }
                if (Email != null)
                {
                    claims.Add(new Claim(ClaimTypes.Email, Email, null, LoginProvider));
                }
                claims.Add(new Claim(ClaimTypes.GivenName, FirstName, null, LoginProvider));
                claims.Add(new Claim(ClaimTypes.Surname, LastName, null, LoginProvider));

                return claims;
            }

            public static ExternalLoginData FromIdentity(ClaimsIdentity identity)
            {
                if (identity == null)
                {
                    return null;
                }

                Claim providerKeyClaim = identity.FindFirst(ClaimTypes.NameIdentifier);

                if (providerKeyClaim == null || String.IsNullOrEmpty(providerKeyClaim.Issuer)
                    || String.IsNullOrEmpty(providerKeyClaim.Value))
                {
                    return null;
                }

                if (providerKeyClaim.Issuer == ClaimsIdentity.DefaultIssuer)
                {
                    return null;
                }
                
                var firstName = identity.FindFirstValue(ClaimTypes.GivenName) ?? identity.Claims.First(c => c.Type.Contains("first_name")).Value;
                var lastName = identity.FindFirstValue(ClaimTypes.Surname) ?? identity.Claims.First(c => c.Type.Contains("last_name")).Value;

                return new ExternalLoginData
                {
                    LoginProvider = providerKeyClaim.Issuer,
                    ProviderKey = providerKeyClaim.Value,
                    UserName = identity.FindFirstValue(ClaimTypes.Name),
                    Email = identity.FindFirstValue(ClaimTypes.Email),
                    FirstName = firstName,
                    LastName = lastName,
                    AccessToken = identity.Claims.First(c => c.Type.Contains("access_token")).Value
                };
            }
        }

        private static class RandomOAuthStateGenerator
        {
            private static RandomNumberGenerator _random = new RNGCryptoServiceProvider();

            public static string Generate(int strengthInBits)
            {
                const int bitsPerByte = 8;

                if (strengthInBits % bitsPerByte != 0)
                {
                    throw new ArgumentException("strengthInBits must be evenly divisible by 8.", "strengthInBits");
                }

                int strengthInBytes = strengthInBits / bitsPerByte;

                byte[] data = new byte[strengthInBytes];
                _random.GetBytes(data);
                return HttpServerUtility.UrlTokenEncode(data);
            }
        }

        private static byte[] CreateHash(string resetToken)
        {
            return new SHA256Managed().ComputeHash(System.Text.Encoding.UTF8.GetBytes(resetToken));
        }

        private static void SetUserLocationFromIp(PyxisIdentityUser user)
        {
            var ipLocation = GetLocationFromIp();
            user.Country = ipLocation.Country;
            user.City = ipLocation.City;
        }

        private bool ValidateEmail(string emailString)
        {
            return Regex.IsMatch(emailString, @"\A(?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?)\Z", RegexOptions.IgnoreCase);
        }

        private async Task<PyxisIdentityUser> FindByNameOrEmailAsync(string userName)
        {
            return await Retry.ExecuteAsync(() => UserManager.FindByNameAsync(userName)) ?? await Retry.ExecuteAsync(() => UserManager.FindByEmailAsync(userName));
        }

        private async Task<BadRequestErrorMessageResult> ConfirmNameAndEmailAvailable(PyxisIdentityUser registrant)
        {
            if (NameBlacklist.Contains(registrant.UserName))
            {
                return BadRequest("Specified user name (" + registrant.UserName + ") is reserved");
            }
            var existingUser = await FindByNameOrEmailAsync(registrant.UserName);
            if (existingUser != null)
            {
                return BadRequest("User with specified user name (" + registrant.UserName + ") already exists");
            }
            existingUser = await Retry.ExecuteAsync(() => UserManager.FindByEmailAsync(registrant.Email));
            if (existingUser != null)
            {
                return BadRequest("User with specified email (" + registrant.Email + ") already exists");
            }
            return null;
        }

        public async Task<IHttpActionResult> SendConfirmationEmail(string userId)
        {
            var identity = await Retry.ExecuteAsync(() => UserManager.FindByIdAsync(userId));
            var confirmationToken = await Retry.ExecuteAsync(() => UserManager.GenerateEmailConfirmationTokenAsync(identity.Id));
            identity.EmailConfirmedHash = CreateHash(confirmationToken);
            var result = await Retry.ExecuteAsync(() => UserManager.UpdateAsync(identity));
            IHttpActionResult errorResult = GetErrorResult(result);
            if (errorResult != null)
            {
                return errorResult;
            }

            var frontEndUrl = GetReferrerBaseUrl();
            var bodyStringBuilder = new StringBuilder(s_emailTemplates["confirm-email"]);
            bodyStringBuilder.Replace("{{Url}}", frontEndUrl)
                .Replace("{{UserName}}", identity.UserName)
                .Replace("{{Email}}", identity.Email)
                .Replace("{{Token}}", confirmationToken);
            await Retry.ExecuteAsync(() =>
                UserManager.EmailService.SendAsync(new IdentityMessage
                {
                    Destination = identity.Email,
                    Subject = "Confirm your PYXIS Studio Account",
                    Body = bodyStringBuilder.ToString()
                }));

            return Ok();
        }

        private string GetReferrerBaseUrl()
        {
            var baseUrl = Request.Headers.Referrer.GetLeftPart(UriPartial.Authority) + '/';
            if (String.IsNullOrEmpty(baseUrl))
            {
                baseUrl = Properties.Settings.Default.FrontEndUrl;
            }
            return baseUrl;
        }

        private class IpLocationModel
        {
            public string Ip { get; set; }
            public string Country { get; set; }
            public string City { get; set; }
        }

        private static IpLocationModel GetLocationFromIp()
        {
            var ip = HttpContext.Current.Request.UserHostAddress;
            var webClient = new WebClient();
            var ipLocationModel = new IpLocationModel { Ip = ip, Country = "-", City = "-" };
            try
            {
                dynamic ipLocation = JsonConvert.DeserializeObject(webClient.DownloadString("https://freegeoip.net/json/" + ip));
                ipLocationModel = new IpLocationModel { Ip = ipLocation.ip, Country = ipLocation.country_name, City = ipLocation.city };
            }
            catch
            {
                // failed to contact location server
            }
            return ipLocationModel;
        }

        private class JwtSettings
        {
            public string Key { get; set; }
            public string Url { get; set; }
        }

        #endregion
    }
}
