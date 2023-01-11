#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using System.Configuration;
#if FRAMEWORK_1_1
using System.Collections;
#else
using System.Collections.Generic;
using System.IO;
using System.Web;
#endif

namespace HoytSoft.Common {

	public class Settings : IConfigurationSectionHandler {
		#region Constants
		public const string
			DEFAULT_NAME = "default", 
			DEFAULT_SETTINGS_CONFIG_FILE = "Web.common.config"
		;
		#endregion

		#region Enums
		public enum Section : byte {
			Other				= 0, 
			Website				= 1,
			VirtualDirectory	= 2, 
			ServerControls		= 3, 
			Application			= 4, 
			ConnectionString	= 5, 
			LinkPoint			= 6, 
			CustomErrors		= 7,
			HttpSecurity		= 8, 
			Cryptography		= 9, 
			Branding			= 10,
			AppSettings			= 11, 
			Email				= 12, 
			Demos				= 13, 
			SystemEvents		= 14, 
			OnePageWonders		= 15,
			Licenses			= 16,
			I18N				= 17, 
			URLRewrite			= 18, 
			Authentication		= 19
		}
		#endregion

		#region Interfaces
		public interface ISectionHandler : IConfigurationSectionHandler {
			Section SectionName { get; }
			string ConfigSectionName { get; }
		}
		#endregion

		#region Variables
		///<summary>The name of the XML tag holding specific module configuration settings.</summary>
		public static readonly string SectionName = typeof(Settings).FullName;
		private static bool debugMode = false, testMode = false;
		private static bool loaded = false;
		private static Configuration.EmailTemplates email = new Configuration.EmailTemplates();
		#if FRAMEWORK_1_1
		private static Hashtable mySettings = new Hashtable();
		private static Hashtable dictText = null;
		#else
		private static Dictionary<Section, object> mySettings = new Dictionary<Section, object>();
		private static Dictionary<string, string> dictText = null;
		#endif
		#endregion

		#region Properties
		///<summary>Returns if debugMode attribute was set to true/false.</summary>
		public static bool DebugMode { get { if (!loaded) Load(); return debugMode; } }
		///<summary>Returns if testMode attribute was set to true/false.</summary>
		public static bool TestMode { get { if (!loaded) Load(); return testMode; } }
		///<summary>Returns a dictionary containing application settings that did not fit into an XML tag of their own.</summary>
		public static Dictionary<string, string> AppSettings {
			get {
				if (!loaded) Load();
				Configuration.Internal.AppSettings s = From<Configuration.Internal.AppSettings>(Section.AppSettings);
				if (s != null)
					return s.Settings;
				else
					return null;
			}
		}

		public static Dictionary<string, string> Text {
			get {
				if (!loaded) Load();
				return dictText;
			}
		}

		public static Configuration.EmailTemplates EmailTemplates {
			get {
				if (!loaded) Load();
				return email;
			}
		}
		#endregion

		#region Public Methods
		///<summary>Ensures that we re-read the configuration file.</summary>
		public static void Reload() {
			Load();
		}

		public static void Reload(string File) {
			Load(File);
		}

		public static void Reload(FileInfo File) {
			Load(File);
		}

		public static void Reload(Stream stream) {
			Load(stream);
		}

		///<summary>Retrieves configuration information for the module specified by ModuleSectionName.</summary>
		/// <param name="ModuleSectionName">The XML tag name of the module whose configuration settings you're seeking.</param>
		/// <returns>An object created by the server control representing its settings.</returns>
		public static object From(Section SectionName) {
			//If we haven't read in all the controls, then go ahead and do so now...
			//This is lazy initialization - don't do anything we don't have to!
			if (!loaded)
				Load();

			//After loading we should have all the instantiated objects in the hash table...
			if (mySettings != null && mySettings.ContainsKey(SectionName))
				return mySettings[SectionName];
			return null;
		}

		#if FRAMEWORK_1_1
		#else
		public static T From<T>(Section SectionName) {
			return (T)From(SectionName);
		}
		#endif
		#endregion

		#region Helpers
		public object Create(object parent, object configContext, XmlNode section) {
			if (!SectionName.Equals(section.LocalName, StringComparison.InvariantCultureIgnoreCase))
				throw new ArgumentException("Invalid XmlNode for processing settings");

