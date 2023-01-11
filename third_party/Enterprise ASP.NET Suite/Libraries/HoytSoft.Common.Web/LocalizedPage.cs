#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Globalization;
using System.Web;
using HoytSoft.Common.Configuration;

namespace HoytSoft.Common.Web {
	#region Delegates
	public delegate void CheckCultureDelegate(LocalizedPage Page);
	#endregion

	[PublicPage]
	public class PublicSharedLocalizedPage : LocalizedPage {
	}

	public class LocalizedPage : HoytSoft.Common.Web.Page {
		protected override void InitializeCulture() {
			base.InitializeCulture();

			//Here's where we do our magic...
			Configuration.Internal.I18NSettings i18n = Settings.From<Configuration.Internal.I18NSettings>(Settings.Section.I18N);
			if (i18n == null || string.IsNullOrEmpty(i18n.CookieName))
				return;

			//Does the cookie exist?
			if (Request.Cookies[i18n.CookieName] != null) {
				string value = Request.Cookies[i18n.CookieName].Value;
				int lcid = 0;

				if (!string.IsNullOrEmpty(value) && int.TryParse(value, out lcid)) {
					//It does, so find the culture they want and set it accordingly...
					Thread.CurrentThread.CurrentUICulture = Thread.CurrentThread.CurrentCulture = CultureInfo.GetCultureInfo(lcid);

					//Updated sliding expiration...
					if (i18n.CookieSlidingExpiration && Response.Cookies[i18n.CookieName] != null) {
						Response.Cookies[i18n.CookieName].Value = value; //You have to set the value of the cookie again...
						Response.Cookies[i18n.CookieName].Expires = DateTime.Now.AddDays(i18n.CookieTimeoutDays);
					}
				} else {
					//Invalid language - so set to default...
					Thread.CurrentThread.CurrentCulture = CultureInfo.CreateSpecificCulture(i18n.DefaultCulture);
					Thread.CurrentThread.CurrentUICulture = new CultureInfo(i18n.DefaultCulture);
				}
			} else {
				try {
					if (!i18n.UseBrowserCulture && !string.IsNullOrEmpty(i18n.DefaultCulture)) {
						//Set to default
						Thread.CurrentThread.CurrentCulture = CultureInfo.CreateSpecificCulture(i18n.DefaultCulture);
						Thread.CurrentThread.CurrentUICulture = new CultureInfo(i18n.DefaultCulture);
					} else if (i18n.UseBrowserCulture) {
						//Is one of the browser's culture in our allowable list?
						if (Request.UserLanguages != null && Request.UserLanguages.Length > 0) {
							CultureInfo ci = null;
							foreach(string lang in Request.UserLanguages) {
								if ((ci = getCultureInfo(lang, true)) == null || (ci = CultureInfo.CreateSpecificCulture(ci.Name)) == null)
									continue;
								if (I18N.FindSupportedCultureByLCID(ci.LCID) == null)
									continue;

								Thread.CurrentThread.CurrentUICulture = Thread.CurrentThread.CurrentCulture = ci;
								return;
							}
						}

						//If we couldn't load one of the browser's languages, then see if we can use the default culture...
						//If we still can't, we'll end up using the server's default culture...
						if (!string.IsNullOrEmpty(i18n.DefaultCulture)) {
							Thread.CurrentThread.CurrentCulture = CultureInfo.CreateSpecificCulture(i18n.DefaultCulture);
							Thread.CurrentThread.CurrentUICulture = new CultureInfo(i18n.DefaultCulture);
						}
					}
				} catch {
				}
			}
		}

		protected static CultureInfo getCultureInfo(string userLanguage, bool useUserOverride) {
			try { 
				return new CultureInfo(cleanBrowserUserLanguage(userLanguage), useUserOverride); 
			} catch (ArgumentException) { 
				return null; 
			}
		}

		protected static string cleanBrowserUserLanguage(string value) {
			int semiColonIndex = value.IndexOf(";");
			if (semiColonIndex != -1)
				value = value.Substring(0, semiColonIndex);
			return value;
		}

		protected virtual void OnLoadI18N() {
		}

		protected override void OnInitComplete(EventArgs e) {
			base.OnInitComplete(e);
			OnLoadI18N();
		}

		public bool SaveCulture(string CultureName) {
			return SaveCulture(CultureName, true);
		}

		public bool SaveCulture(int LCID) {
			return SaveCulture(LCID, true);
		}

		public bool SaveCulture(CultureInfo Culture) {
			return SaveCulture(Culture, true);
		}

		public bool SaveCulture(string CultureName, bool SetCurrentCulture) {
			try {
				return SaveCulture(CultureInfo.CreateSpecificCulture(CultureName), SetCurrentCulture);
			} catch {
				return false;
			}
		}

		public bool SaveCulture(int LCID, bool SetCurrentCulture) {
			try {
				return SaveCulture(new CultureInfo(LCID), SetCurrentCulture);
			} catch {
				return false;
			}
		}

		public bool SaveCulture(CultureInfo Culture, bool SetCurrentCulture) {
			if (Culture == null)
				return false;

			Configuration.Internal.I18NSettings i18n = Settings.From<Configuration.Internal.I18NSettings>(Settings.Section.I18N);
			if (i18n == null || string.IsNullOrEmpty(i18n.CookieName))
				return false;

			if (Response.Cookies[i18n.CookieName] == null) {
				HttpCookie cookie = new HttpCookie(i18n.CookieName, Culture.LCID.ToString());
				cookie.Expires = DateTime.Now.AddDays(i18n.CookieTimeoutDays);
				Response.Cookies.Add(cookie);
			} else {
				Response.Cookies[i18n.CookieName].Value = Culture.LCID.ToString();
				if (i18n.CookieSlidingExpiration)
					Response.Cookies[i18n.CookieName].Expires = DateTime.Now.AddDays(i18n.CookieTimeoutDays);
			}

			if (SetCurrentCulture) {
				Thread.CurrentThread.CurrentCulture = Culture;
				Thread.CurrentThread.CurrentUICulture = Culture;
			}
			return true;
		}

		public SupportedCulture FindCurrentSupportedCulture() {
			//Try current culture first...
			CultureInfo ci = Thread.CurrentThread.CurrentCulture;
			if (ci != null)
				return I18N.FindSupportedCultureByLCID(ci.LCID);

			//Try UI culture now...
			ci = Thread.CurrentThread.CurrentUICulture;
			if (ci != null)
				return I18N.FindSupportedCultureByLCID(ci.LCID);

			try {
				//Now try looking it up based on settings...
				Configuration.Internal.I18NSettings i18n = Settings.From<Configuration.Internal.I18NSettings>(Settings.Section.I18N);
				if (i18n == null)
					return null;

				//Do we have the cookie defined? We intentionally continue on in case we can't find one 
				//so we can load in the default culture...
				if (!string.IsNullOrEmpty(i18n.CookieName)) {
					int lcid = 0;
					if (Request.Cookies[i18n.CookieName] != null && int.TryParse(Request.Cookies[i18n.CookieName].Value, out lcid))
						return I18N.FindSupportedCultureByLCID(lcid);
				}

				//Okay, do we have a default culture defined?
				if (i18n == null || string.IsNullOrEmpty(i18n.DefaultCulture))
					return I18N.FindSupportedCultureByLCID(CultureInfo.GetCultureInfo(i18n.DefaultCulture).LCID);
			} catch {
			}

			//At this point, there's nothing left to check...
			return null;
		}
	}
}
