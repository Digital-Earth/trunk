using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HoytSoft.Common.Configuration {
	public class HttpSecurity {
		public static string SecureSite {
			get {
				Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
				if (security != null)
					return security.SecureSite;
				else
					return string.Empty;
			}
		}

		public static string UnsecureSite {
			get {
				Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
				if (security != null)
					return security.UnsecureSite;
				else
					return string.Empty;
			}
		}

		public static string FindPathToSecureSite(string Path) {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			return security.FindPathToSecureSite(Path);
		}

		public static string FindPathToSecureSite(string Path, bool ShowReturnUrl) {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			return security.FindPathToSecureSite(Path, ShowReturnUrl);
		}

		public static string FindPathToUnsecureSite(string Path) {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			return security.FindPathToUnsecureSite(Path);
		}

		public static string FindPathToUnsecureSite(string Path, bool ShowReturnUrl) {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			return security.FindPathToUnsecureSite(Path, ShowReturnUrl);
		}

		public static void RedirectToSecureSite() {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			security.RedirectToSecureSite();
		}

		public static void RedirectFromSecureSite() {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			security.RedirectFromSecureSite();
		}

		public static void RedirectFromSecureSite(bool ShowReturnUrl) {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security == null)
				throw new ArgumentNullException("settings");
			security.RedirectFromSecureSite(ShowReturnUrl);
		}
	}
}
