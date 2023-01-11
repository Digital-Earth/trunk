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
	public class ConnectionStringSettings : HoytSoft.Common.Settings.ISectionHandler {
		public const string 
			DEFAULT = "default",
			SETTINGS_SECTION = "ConnectionStrings";
		private Dictionary<string, string> dictConnStr = null;

		public Settings.Section SectionName { get { return Settings.Section.ConnectionString; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public Dictionary<string, string> ConnectionStrings { get { return this.dictConnStr; } }

		public string LookUp(string Name) { return this[Name]; }
		public string this[string Key] { 
			get {
				//A null key means to get the default connection string...
				if (string.IsNullOrEmpty(Key)) {
					if (this.dictConnStr == null || !this.dictConnStr.ContainsKey(DEFAULT))
						return null;
					return this.dictConnStr[DEFAULT];
				} else {
					if (this.dictConnStr == null || !this.dictConnStr.ContainsKey(Key))
						return null;
					return this.dictConnStr[Key];
				}
			} 
			set {
				if (this.dictConnStr == null)
					this.dictConnStr = new Dictionary<string, string>(new IgnoreCaseComparer());
				if (this.dictConnStr.ContainsKey(Key))
					this.dictConnStr.Remove(Key);
				this.dictConnStr.Add(Key, value);
			} 
		}

		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string name = null;
					string connStr = null;
					foreach (XmlNode xn in xnl) {
						if (xn.Attributes["name"] != null)
							name = xn.Attributes["name"].Value;

						if (
							   xn.Attributes["connectionString"] != null
							&& !string.IsNullOrEmpty((connStr = xn.Attributes["connectionString"].Value))
						) {
							if (this.dictConnStr == null)
								this.dictConnStr = new Dictionary<string,string>(new IgnoreCaseComparer());
							if (System.Web.HttpContext.Current != null) {
								if (connStr.Contains("~/"))
									connStr = connStr.Replace("~/", System.Web.HttpContext.Current.Request.PhysicalApplicationPath);
							}

							if (string.IsNullOrEmpty(name))
								name = DEFAULT;
							if (this.dictConnStr.ContainsKey(name))
								this.dictConnStr.Remove(name);
							this.dictConnStr.Add(name, connStr);
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
