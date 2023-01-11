#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class I18NSettings : HoytSoft.Common.Settings.ISectionHandler {
		#region Constants
		public const string 
			DEFAULT						= "default",
			SETTINGS_SECTION			= "I18N",
			ATTRIB_DEFAULT_CULTURE		= "defaultCulture", 
			ATTRIB_COOKIE_NAME			= "cookieName",
			ATTRIB_COOKIE_TIMEOUT_DAYS	= "cookieTimeoutDays",
			ATTRIB_COOKIE_SLIDING_EXP	= "cookieSlidingExpiration", 
			ATTRIB_ENABLED				= "enabled", 
			ATTRIB_USE_BROWSER_CULTURE	= "useBrowserCulture"
		;
		#endregion

		#region Variables
		private string defaultCulture = "en-US";
		private string cookieName = ".Culture";
		private bool enabled = true;
		private bool useBrowserCulture = true;
		private bool cookieSlidingExpiration = true;
		private int cookieTimeoutDays = 366;
		#endregion

		public Settings.Section SectionName	{ get { return Settings.Section.I18N; } }
		public string ConfigSectionName		{ get { return SETTINGS_SECTION; } }
		public string DefaultCulture		{ get { return this.defaultCulture; } }
		public string CookieName			{ get { return this.cookieName; } }
		public bool Enabled					{ get { return this.enabled; } }
		public bool UseBrowserCulture		{ get { return this.useBrowserCulture; } }
		public int CookieTimeoutDays		{ get { return this.cookieTimeoutDays; } }
		public bool CookieSlidingExpiration { get { return this.cookieSlidingExpiration; } }

		public object Create(object Parent, object Context, XmlNode Section) {
			string testDefaultCulture, testCookieName, testEnabled, testUseBrowserCulture, testCookieSlidingExpiration, testCookieTimeoutDays;
			bool bEnabled, bUseBrowserCulture, bCookieSlidingExpiration;
			int iCookieTimeoutDays;

			if (Section.Attributes[ATTRIB_DEFAULT_CULTURE] != null && !string.IsNullOrEmpty((testDefaultCulture = Section.Attributes[ATTRIB_DEFAULT_CULTURE].Value)))
				this.defaultCulture = testDefaultCulture;

			if (Section.Attributes[ATTRIB_COOKIE_NAME] != null && !string.IsNullOrEmpty((testCookieName = Section.Attributes[ATTRIB_COOKIE_NAME].Value)))
				this.cookieName = testCookieName;

			if (Section.Attributes[ATTRIB_ENABLED] != null && !string.IsNullOrEmpty((testEnabled = Section.Attributes[ATTRIB_ENABLED].Value)) && bool.TryParse(testEnabled.ToLower(), out bEnabled))
				this.enabled = bEnabled;

			if (Section.Attributes[ATTRIB_USE_BROWSER_CULTURE] != null && !string.IsNullOrEmpty((testUseBrowserCulture = Section.Attributes[ATTRIB_USE_BROWSER_CULTURE].Value)) && bool.TryParse(testUseBrowserCulture.ToLower(), out bUseBrowserCulture))
				this.useBrowserCulture = bUseBrowserCulture;

			if (Section.Attributes[ATTRIB_COOKIE_SLIDING_EXP] != null && !string.IsNullOrEmpty((testCookieTimeoutDays = Section.Attributes[ATTRIB_COOKIE_SLIDING_EXP].Value)) && bool.TryParse(testCookieTimeoutDays.ToLower(), out bCookieSlidingExpiration))
				this.cookieSlidingExpiration = bCookieSlidingExpiration;

			if (Section.Attributes[ATTRIB_COOKIE_TIMEOUT_DAYS] != null && !string.IsNullOrEmpty((testCookieTimeoutDays = Section.Attributes[ATTRIB_COOKIE_TIMEOUT_DAYS].Value)) && int.TryParse(testCookieTimeoutDays, out iCookieTimeoutDays))
				this.cookieTimeoutDays = iCookieTimeoutDays;

			return this;
		}

	} //class
} //namespace
