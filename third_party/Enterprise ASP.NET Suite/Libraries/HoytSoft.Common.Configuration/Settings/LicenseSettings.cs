#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using System.Collections.Generic;
using HoytSoft.Common;

namespace HoytSoft.Common.Configuration.Internal {
	public class LicenseSettings : Settings.ISectionHandler {
		#region Constants
		public const string 
			SETTINGS_SECTION = "Licenses"
		;
		private const string
			ATTRIB_ADD_NAME = "name",
			ATTRIB_ADD_LOAD_ON_STARTUP = "loadOnStartup",
			ATTRIB_ADD_TYPE_IN_ASSEMBLY = "typeInAssembly", 
			ATTRIB_CODE_LANGUAGE = "language"
		;
		private const string
			TAG_KEY = "Key",
			TAG_CODE = "Code"
		;
		#endregion

		#region Variables
		private static bool loaded = false;
		private static LicenseSettings singleton = null;
		private int licensesCount = 0;
		private System.Collections.Generic.IList<ILicense> licenses = null;
		#endregion

		#region Constructors
		static LicenseSettings() {
		}
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.Licenses; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public static IList<ILicense> Licenses { get { if (!loaded) Reload(); if (singleton != null) return singleton.licenses; else return null; } }
		public static int LicenseCount { get { if (!loaded) Reload(); if (singleton != null) return singleton.licensesCount; else return 0; } }
		#endregion

		#region Public Methods
		public static void Reload() {
			loaded = true;
			singleton = (LicenseSettings)Settings.From(Settings.Section.Licenses);
		}

		public static void AddLicense(ILicense License) {
			if (!loaded) Reload();
			if (License == null || singleton == null) return;
			if (singleton.licenses == null)
				singleton.licenses = new List<ILicense>();
			singleton.licenses.Add(License);
		}

		public static void RemoveLicense(ILicense License) {
			if (!loaded) Reload();
			if (License == null || singleton == null || singleton.licenses == null || !singleton.licenses.Contains(License)) return;
			singleton.licenses.Remove(License);
		}

		public static void ClearLicenses(ILicense License) {
			if (!loaded) Reload();
			if (License == null || singleton == null || singleton.licenses == null) return;
			singleton.licenses.Clear();
		}
		#endregion

		#region IConfigurationSectionHandler
		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				XmlNode node = null;

				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string name = string.Empty;
					string key = string.Empty;
					string code = string.Empty;
					string typeInAssembly = string.Empty;
					string sLanguage = string.Empty;
					string sLoadOnStartup = string.Empty;
					bool loadOnStartup = false;

					foreach (XmlNode xn in xnl) {
						name = string.Empty;
						key = string.Empty;
						code = string.Empty;
						typeInAssembly = string.Empty;
						sLanguage = string.Empty;
						sLoadOnStartup = string.Empty;
						loadOnStartup = false;

						if (xn.Attributes[ATTRIB_ADD_NAME] != null && !string.IsNullOrEmpty(xn.Attributes[ATTRIB_ADD_NAME].Value))
							name = xn.Attributes[ATTRIB_ADD_NAME].Value;
						if (xn.Attributes[ATTRIB_ADD_TYPE_IN_ASSEMBLY] != null && !string.IsNullOrEmpty(xn.Attributes[ATTRIB_ADD_TYPE_IN_ASSEMBLY].Value))
							typeInAssembly = xn.Attributes[ATTRIB_ADD_TYPE_IN_ASSEMBLY].Value;
						if (xn.Attributes[ATTRIB_ADD_LOAD_ON_STARTUP] != null && !string.IsNullOrEmpty(xn.Attributes[ATTRIB_ADD_LOAD_ON_STARTUP].Value))
							sLoadOnStartup = xn.Attributes[ATTRIB_ADD_LOAD_ON_STARTUP].Value;
						if (!string.IsNullOrEmpty(sLoadOnStartup))
							bool.TryParse(sLoadOnStartup, out loadOnStartup);
						
						//Get key...
						if ((node = xn.SelectSingleNode(TAG_KEY)) != null) {
							key = node.InnerText;
						}

						//Check assembly
						//if (System.Web.HttpContext.Current != null) {
						//	if (typeInAssembly.StartsWith("~/"))
						//		typeInAssembly = typeInAssembly.Replace("~/", System.Web.HttpContext.Current.Request.PhysicalApplicationPath);
						//}

						//Load type in assembly...
						Type tInAssembly = null;
						if (!string.IsNullOrEmpty(typeInAssembly) && (tInAssembly = Type.GetType(typeInAssembly, false, true)) != null) {
						}

						//Get code...
						if ((node = xn.SelectSingleNode(TAG_CODE)) != null) {
							code = node.InnerText;
							if (node.Attributes[ATTRIB_CODE_LANGUAGE] != null && !string.IsNullOrEmpty(node.Attributes[ATTRIB_CODE_LANGUAGE].Value))
								sLanguage = node.Attributes[ATTRIB_CODE_LANGUAGE].Value;
						}

						if (!string.IsNullOrEmpty(code))
							code = code.Replace("{Key}", key).Replace("{Name}", name);

						if (this.licenses == null)
							this.licenses = new List<ILicense>();
						this.licenses.Add(new License(name, key, loadOnStartup, tInAssembly, sLanguage, code));
					}
				}
			}
			return this;
		}
		#endregion
	}
}
