#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Configuration.Provider;
using System.Security.Permissions;
using System.Web;
using System.Web.UI.WebControls.WebParts;
using System.Collections.Specialized;
using System.Security.Cryptography;
using System.Text;
using System.IO;
using System.Net;

namespace HoytSoft.Common.Web.Providers {
	///<summary>
	/// This code is copyrighted Copyright © 2006 Erno de Weerd
	/// http://www.airknow.com/Tutorials/ASP_NET_2_0/CookiePersonalizationProvider/Page_01.aspx
	/// 
	/// All modifications are indicated by a comment. Anywhere "COOKIE_NAME" appears was also a 
	/// modification made by David Hoyt.
	///</summary>
	public class CookiePersonalizationProvider : PersonalizationProvider {
		/* Added by David Hoyt */
		private const string
			COOKIE_NAME = ".Personalization";

		private bool _useCookie = false;

		public override string ApplicationName {
			get { throw new NotSupportedException(); }
			set { throw new NotSupportedException(); }
		}

		/* Added by David Hoyt */
		public override string Name {
			get {
				return CookiePersonalizationProvider.ProviderName;
			}
		}

		/* Added by David Hoyt */
		public static string ProviderName {
			get {
				return typeof(CookiePersonalizationProvider).FullName;
			}
		}

		public override void Initialize(string name, NameValueCollection config) {
			// Verify that config isn't null
			if (config == null)
				throw new ArgumentNullException("config");

			// Assign the provider a default name if it doesn't have one
			if (String.IsNullOrEmpty(name))
				name = "CookiePersonalizationProvider";

			// Add a default "description" attribute to config if the
			// attribute doesn't exist or is empty
			if (string.IsNullOrEmpty(config["description"])) {
				config.Remove("description");
				config.Add("description", "Cookie personalization provider");
			}

			// Call the base class's Initialize method
			base.Initialize(name, config);

			// Throw an exception if unrecognized attributes remain
			if (config.Count > 0) {
				string attr = config.GetKey(0);
				if (!String.IsNullOrEmpty(attr))
					throw new ProviderException("Unrecognized attribute: " + attr);
			}

			//check for cookie availability:
			HttpContext context = HttpContext.Current;
			if (context != null) {
				_useCookie = (context.Session.CookieMode == HttpCookieMode.UseCookies);
			}
		}

		protected override void LoadPersonalizationBlobs(WebPartManager webPartManager, string path, string userName, ref byte[] sharedDataBlob, ref byte[] userDataBlob) {
			// Load shared state
			HttpContext context = HttpContext.Current;
			if (_useCookie) {
				HttpCookie cookie;
				cookie = context.Request.Cookies[COOKIE_NAME];
				if (cookie != null) {
					sharedDataBlob = null;
					if (cookie.Values[path] != null) {
						userDataBlob = Convert.FromBase64String(cookie.Values[path]);
					}
				}
			}
		}

		protected override void ResetPersonalizationBlob(WebPartManager webPartManager, string path, string userName) {
			// Delete the specified personalization 
			HttpContext context = HttpContext.Current;
			if (_useCookie) {
				context.Response.Cookies.Remove(COOKIE_NAME);
			}
		}

		protected override void SavePersonalizationBlob(WebPartManager webPartManager, string path, string userName, byte[] dataBlob) {
			HttpContext context = HttpContext.Current;
			if (_useCookie) {
				HttpCookie cookie;
				cookie = context.Request.Cookies[COOKIE_NAME];
				if (cookie == null) {
					cookie = new HttpCookie(COOKIE_NAME);
				}
				cookie.Values[path] = Convert.ToBase64String(dataBlob);
				cookie.Expires = DateTime.Now.AddYears(1);
				context.Response.Cookies.Set(cookie);
			}
		}

		public override PersonalizationStateInfoCollection FindState(PersonalizationScope scope, PersonalizationStateQuery query, int pageIndex, int pageSize, out int totalRecords) {
			throw new NotSupportedException();
		}

		public override int GetCountOfState(PersonalizationScope scope, PersonalizationStateQuery query) {
			throw new NotSupportedException();
		}

		public override int ResetState(PersonalizationScope scope, string[] paths, string[] usernames) {
			throw new NotSupportedException();
		}

		public override int ResetUserState(string path, DateTime userInactiveSinceDate) {
			throw new NotSupportedException();
		}


	}
}

