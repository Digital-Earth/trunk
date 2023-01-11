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

namespace HoytSoft.Common.Web.Providers {
	public class MembershipProvider : System.Web.Security.MembershipProvider {
		#region MembershipProvider
		public override string Name {
			get {
				return typeof(Common.Web.Providers.MembershipProvider).FullName; 
			}
		}

		public override string ApplicationName {
			get {
				if (System.Web.HttpContext.Current != null)
					return System.Web.HttpContext.Current.Request.ApplicationPath;
				return string.Empty;
			}
			set { }
		}

		public override bool ChangePassword(string username, string oldPassword, string newPassword) {
			return true;
		}

		public override bool ChangePasswordQuestionAndAnswer(string username, string password, string newPasswordQuestion, string newPasswordAnswer) {
			return true;
		}

		public override MembershipUser CreateUser(string username, string password, string email, string passwordQuestion, string passwordAnswer, bool isApproved, object providerUserKey, out System.Web.Security.MembershipCreateStatus status) {
			status = MembershipCreateStatus.UserRejected;
			return null;
		}

		public override bool DeleteUser(string username, bool deleteAllRelatedData) {
			return true;
		}

		public override bool EnablePasswordReset {
			get { return false; }
		}

		public override bool EnablePasswordRetrieval {
			get { return false; }
		}

		public override MembershipUserCollection FindUsersByEmail(string emailToMatch, int pageIndex, int pageSize, out int totalRecords) {
			totalRecords = 0;
			return new MembershipUserCollection();
		}

		public override MembershipUserCollection FindUsersByName(string usernameToMatch, int pageIndex, int pageSize, out int totalRecords) {
			totalRecords = 0;
			return new MembershipUserCollection();
		}

		public override MembershipUserCollection GetAllUsers(int pageIndex, int pageSize, out int totalRecords) {
			totalRecords = 0;
			return new MembershipUserCollection();
		}

		public override int GetNumberOfUsersOnline() {
			return 0;
		}

		public override string GetPassword(string username, string answer) {
			return string.Empty;
		}

		public override MembershipUser GetUser(string username, bool userIsOnline) {
			if (HttpContext.Current != null && HttpContext.Current.Session[Authentication.SESSION_USER_KEY] != null)
				return (MembershipUser)HttpContext.Current.Session[Authentication.SESSION_USER_KEY];
			return null;
		}

		public override MembershipUser GetUser(object providerUserKey, bool userIsOnline) {
			if (HttpContext.Current != null && HttpContext.Current.Session[Authentication.SESSION_USER_KEY] != null)
				return (MembershipUser)HttpContext.Current.Session[Authentication.SESSION_USER_KEY];
			return null;
		}

		public override string GetUserNameByEmail(string email) {
			return string.Empty;
		}

		public override int MaxInvalidPasswordAttempts {
			get { return 0; }
		}

		public override int MinRequiredNonAlphanumericCharacters {
			get { return 3; }
		}

		public override int MinRequiredPasswordLength {
			get { return 4; }
		}

		public override int PasswordAttemptWindow {
			get { return 3; }
		}

		public override MembershipPasswordFormat PasswordFormat {
			get { return MembershipPasswordFormat.Hashed; }
		}

		public override string PasswordStrengthRegularExpression {
			get { return string.Empty; }
		}

		public override bool RequiresQuestionAndAnswer {
			get { return false; }
		}

		public override bool RequiresUniqueEmail {
			get { return true; }
		}

		public override string ResetPassword(string username, string answer) {
			return string.Empty;
		}

		public override bool UnlockUser(string userName) {
			return true;
		}

		public override void UpdateUser(System.Web.Security.MembershipUser user) {
		}

		public override bool ValidateUser(string username, string password) {
			if (HttpContext.Current != null && HttpContext.Current.Session[Authentication.SESSION_USER_KEY] != null)
				return true;
			if (HttpContext.Current.CurrentHandler is Page)
				return (HttpContext.Current.CurrentHandler as Page).ValidateLogin(username, password);
			return false;
		}
		#endregion
	}
}
