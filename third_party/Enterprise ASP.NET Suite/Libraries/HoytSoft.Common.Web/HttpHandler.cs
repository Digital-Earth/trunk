#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Web.Security;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Web {
	public abstract class HttpHandler : IHttpHandler {
		#region Properties
		private bool
			authenticatedChecked = false,
			authenticatedValue = false
		;

		public bool Authenticated {
			get {
				if (!authenticatedChecked) {
					if (HttpContext.Current.User != null) {
						if (HttpContext.Current.User.Identity.AuthenticationType != "Forms") {
							authenticatedValue = false;
							authenticatedChecked = true;
							return authenticatedValue;
						}

						System.Security.Principal.IIdentity user = HttpContext.Current.User.Identity;
						authenticatedValue = user.IsAuthenticated;
						authenticatedChecked = true;
						return authenticatedValue;
					}//user != null

					authenticatedValue = false;
					authenticatedChecked = true;
					return authenticatedValue;
				}
				return authenticatedValue;
			}
		}

		private IUser user;
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

				HttpContext.Current.Session[Authentication.SESSION_USER_KEY] = this.user;

				return this.user;
			}
		}
		#endregion

		#region IHttpHandler Members
		public abstract bool IsReusable {
			get;
		}

		public abstract void ProcessRequest(HttpContext context);
		#endregion

		#region Virtual Methods
		public virtual IUser CheckLogin(string Login, string Password) {
			throw new NotImplementedException("Please implement CheckLogin(String, String) in your page class");
		}

		public virtual IUser CheckLogin(int UserID) {
			throw new NotImplementedException("Please implement CheckLogin(int) in your page class");
		}
		#endregion
	}
}