			if (section.HasChildNodes) {
				XmlNodeList xnl = null, xnl2 = null;

				//First read in any text nodes...
				if ((xnl = section.SelectNodes("Text/Add")) != null && xnl.Count > 0) {
					foreach (XmlNode xn in xnl) {
						if (
							   xn.Attributes["name"] != null
							&& !string.IsNullOrEmpty(xn.Attributes["name"].Value)
						) {
							//We have a name...
							if (dictText == null)
								dictText = new Dictionary<string, string>();
							dictText[xn.Attributes["name"].Value] = (!string.IsNullOrEmpty(xn.InnerText.Trim())) ? xn.InnerText.Trim() : string.Empty;
						}
					}
				}

				string machineName = System.Environment.MachineName;
				if ((xnl = section.SelectNodes("Machine")) != null && xnl.Count > 0) {
					XmlNodeList xnlDefault = null;
					XmlNodeList xnlMachineSpecific = null;

					//Find default first
					foreach (XmlNode xn in xnl) {
						if (
							   xn.Attributes["name"] == null 
							|| (
								   !string.IsNullOrEmpty(xn.Attributes["name"].Value)
								&& xn.Attributes["name"].Value.Equals(DEFAULT_NAME, StringComparison.InvariantCultureIgnoreCase)
							)
						) {
							//Get out the 2 values of debug and test mode...
							if (xn.Attributes["debugMode"] != null && !string.IsNullOrEmpty(xn.Attributes["debugMode"].Value))
								debugMode = bool.Parse(xn.Attributes["debugMode"].Value);
							if (xn.Attributes["testMode"] != null && !string.IsNullOrEmpty(xn.Attributes["testMode"].Value))
								testMode = bool.Parse(xn.Attributes["testMode"].Value);

							if ((xnl2 = xn.SelectNodes("child::*")) != null && xnl2.Count > 0) {
								xnlDefault = xnl2;
								break;
							}
						}
					}

					if (xnlDefault == null)
						throw new ArgumentOutOfRangeException("Missing default machine configuration");

					foreach (XmlNode xn in xnl) {
						if (
							   xn.Attributes["name"] != null
							&& !string.IsNullOrEmpty(xn.Attributes["name"].Value)
							&& xn.Attributes["name"].Value.Equals(machineName, StringComparison.InvariantCultureIgnoreCase)
						) {
							//Get out the 2 values of debug and test mode...
							if (xn.Attributes["debugMode"] != null && !string.IsNullOrEmpty(xn.Attributes["debugMode"].Value))
								debugMode = bool.Parse(xn.Attributes["debugMode"].Value);
							if (xn.Attributes["testMode"] != null && !string.IsNullOrEmpty(xn.Attributes["testMode"].Value))
								testMode = bool.Parse(xn.Attributes["testMode"].Value);

							if ((xnl2 = xn.SelectNodes("child::*")) != null && xnl2.Count > 0) {
								xnlMachineSpecific = xnl2;
								break;
							}
						}
					}

					//It's okay to have a machine-specific be null, but you must have a default configuration at least!
					return new CreateInfo(parent, configContext, xnlDefault, xnlMachineSpecific);
				}
			}
			return null;
		}

		private static void Load() {
			//Does the file exist at the (web) application's root?
			if (File.Exists(DEFAULT_SETTINGS_CONFIG_FILE)) {
				Load(DEFAULT_SETTINGS_CONFIG_FILE);
			} else {
				//Double check using Server.MapPath
				if (HttpContext.Current != null && File.Exists(HttpContext.Current.Server.MapPath("~/" + DEFAULT_SETTINGS_CONFIG_FILE))) {
					Load(HttpContext.Current.Server.MapPath("~/" + DEFAULT_SETTINGS_CONFIG_FILE));
					return;
				}
				//Try loading it from Web.config or app.config
				CreateInfo ci = null;
				#if FRAMEWORK_1_1
				ci = (CreateInfo)System.Configuration.ConfigurationSettings.GetConfig(Settings.SectionName);
				#else
				ci = (CreateInfo)System.Web.Configuration.WebConfigurationManager.GetSection(Settings.SectionName);
				if (ci == null)
					ci = (CreateInfo)System.Configuration.ConfigurationManager.GetSection(Settings.SectionName);
				#endif

				Load(ci);
			}
		}

		private static void Load(string File) {
			Load(new FileInfo(File));
		}

