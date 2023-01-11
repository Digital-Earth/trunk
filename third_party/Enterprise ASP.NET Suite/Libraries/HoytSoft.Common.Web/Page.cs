#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Web.Security;
using System.Net;
using HoytSoft.Common.Data;

namespace HoytSoft.Common.Web {

	[PublicPage]
	public class PublicSharedPage : Page {
	}

	public class Page : System.Web.UI.Page {
		#region Variables/Constants
		public const string
			IMPERSONATE_COOKIE_NAME	= ".Impersonate"
		;

		private bool 
			authenticatedChecked = false, 
			authenticatedValue = false
		;

		private IUser
			user
		;
		#endregion

		#region Properties
		public bool DebugMode { get { return Settings.DebugMode; } }
		public bool TestMode { get { return Settings.TestMode; } }

		public bool Authenticated {
			get {
				if (!authenticatedChecked) {
					authenticatedChecked = true;
					authenticatedValue = Authentication.CheckUserAuthenticated();
				}
				return authenticatedValue;
			}
		}

		public bool IsAdministrator {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.IsAdministrator;
			}
		}

		public bool IsAdministratorManager {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.IsAdministratorManager;
			}
		}

		public bool IsContentEditor {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.IsContentEditor;
			}
		}

		public bool CanImpersonateOthers {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.OptionCanImpersonate;
			}
		}

		public bool CanChangeLogin {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.OptionCanChangeLogin;
			}
		}

		public bool CanChangePassword {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.OptionCanChangePassword;
			}
		}

		public bool CanEditPersonalInformation {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.OptionCanEditPersonalInformation;
			}
		}

		public bool IsValidUser {
			get {
				if (!Authenticated)
					return false;
				return (this.SiteUser != null);
			}
		}

		public bool IsImpersonating {
			get {
				if (!Authenticated)
					return false;
				if (this.SiteUser == null)
					return false;
				return this.SiteUser.IsImpersonating;
			}
		}

		public IUser SiteUser {
			get {
				if (!Authenticated)
					return null;

				if (this.user != null)
					return this.user;

				//If the member's session information doesn't exist, then we need to essentially
				//log him in again...
				//if ((this.Request.Cookies[IMPERSONATE_COOKIE_NAME] != null || (this.user = (IUser)Session[SESSION_MEMBER_KEY]) == null) && HttpContext.Current.User != null && HttpContext.Current.User.Identity is FormsIdentity)
				if (Authentication.Enabled)
					this.user = Authentication.LoadFromIdentity();

				Session[Authentication.SESSION_USER_KEY] = this.user;

				return this.user;
			}
		}
		#endregion

		#region Protected Methods
		internal void AddErrorInternal(string Message) {
			this.AddError(Message);
		}

		internal void AddSuccessInternal(string Message) {
			this.AddSuccess(Message);
		}

		protected void AddError(string Message) {
			if (this.Master != null && this.Master is MasterPage)
				(this.Master as MasterPage).AddError(Message);
		}

		protected void AddSuccess(string Message) {
			if (this.Master != null && this.Master is MasterPage)
				(this.Master as MasterPage).AddSuccess(Message);
		}

		protected void SetSuccessBulletStyle(System.Web.UI.WebControls.BulletStyle BulletStyle) {
			if (this.Master != null && this.Master is MasterPage)
				(this.Master as MasterPage).SetSuccessBulletStyle(BulletStyle);
		}

		protected void SetErrorBulletStyle(System.Web.UI.WebControls.BulletStyle BulletStyle) {
			if (this.Master != null && this.Master is MasterPage)
				(this.Master as MasterPage).SetErrorBulletStyle(BulletStyle);
		}

		protected virtual void CheckForImpersonation() {
			if (this.SiteUser == null || Request.Cookies[IMPERSONATE_COOKIE_NAME] == null) return;

			//Relogin the person...
			LoginCurrentUser(this.SiteUser, false);

			try {
				string origMemberID = Common.Configuration.Cryptography.Decrypt(Request.Cookies[IMPERSONATE_COOKIE_NAME].Value);
				int tmp;
				if (int.TryParse(origMemberID, out tmp)) {
					//Get the original member's information...
					IUser origMember = CheckLogin(tmp);
					if (origMember != null && (origMember.OptionCanImpersonate)) {
						this.SiteUser.IsImpersonating = true;
						this.SiteUser.OriginalUser = origMember;
					}
				}
			} catch {
			}
		}

		///<summary>Override this method with your own implementation. It is called after the PageLoad event.</summary>
		protected virtual void LoadData() { }
		#endregion

		#region Overrides
		protected override void OnPreInit(EventArgs e) {
			//See http://www.west-wind.com/WebLog/posts/4057.aspx 
			//Make sure users always come in using the full domain name.
			//This is necessary to ensure that switching to SSL works reliably...

			//This appears when users access the site through http://www.....net/
			//then use HTTPS through https://....net/ 
			//This would create 2 cookies and sessions and confused IE and FireFox. It also 
			//made it impossible to impersonate on IE.
			Configuration.Internal.HttpSecuritySettings security = Settings.From<Configuration.Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security != null) {
				Uri WebHost = new Uri(security.UnsecureSite);
				if (Request.Url.Host != WebHost.Host) {
					UriBuilder NewUrl = new UriBuilder(Request.Url);
					NewUrl.Host = WebHost.Host;
					Response.Redirect(NewUrl.Uri.ToString(), true);
					return;
				}
			}

			this.CheckForImpersonation();

			object[] o = this.GetType().GetCustomAttributes(typeof(PublicPageAttribute), true);
			if (o == null || o.Length == 0)
				this.CheckAuthenticatedUser();

			base.OnPreInit(e);
		}

		protected override void OnLoad(EventArgs e) {
			base.OnLoad(e);
			
			if (!this.IsPostBack)
				this.LoadData();
		}
		#endregion

		#region Public Methods
		public string GetIPAddress() {
			string ip = Request.ServerVariables["HTTP_X_FORWARDED_FOR"];
			if (string.IsNullOrEmpty(ip))
				ip = Request.ServerVariables["REMOTE_ADDR"];
			if (ip == null)
				return string.Empty;
			return ip;
		}

		public long GetIPAddressAsLong() {
			return Utils.GetIPAddressAsLong(GetIPAddress());
		}

		public string FindPathToSecureSite(string Path) { return FindPathToSecureSite(Path, true); }
		public string FindPathToSecureSite(string Path, bool ShowReturnUrl) {
			Configuration.Internal.HttpSecuritySettings security = Settings.From<Configuration.Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security != null)
				return security.FindPathToSecureSite(Path, ShowReturnUrl);
			return Utils.CheckApplicationPath(Path);
		}

		public string FindPathToUnsecureSite(string Path) { return FindPathToUnsecureSite(Path, true); }
		public string FindPathToUnsecureSite(string Path, bool ShowReturnUrl) {
			Configuration.Internal.HttpSecuritySettings security = Settings.From<Configuration.Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security != null)
				return security.FindPathToUnsecureSite(Path, ShowReturnUrl);
			else
				return Utils.CheckApplicationPath(Path);
		}

		public void RedirectToSecureSite(string Path) { RedirectToSecureSite(Path, true); }
		public void RedirectToSecureSite(string Path, bool ShowReturnUrl) {
			Response.Redirect(FindPathToSecureSite(Path, ShowReturnUrl), true);
		}

		public void RedirectFromSecureSite(string Path) { RedirectFromSecureSite(Path, true); }
		public void RedirectFromSecureSite(string Path, bool ShowReturnUrl) {
			Response.Redirect(FindPathToUnsecureSite(Path, ShowReturnUrl), true);
		}

		public void RedirectToSecureSite() {
			Configuration.Internal.HttpSecuritySettings s = Settings.From<Configuration.Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (s != null)
				s.RedirectToSecureSite();
		}

		public void RedirectFromSecureSite() {
			Configuration.Internal.HttpSecuritySettings s = Settings.From<Configuration.Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (s != null)
				s.RedirectFromSecureSite();
		}

		public virtual void CheckAuthenticatedUser() {
			if (!Authenticated) {
				Authentication.UserNotAuthenticated();
				Response.Flush();
				Response.End();
			}
		}

		public bool ValidateLogin(string Login, string Password) {
			IUser user = null;
			if (Authentication.Enabled && (user = Authentication.CheckLogin(Login, Password)) != null) {
				if (HttpContext.Current.Session != null)
					HttpContext.Current.Session[Authentication.SESSION_USER_KEY] = user;
				return true;
			}
			return false;
		}

		public bool ValidateLogin(string UserID) {
			IUser user = null;
			if (Authentication.Enabled && (user = Authentication.CheckLogin(UserID)) != null) {
				if (HttpContext.Current.Session != null)
					HttpContext.Current.Session[Authentication.SESSION_USER_KEY] = user;
				return true;
			}
			return false;
		}

		public bool ReloadUserInformation() {
			return ReloadUserInformation(delegate(IUser User) { return User.ID.ToString(); });
		}

		public bool ReloadUserInformation(Authentication.FetchUniqueIdentifierDelegate FetchDelegate) {
			if (this.SiteUser == null) return false;

			IUser u = null;
			if (Authentication.Enabled && (u = Authentication.CheckLogin(FetchDelegate(this.SiteUser))) != null) {
				if (HttpContext.Current.Session != null)
					HttpContext.Current.Session[Authentication.SESSION_USER_KEY] = u;
				this.user = u;
				return true;
			}
			return false;
		}

		public void LoginCurrentUser(IUser User, bool RememberMe) {
			LoginCurrentUser(User, RememberMe, delegate(IUser MyUser) { return MyUser.ID.ToString(); });
		}

		public void LoginCurrentUser(IUser User, bool RememberMe, Authentication.FetchUniqueIdentifierDelegate FetchDelegate) {
			if (User == null) return; //Not a valid user

			if (!Authentication.LoginUser(User, RememberMe, FetchDelegate))
				return;

			HttpContext.Current.Session[Authentication.SESSION_USER_KEY] = User;

			//Let everyone know we're logging someone in...
			SystemEvents.UserLogin(User);
		}

		public void LogoutCurrentUser() {
			LogoutCurrentUser((IUser)HttpContext.Current.Session[Authentication.SESSION_USER_KEY]);
		}

		public void LogoutCurrentUser(IUser User) {
			if (User != null) {
				if (User.IsImpersonating && User.OriginalUser != null) {
					LoginCurrentUser(User.OriginalUser, true);
					Utils.RemoveCookie(IMPERSONATE_COOKIE_NAME);
					return;
				}
			}

			//Let the system know we're logging the member out...
			if (User != null && !SystemEvents.UserLogout(User))
				return;

			Authentication.LogoutUser(User);

			HttpContext.Current.Session.Clear();
			HttpContext.Current.Session.Abandon();
		}

		public bool ImpersonateUser(IUser CurrentUser, int UserIDToImpersonate) {
			//The member to impersonate...
			IUser user = null;

			//Only certain administrators can impersonate other users...
			if (CurrentUser == null || UserIDToImpersonate <= 0 || !CurrentUser.OptionCanImpersonate) return false;
			if ((user = CheckLogin(UserIDToImpersonate)) == null) return false;
			if (user.IsAdministrator) return false; //You can't impersonate other administrators!

			//Okay, we're clear to impersonate this user...

			//Make a cookie that holds our current member id so when we log out as the impersonated user, we can go back to our normal login...
			HttpCookie cookie = new HttpCookie(IMPERSONATE_COOKIE_NAME, Common.Configuration.Cryptography.Encrypt(CurrentUser.ID.ToString()));
			//cookie.Expires = DateTime.Now.AddYears(1); //Make it a session cookie
			HttpContext.Current.Response.SetCookie(cookie);

			LoginCurrentUser(user, false);
			return true;
		}
		#endregion

		#region Virtual Methods
		public virtual IUser CheckLogin(string Login, string Password) {
			return Authentication.CheckLogin(Login, Password);
		}

		public virtual IUser CheckLogin(int UserID) {
			return Authentication.CheckLogin(UserID.ToString());
		}
		#endregion

	} //class

} //namespace
