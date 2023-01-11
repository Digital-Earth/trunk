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
	public class VirtualDirectorySettings : HoytSoft.Common.Settings.ISectionHandler {
		private VirtualDirectoryDictionary dictDirs = null;

		public Settings.Section SectionName { get { return Settings.Section.VirtualDirectory; } }
		public string ConfigSectionName { get { return "VirtualDirectories"; } }
		public VirtualDirectoryDictionary VirtualDirectories { get { return this.dictDirs; } }

		public string Find(string URL, ref int URLLocation, ref string Path) {
			if (this.dictDirs == null)
				return null;
			return this.dictDirs.FindAssociatedKey(URL, ref URLLocation, ref Path);
		}

		public object Create(object Parent, object Context, XmlNode Section) {
			//Just for use when compiling into the "Full Compilation" directory - has no use anywhere else...
			if (System.Web.HttpContext.Current != null && System.Web.HttpContext.Current.Request.PhysicalApplicationPath.EndsWith(@"Full Compilation\Website\")) {
				if (this.dictDirs == null)
					this.dictDirs = new VirtualDirectoryDictionary();
				this.dictDirs.Add("common/theme/", System.IO.Path.Combine(System.Web.HttpContext.Current.Request.PhysicalApplicationPath, @"common\themes\2.0\"));
				this.dictDirs.Add("common/", System.IO.Path.Combine(System.Web.HttpContext.Current.Request.PhysicalApplicationPath, @"common\"));
			}

			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string url = null;
					string path = null;
					foreach (XmlNode xn in xnl) {
						if (
							   xn.Attributes["url"] != null
							&& xn.Attributes["path"] != null
							&& !string.IsNullOrEmpty((url = xn.Attributes["url"].Value))
							&& !string.IsNullOrEmpty((path = xn.Attributes["path"].Value))
						) {
							if (this.dictDirs == null)
								this.dictDirs = new VirtualDirectoryDictionary();
							if (System.Web.HttpContext.Current != null) {
								if (path.StartsWith("~/"))
									path = path.Replace("~/", System.Web.HttpContext.Current.Request.PhysicalApplicationPath);
								if (url.StartsWith("~/"))
									url = url.Replace("~/", (System.Web.HttpContext.Current.Request.ApplicationPath == "/" ? "/" : System.Web.HttpContext.Current.Request.ApplicationPath + "/"));
							}
							
							path = path.Replace('/', System.IO.Path.DirectorySeparatorChar);
							if (!System.IO.Directory.Exists(path))
								continue;
								//throw new System.IO.DirectoryNotFoundException("'" + path + "' is not a valid physical directory for a virtual directory");

							System.IO.DirectoryInfo di = new System.IO.DirectoryInfo(path);
							this.dictDirs.Add(url, di.FullName);
						}
					}
				}
				return this;
			}
			return null;
		}

		#region Helper Classes
		public class VirtualDirectoryDictionary : System.Collections.Hashtable {
			public void Add(string Key, string Value) { base.Add(Key, Value); }
			public bool Contains(string Key) { return base.Contains(Key); }
			public bool ContainsValue(string Value) { return base.ContainsValue(Value); }
			public string this[string Key] { get { return (string)base[Key]; } set { base[Key] = value; } }
			public override bool ContainsKey(object Key) {
				return this.ContainsKey((string)Key);
			}

			public bool ContainsKey(string Key) {
				if (base.ContainsKey(Key))
					return true;

				foreach (string k in this.Keys) {
					if (Key.StartsWith(k))
						return true;
				}
				return false;
			}

			public string FindAssociatedKey(string SearchKey, ref int SearchKeyIndex, ref string Value) {
				if (base.ContainsKey(SearchKey))
					return SearchKey;

				int loc = -1;
				foreach (string k in this.Keys) {
					if ((loc = SearchKey.LastIndexOf(k)) >= 0) {
						SearchKeyIndex = loc;
						Value = this[k];
						return k;
					}
				}
				return null;
			}
			
		}
		#endregion
	}
}