		private static void Load(FileInfo File) {
			if (File == null)
				throw new ArgumentNullException("File");
			if (!File.Exists)
				throw new ArgumentException("Invalid file specified: " + File.FullName);
			using (FileStream fs = File.OpenRead()) {
				Load(fs);
			}
		}

		private static void Load(Stream stream) {
			XmlDocument doc = new XmlDocument();
			doc.Load(stream);
			Settings s = new Settings();
			Load((CreateInfo)s.Create(null, null, doc.DocumentElement));
		}

		private static void Load(CreateInfo ci) {
			if (ci == null)
				throw new ArgumentNullException("ci");

			loaded = true;

			string sectionHandlerClass = typeof(ISectionHandler).FullName;

			//Find all the objects in this assembly that implement ISectionHandler...
			#if FRAMEWORK_1_1
			Hashtable dict = new Hashtable(1);
			#else
			IDictionary<string, ISectionHandler> dict = new Dictionary<string, ISectionHandler>(1);
			#endif

			//Searches for all types in the loaded assemblies that implement the ISectionHandler interface 
			//and adds it to the dict variable...
			foreach (System.Reflection.Assembly assem in System.AppDomain.CurrentDomain.GetAssemblies()) {
				//Assemblies to ignore - filter them out by skipping over them...
				//Note that if the assemblies ever change their name, this won't work anymore!
				if (
					   assem.FullName.StartsWith("System", StringComparison.InvariantCultureIgnoreCase)
					|| assem.FullName.StartsWith("mscorlib", StringComparison.InvariantCultureIgnoreCase)
					|| assem.FullName.StartsWith("Microsoft", StringComparison.InvariantCultureIgnoreCase)
					|| assem.FullName.StartsWith("VJSharpCodeProvider", StringComparison.InvariantCultureIgnoreCase)
					|| assem.FullName.StartsWith("CppCodeProvider", StringComparison.InvariantCultureIgnoreCase)
					|| assem.FullName.StartsWith("stdole", StringComparison.InvariantCultureIgnoreCase)
				)
					continue;

				Type[] ts = assem.GetTypes();
				foreach (Type t in ts) {
					Type[] tt = t.GetInterfaces();
					foreach (Type ti in tt) {
						if (ti.IsInterface && ti.FullName == sectionHandlerClass) {
							ISectionHandler ish = (ISectionHandler)Activator.CreateInstance(t);
							dict[ish.ConfigSectionName] = ish;
						}
					}
				}
			}

			//Now we go through and call create on all the instances of ISectionHandler we've found - 
			//essentially treating them like another section in the config manager...

			//Process default machine configuration first
			if (ci != null && ci.defaultResults != null && ci.defaultResults.Count > 0) {
				foreach (XmlNode xn in ci.defaultResults) {
					if (dict.ContainsKey(xn.LocalName)) {
						#if FRAMEWORK_1_1
						ISectionHandler ish = (ISectionHandler)dict[xn.LocalName];
						#else
						ISectionHandler ish = dict[xn.LocalName];
						#endif
						object o = ish.Create(ci.parent, ci.configContext, xn);
						if (o != null)
							mySettings[ish.SectionName] = o;
					}
				}
			}

			//Now process machine-specific configuration first, overriding anything in the default one
			if (ci != null && ci.machineSpecificResults != null && ci.machineSpecificResults.Count > 0) {
				foreach (XmlNode xn in ci.machineSpecificResults) {
					if (dict.ContainsKey(xn.LocalName)) {
						#if FRAMEWORK_1_1
						ISectionHandler ish = (ISectionHandler)dict[xn.LocalName];
						#else
						ISectionHandler ish = dict[xn.LocalName];
						#endif
						object o = ish.Create(ci.parent, ci.configContext, xn);
						if (o != null)
							mySettings[ish.SectionName] = o;
					}
				}
			}
		}
		#endregion
	}

	#region Helper Classes
	internal class CreateInfo {
		public object parent;
		public object configContext;
		public XmlNodeList defaultResults;
		public XmlNodeList machineSpecificResults;
		public CreateInfo(object parent, object configContext, XmlNodeList defaultResults, XmlNodeList machineSpecificResults) {
			this.parent = parent;
			this.configContext = configContext;
			this.defaultResults = defaultResults;
			this.machineSpecificResults = machineSpecificResults;
		}
	}
	#endregion

}
