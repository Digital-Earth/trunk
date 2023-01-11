#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using HoytSoft.Common.Data;

namespace HoytSoft.Common.Web {
	public class MasterPage : System.Web.UI.MasterPage {
		protected SiteMapPath breadCrumbs;
		protected PlaceHolder phErrors;
		protected BulletedList lstErrors;
		protected PlaceHolder phSuccess;
		protected BulletedList lstSuccess;

		#region Properties
		public SiteMapPath BreadCrumbs { get { return this.breadCrumbs; } }
		public bool DebugMode { get { return Settings.DebugMode; } }
		public bool TestMode { get { return Settings.TestMode; } }

		public bool Authenticated {
			get {
				if (this.Page != null && this.Page is Page)
					return (this.Page as Page).Authenticated;
				else
					return false;
			}
		}

		public bool IsAdministrator {
			get {
				if (this.Page != null && this.Page is Page)
					return (this.Page as Page).IsAdministrator;
				else
					return false;
			}
		}

		public bool IsValidUser {
			get {
				if (this.Page != null && this.Page is Page)
					return (this.Page as Page).IsValidUser;
				else
					return false;
			}
		}
		#endregion

		#region Public Methods
		public void AddError(string Message) {
			if (this.phErrors == null || this.lstErrors == null) return;
			this.lstErrors.Items.Add(Message);
			this.phErrors.Visible = true;
			this.lstErrors.Visible = true;
		}

		public void SetErrorBulletStyle(BulletStyle BulletStyle) {
			if (this.lstErrors != null)
				this.lstErrors.BulletStyle = BulletStyle;
		}

		public void AddSuccess(string Message) {
			if (this.phSuccess == null || this.lstSuccess == null) return;
			this.lstSuccess.Items.Add(Message);
			this.phSuccess.Visible = true;
			this.lstSuccess.Visible = true;
		}

		public void SetSuccessBulletStyle(BulletStyle BulletStyle) {
			if (this.lstSuccess != null)
				this.lstSuccess.BulletStyle = BulletStyle;
		}

		public string FindPathToSecureSite(string Path) {
			if (this.Page != null && this.Page is Page)
				return (this.Page as Page).FindPathToSecureSite(Path);
			return null;
		}

		public string FindPathToUnsecureSite(string Path) {
			if (this.Page != null && this.Page is Page)
				return (this.Page as Page).FindPathToUnsecureSite(Path);
			return null;
		}

		public void RedirectToSecureSite(string Path) {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).RedirectToSecureSite(Path);
		}

		public void RedirectFromSecureSite(string Path) {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).RedirectFromSecureSite(Path);
		}

		public void RedirectToSecureSite() {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).RedirectToSecureSite();
		}

		public void RedirectFromSecureSite() {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).RedirectFromSecureSite();
		}

		public virtual void CheckAuthenticatedUser() {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).CheckAuthenticatedUser();
		}

		public virtual void LoginCurrentUser(IUser Member, bool RememberMe) {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).LoginCurrentUser(Member, RememberMe);
		}

		public virtual void LogoutCurrentUser() {
			if (this.Page != null && this.Page is Page)
				(this.Page as Page).LogoutCurrentUser();
		}

		public string GetIPAddress() {
			if (this.Page != null && this.Page is Page)
				return (this.Page as Page).GetIPAddress();
			return null;
		}

		public long GetIPAddressAsLong() {
			if (this.Page != null && this.Page is Page)
				return (this.Page as Page).GetIPAddressAsLong();
			return 0L;
		}
		#endregion
	}
}