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
	public class BrandingSettings : HoytSoft.Common.Settings.ISectionHandler {
		public const string 
			DEFAULT = "default",
			SETTINGS_SECTION = "Branding",
			ATTRIB_COMPANY_SETTINGS = "companySettings";
		private string origCompanySettings = "", companySettings = "", virtCompanySettings;

		public Settings.Section SectionName			{ get { return Settings.Section.Branding; } }
		public string ConfigSectionName				{ get { return SETTINGS_SECTION; } }
		public string OriginalCompanySettings		{ get { return this.origCompanySettings; } }
		public string CompanySettings				{ get { return this.companySettings; } }
		public string VirtualPathCompanySettings	{ get { return this.virtCompanySettings; } }

		public object Create(object Parent, object Context, XmlNode Section) {
			string testOrigCompanySettings;

			if (Section.Attributes[ATTRIB_COMPANY_SETTINGS] != null && !string.IsNullOrEmpty((testOrigCompanySettings = Section.Attributes[ATTRIB_COMPANY_SETTINGS].Value)))
				this.origCompanySettings = testOrigCompanySettings;

			this.origCompanySettings = this.origCompanySettings.Replace('\\', '/');
			if (!this.origCompanySettings.EndsWith(@"/"))
				this.origCompanySettings += '/';

			if (HttpContext.Current != null)
				this.companySettings = HttpContext.Current.Server.MapPath(this.origCompanySettings);
			else
				this.companySettings = this.origCompanySettings;

			if (this.origCompanySettings.StartsWith("~/") && HttpContext.Current != null)
				this.virtCompanySettings = this.origCompanySettings.Replace("~/", (System.Web.HttpContext.Current.Request.ApplicationPath == "/" ? "/" : System.Web.HttpContext.Current.Request.ApplicationPath + "/"));
			else
				this.virtCompanySettings = this.origCompanySettings;

			return this;
		}

	} //class
} //namespace
