using System;
using System.Xml;
using System.Configuration;
using HoytSoft.Common.Cryptography;

namespace HoytSoft.Common.Configuration.Internal {
	///<summary>Section handler for reading encryption configuration information from an xml config file.</summary>
	public class CryptographySettings : Settings.ISectionHandler {
		#region Constants
		///<summary>The name of the XML tag holding specific module configuration settings.</summary>
		public const string 
			SettingsName = "Cryptography"
		;
		#endregion

		public Settings.Section SectionName { get { return Settings.Section.Cryptography; } }
		public string ConfigSectionName { get { return SettingsName; } }

		#region IConfigurationSectionHandler Members
		public object Create(object parent, object configContext, XmlNode section) {
			ReturnInfo ri = null;

			if (section != null && section.Attributes["cipherType"] != null) {
				Type t = Type.GetType(section.Attributes.GetNamedItem("cipherType").Value, false);
				if (t != null) {
					object o = Activator.CreateInstance(t);
					if (o == null) return null;

					if (ri == null)
						ri = new ReturnInfo();

					if (o is IConfigurationSectionHandler)
						ri.CipherSettingsObj = (o as IConfigurationSectionHandler).Create(parent, configContext, section);

					if (o is IEncryption)
						ri.CipherProvider = (IEncryption)o;
				}
			}

			if (section != null && section.Attributes["hashType"] != null) {
				Type t = Type.GetType(section.Attributes.GetNamedItem("hashType").Value, false);
				if (t != null) {
					object o = Activator.CreateInstance(t);
					if (o == null) return null;

					if (ri == null)
						ri = new ReturnInfo();

					if (o is IConfigurationSectionHandler)
						ri.HashSettingsObj = (o as IConfigurationSectionHandler).Create(parent, configContext, section);

					if (o is IHash)
						ri.HashProvider = (IHash)o;
				}
			}
			return ri;
		}
		#endregion
	}

	internal class ReturnInfo {
		public object CipherSettingsObj;
		public IEncryption CipherProvider;
		public object HashSettingsObj;
		public IHash HashProvider;
	}
}
