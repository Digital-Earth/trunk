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
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class OnePageWondersSettings : HoytSoft.Common.Settings.ISectionHandler {
		#region Variables and Constants
		public const string
			DEFAULT = "default",
			SETTINGS_SECTION = "OnePageWonders";
		private Dictionary<string, Wonder> dictWonders = null;
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.OnePageWonders; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public Dictionary<string, Wonder> OnePageWonderObjects { get { return this.dictWonders; } }
		#endregion

		#region Load From Web.config
		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string name = null;
					string domains = null;
					string page = null;
					string origPage = null;

					foreach (XmlNode xn in xnl) {
						name = string.Empty;

						if (xn.Attributes["name"] != null)
							name = xn.Attributes["name"].Value;

						if (
							   xn.Attributes["domains"] != null
							&& xn.Attributes["page"] != null
							&& !string.IsNullOrEmpty((domains = xn.Attributes["domains"].Value))
							&& !string.IsNullOrEmpty((origPage = xn.Attributes["page"].Value))
						) {
							if (string.IsNullOrEmpty(name))
								name = DEFAULT;

							if (this.dictWonders == null)
								this.dictWonders = new Dictionary<string, Wonder>();

							page = origPage;
							if (System.Web.HttpContext.Current != null) {
								if (origPage.StartsWith("~/"))
									page = origPage.Replace("~/", (System.Web.HttpContext.Current.Request.ApplicationPath == "/" ? "/" : System.Web.HttpContext.Current.Request.ApplicationPath + "/"));
							}

							this.dictWonders[name] = new Wonder(name, origPage, page, domains.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries));
						}
					}
				}
				return this;
			}
			return null;
		}
		#endregion
	}
}
