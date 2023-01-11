#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Web.Security;
using HoytSoft.Common.Data;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Web {
	public abstract class UserControl : System.Web.UI.UserControl {
		#region Properties
		private bool
			authenticatedChecked = false,
			authenticatedValue = false;

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
				if (this.Page == null || !(this.Page is Page))
					return null;
				return (this.Page as Page).SiteUser;
			}
		}
		#endregion

		#region Methods
		protected void AddError(string Message) {
			if (this.Page is Common.Web.Page) {
				(this.Page as Common.Web.Page).AddErrorInternal(Message);
			}
		}

		protected void AddSuccess(string Message) {
			if (this.Page is Common.Web.Page) {
				(this.Page as Common.Web.Page).AddSuccessInternal(Message);
			}
		}

		protected IUser CheckLogin(string Login, string Password) {
			if (this.Page is Common.Web.Page)
				return (this.Page as Common.Web.Page).CheckLogin(Login, Password);
			else
				return null;
		}

		protected IUser CheckLogin(int UserID) {
			if (this.Page is Common.Web.Page)
				return (this.Page as Common.Web.Page).CheckLogin(UserID);
			else
				return null;
		}
		#endregion
	}
}
