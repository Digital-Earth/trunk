#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Web.Security;
using System.Security.Principal;
using System.Collections.Generic;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	public interface IAuthenticationCallback {
		IUser CheckLogin(string Login, string Password);
		IUser CheckLogin(string UserID);
		bool LoginUser(IUser User);
		bool LoginUser(IUser User, bool RememberMe);
		bool LoginUser(IUser User, bool RememberMe, Authentication.FetchUniqueIdentifierDelegate FetchIdentifier);
		bool LogoutUser(IUser User);
		string[] CheckRoles(IUser User);
		void AuthenticateRequest();
		bool CheckUserAuthenticated();
		void UserNotAuthenticated();
		IUser LoadFromIdentity();
	}

	public interface IFormsAuthenticationCallback : IAuthenticationCallback {
	}

	public sealed class Authentication {
		#region Constants
		public static readonly string
			SESSION_USER_KEY = "SESSION_" + typeof(Authentication).FullName,
			SESSION_USER_ROLES_KEY = "SESSION_ROLES_" + typeof(Authentication).FullName
		;
		#endregion

		#region Delegates
		public delegate string FetchUniqueIdentifierDelegate(IUser User);
		#endregion

		#region Properties
		public static IAuthenticationCallback Callback {
			get {
				AuthenticationSettings settings = Settings.From<AuthenticationSettings>(Settings.Section.Authentication);
				if (settings == null || !settings.Enabled)
					return null;
				return settings.Callback;
			}
		}

		public static bool Enabled {
			get {
				AuthenticationSettings settings = Settings.From<AuthenticationSettings>(Settings.Section.Authentication);
				if (settings == null)
					return false;
				return settings.Enabled;
			}
		}
		#endregion

		#region Public Static Methods
		public static IUser CheckLogin(string Login, string Password) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return null;
			return c.CheckLogin(Login, Password);
		}

		public static IUser CheckLogin(string UserID) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return null;
			return c.CheckLogin(UserID);
		}

		public static bool LoginUser(IUser User) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return false;
			return c.LoginUser(User);
		}

		public static bool LoginUser(IUser User, bool RememberMe) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return false;
			return c.LoginUser(User, RememberMe);
		}

		public static bool LoginUser(IUser User, bool RememberMe, FetchUniqueIdentifierDelegate FetchIdentifier) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return false;
			return c.LoginUser(User, RememberMe, FetchIdentifier);
		}

		public static bool LogoutUser(IUser User) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return false;
			return c.LogoutUser(User);
		}

		public static string[] CheckRoles(IUser User) {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return new string[0];
			return c.CheckRoles(User);
		}

		public static void AuthenticateRequest() {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return;
			c.AuthenticateRequest();
		}

		public static bool CheckUserAuthenticated() {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return false;
			return c.CheckUserAuthenticated();
		}

		public static IUser LoadFromIdentity() {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return null;
			return c.LoadFromIdentity();
		}

		public static void UserNotAuthenticated() {
			IAuthenticationCallback c = Callback;
			if (c == null)
				return;
			c.UserNotAuthenticated();
		}
		#endregion
	}

	#region Implementations
	public abstract class AbstractFormsAuthenticationCallback : IFormsAuthenticationCallback {
		public abstract IUser CheckLogin(string Login, string Password);

		public abstract IUser CheckLogin(string UserID);

		public virtual string[] CheckRoles(IUser User) {
			return new string[0];
		}

		public virtual bool LogoutUser(IUser User) {
			System.Web.Security.FormsAuthentication.SignOut();
			return true;
		}

		public virtual bool LoginUser(IUser User) {
			return LoginUser(User, false);
		}

		public virtual bool LoginUser(IUser User, bool RememberMe) {
			return LoginUser(User, RememberMe, delegate(IUser MyUser) { return MyUser.ID.ToString(); });
		}

		public virtual bool LoginUser(IUser User, bool RememberMe, Authentication.FetchUniqueIdentifierDelegate FetchIdentifier) {
			if (User == null)
				return false;

			string[] arrRoles = CheckRoles(User);

			HttpContext.Current.User = new GenericPrincipal(HttpContext.Current.User.Identity, arrRoles);
			HttpCookie cookie = System.Web.Security.FormsAuthentication.GetAuthCookie(User.Login, RememberMe);
			FormsAuthenticationTicket ticket = System.Web.Security.FormsAuthentication.Decrypt(cookie.Value); //TODO: Find quicker way to do this - decrypt could be costly operation

			//Store roles inside the Forms cookie.
			FormsAuthenticationTicket newticket = new FormsAuthenticationTicket(ticket.Version, ticket.Name, ticket.IssueDate, ticket.Expiration, RememberMe, FetchIdentifier(User) + ";" + string.Join(",", arrRoles), ticket.CookiePath);
			cookie.Value = System.Web.Security.FormsAuthentication.Encrypt(newticket);

			if (HttpContext.Current.Response.Cookies[cookie.Name] != null)
				HttpContext.Current.Response.Cookies.Remove(cookie.Name);
			HttpContext.Current.Response.Cookies.Add(cookie);

			return true;
		}

		public virtual void AuthenticateRequest() {
			if (HttpContext.Current.User != null) {
				if (HttpContext.Current.User.Identity is FormsIdentity) {
					FormsIdentity ident = (HttpContext.Current.User.Identity as FormsIdentity);
					if (ident.Ticket != null && !string.IsNullOrEmpty(ident.Ticket.UserData)) {
						string[] split = ident.Ticket.UserData.Split(';');
						string[] roles = null;

						if (split.Length > 2 || split.Length < 0)
							return;
						if (split.Length == 2) roles = split[1].Split(',');
						else roles = ident.Ticket.UserData.Split(',');

						HttpContext.Current.User = new System.Security.Principal.GenericPrincipal(ident, roles);
					}
				}

			}//user != null
		}

		public virtual IUser LoadFromIdentity() {
			if (HttpContext.Current.Session[Authentication.SESSION_USER_KEY] != null && HttpContext.Current.Session[Authentication.SESSION_USER_KEY] is IUser)
				return (IUser)HttpContext.Current.Session[Authentication.SESSION_USER_KEY];

			if (HttpContext.Current.User == null || !(HttpContext.Current.User.Identity is FormsIdentity))
				return null;

			FormsIdentity ident = (HttpContext.Current.User.Identity as FormsIdentity);
			if (ident.Ticket != null && !string.IsNullOrEmpty(ident.Ticket.UserData)) {

				string[] split = ident.Ticket.UserData.Split(';');
				if (split.Length != 2)
					return null;

				return CheckLogin(split[0]);
			}
			return null;
		}

		public virtual bool CheckUserAuthenticated() {
			if (HttpContext.Current.User == null || HttpContext.Current.User.Identity.AuthenticationType != "Forms")
				return false;
			System.Security.Principal.IIdentity user = HttpContext.Current.User.Identity;
			return user.IsAuthenticated;
		}

		public virtual void UserNotAuthenticated() {
			FormsAuthentication.RedirectToLoginPage();
		}
	}
	#endregion
}
