#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class HttpSecuritySettings : HoytSoft.Common.Settings.ISectionHandler {
		public const string 
			DEFAULT = "default",
			SETTINGS_SECTION = "HttpSecurity";
		private string secureSite = "", unsecureSite = "";

		public Settings.Section SectionName { get { return Settings.Section.HttpSecurity; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public string SecureSite { get { return this.secureSite; } }
		public string UnsecureSite { get { return this.unsecureSite; } }

		private string FindPathToSite(string Site, string Path, bool ShowReturnUrl) {
			if (HttpContext.Current == null) return null;
			Path = Path.Replace("~/", HttpContext.Current.Request.ApplicationPath + (HttpContext.Current.Request.ApplicationPath.EndsWith(@"/") || HttpContext.Current.Request.ApplicationPath.EndsWith(@"\") ? "" : @"/"));
			//if (!HttpContext.Current.Request.IsSecureConnection) return Path;
			if (Path.StartsWith("/")) Path = Path.Substring(1);
			if (ShowReturnUrl) {
				if (Path.IndexOf('?') < 0) Path += '?';
				else Path += '&';

				Path = Site + Path + "ReturnUrl=";
				if (HttpContext.Current.Request["ReturnUrl"] == null)
					return Path + HttpUtility.UrlEncode(HttpContext.Current.Request.Url.PathAndQuery);
				else
					return Path + HttpUtility.UrlEncode(HttpContext.Current.Request["ReturnUrl"]);
			} else {
				//Rebuild the query string without the return url in it...
				Path = Site + Path;
				int len = HttpContext.Current.Request.QueryString.Count;
				string queryString = "";
				for (int i = 0; i < len; i++)
					if (!"ReturnUrl".Equals(HttpContext.Current.Request.QueryString.GetKey(i), StringComparison.InvariantCultureIgnoreCase))
						queryString += HttpContext.Current.Request.QueryString.GetKey(i) + "=" + HttpContext.Current.Request.QueryString.Get(i) + (i != len - 1 ? "&" : "");
				if (!string.IsNullOrEmpty(queryString))
					if (Path.IndexOf('?') < 0) Path += '?' + queryString;
					else Path += '&' + queryString;
				return Path;
			}
		}

		public string FindPathToSecureSite(string Path) { return FindPathToSecureSite(Path, true); }
		public string FindPathToSecureSite(string Path, bool ShowReturnUrl) {
			return FindPathToSite(this.secureSite, Path, ShowReturnUrl);
		}

		public string FindPathToUnsecureSite(string Path) { return FindPathToUnsecureSite(Path, true); }
		public string FindPathToUnsecureSite(string Path, bool ShowReturnUrl) {
			return FindPathToSite(this.unsecureSite, Path, ShowReturnUrl);
		}

		public void RedirectToSecureSite() {
			if (HttpContext.Current != null && !HttpContext.Current.Request.IsSecureConnection) {
				//string redir = Uri.UriSchemeHttps + @"://" + HttpContext.Current.Request.Url.Host + (HttpContext.Current.Request.Url.IsDefaultPort ? "" : ":" + HttpContext.Current.Request.Url.Port) + HttpContext.Current.Request.Url.PathAndQuery;
				string redir = this.secureSite + HttpContext.Current.Request.Url.PathAndQuery + (HttpContext.Current.Request["ReturnUrl"] == null ? (string.IsNullOrEmpty(HttpContext.Current.Request.Url.Query) ? '?' : '&') + "ReturnUrl=" + HttpUtility.UrlEncode(HttpContext.Current.Request.Path) : "");
				HttpContext.Current.Response.Redirect(redir, true);
			}
		}

		public void RedirectFromSecureSite() { RedirectFromSecureSite(true); }
		public void RedirectFromSecureSite(bool ShowReturnUrl) {
			if (HttpContext.Current != null && HttpContext.Current.Request.IsSecureConnection) {
				//string redir = Uri.UriSchemeHttp + @"://" + HttpContext.Current.Request.Url.Host + (HttpContext.Current.Request.Url.IsDefaultPort ? "" : ":" + HttpContext.Current.Request.Url.Port) + HttpContext.Current.Request.Url.PathAndQuery;
				//Generate query string
				string redir = this.unsecureSite + HttpContext.Current.Request.Url.AbsolutePath;
				if (ShowReturnUrl) redir = this.unsecureSite + HttpContext.Current.Request.Url.Query + (HttpContext.Current.Request["ReturnUrl"] == null ? +(string.IsNullOrEmpty(HttpContext.Current.Request.Url.Query) ? '?' : '&') + "ReturnUrl=" + HttpUtility.UrlEncode(HttpContext.Current.Request.Path) : "");
				else {
					int len = HttpContext.Current.Request.QueryString.Count;
					for (int i = 0; i < len; i++)
						redir += HttpContext.Current.Request.QueryString.GetKey(i) + "=" + HttpContext.Current.Request.QueryString.Get(i) + (i != len - 1 ? "&" : "");
				}
				HttpContext.Current.Response.Redirect(redir, true);
			}
		}

		public object Create(object Parent, object Context, XmlNode Section) {
			string testSecure, testUnsecure;

			if (Section.Attributes["secureSite"] != null && !string.IsNullOrEmpty((testSecure = Section.Attributes["secureSite"].Value)))
				this.secureSite = testSecure;

			if (Section.Attributes["unsecureSite"] != null && !string.IsNullOrEmpty((testUnsecure = Section.Attributes["unsecureSite"].Value)))
				this.unsecureSite = testUnsecure;

			this.secureSite.Replace('/', '\\');
			if (!this.secureSite.EndsWith(@"/"))
				this.secureSite += '/';

			this.unsecureSite.Replace('/', '\\');
			if (!this.unsecureSite.EndsWith(@"/"))
				this.unsecureSite += '/';

			return this;
		}

		internal class IgnoreCaseComparer : IEqualityComparer<string> {
			public bool Equals(string x, string y) { return x.Equals(y, StringComparison.InvariantCultureIgnoreCase); }

			public int GetHashCode(string obj) { return obj.GetHashCode(); }

		}
	}
}
