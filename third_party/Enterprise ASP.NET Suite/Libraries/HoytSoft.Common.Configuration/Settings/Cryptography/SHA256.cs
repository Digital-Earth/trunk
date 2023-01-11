using System;
using System.Configuration;
using System.Text;
using HoytSoft.Common.Cryptography;

namespace HoytSoft.Common.Configuration.Internal {
	public class SHA256  : IHash, IConfigurationSectionHandler {
		#region Variables/Constants
		public const string
			SETTINGS_NAME = "SHA-HMAC"
		;

		private string
			key = null;
		private byte[]
			byteArrKey = null;
		#endregion

		#region Helper Methods
		protected void checkSettings(ref object Settings) {
			if (Settings != null && Settings is HMACInfo) {
				HMACInfo s = Settings as HMACInfo;
				if ((this.key == null && !string.IsNullOrEmpty(s.Key)) || !this.key.Equals(s.Key, StringComparison.InvariantCultureIgnoreCase)) {
					this.key = s.Key;
					if (!string.IsNullOrEmpty(this.key))
						this.byteArrKey = Encoding.UTF8.GetBytes(this.key);
				}

			}
		}
		#endregion

		public string Hash(string Text, object Settings) {
			this.checkSettings(ref Settings);

			if (this.byteArrKey == null)
				return Text;

			return Common.Cryptography.SHA256.Hash(Text, this.byteArrKey);
		}

		public object Create(object Parent, object Context, System.Xml.XmlNode Section) {
			System.Xml.XmlNode xnSHA = null;

			if (Section != null && (xnSHA = Section.SelectSingleNode(SETTINGS_NAME)) != null) {
				//Create a class to store settings info. and return it w/ any values we have
				//found for it...
				HMACInfo info = new HMACInfo();
				if (xnSHA.Attributes["key"] != null)
					info.Key = xnSHA.Attributes["key"].Value;

				//this.key = info.Key;
				//if (!string.IsNullOrEmpty(this.key))
				//	this.byteArrKey = Encoding.UTF8.GetBytes(this.key);
				return info;
			}
			return null;
		}
	}

	#region Helper Classes
	public class HMACInfo {
		public string Key;

		public HMACInfo() { }
		public HMACInfo(string Key)
			: this() {
			this.Key = Key;
		}
	}
	#endregion
}
