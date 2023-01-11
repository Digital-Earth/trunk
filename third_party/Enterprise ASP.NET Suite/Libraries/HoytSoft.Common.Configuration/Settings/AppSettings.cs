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

namespace HoytSoft.Common.Configuration.Internal {
	public class AppSettings : HoytSoft.Common.Settings.ISectionHandler {
		public const string 
			SETTINGS_SECTION = "AppSettings";
		private Dictionary<string, string> dictSettings = null;

		public Settings.Section SectionName { get { return Common.Settings.Section.AppSettings; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public Dictionary<string, string> Settings { get { return this.dictSettings; } }

		public string LookUp(string Name) { return this[Name]; }
		public string this[string Key] { 
			get {
				//A null key means to get the default connection string...
				if (string.IsNullOrEmpty(Key) || this.dictSettings == null || !this.dictSettings.ContainsKey(Key)) {
					return null;
				} else {
					return this.dictSettings[Key];
				}
			} 
			set {
				if (this.dictSettings == null)
					this.dictSettings = new Dictionary<string, string>(new IgnoreCaseComparer());
				if (this.dictSettings.ContainsKey(Key))
					this.dictSettings.Remove(Key);
				this.dictSettings.Add(Key, value);
			} 
		}

		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string name = null;
					string val = null;
					foreach (XmlNode xn in xnl) {
						if (xn.Attributes["name"] != null)
							name = xn.Attributes["name"].Value;

						if (
							   xn.Attributes["value"] != null
							&& !string.IsNullOrEmpty((val = xn.Attributes["value"].Value))
						) {
							if (this.dictSettings == null)
								this.dictSettings = new Dictionary<string,string>(new IgnoreCaseComparer());
							if (System.Web.HttpContext.Current != null) {
								if (val.Contains("~/"))
									val = val.Replace("~/", System.Web.HttpContext.Current.Request.PhysicalApplicationPath);
							}

							if (!string.IsNullOrEmpty(name))
								this.dictSettings[name] = val;
						}
					}
				}
				return this;
			}
			return null;
		}

		internal class IgnoreCaseComparer : IEqualityComparer<string> {
			public bool Equals(string x, string y) { return x.Equals(y, StringComparison.InvariantCultureIgnoreCase); }

			public int GetHashCode(string obj) { return obj.GetHashCode(); }

		}
	}
}
