using System;
using System.Configuration;
using HoytSoft.Common.Cryptography;
using System.Security.Cryptography;

namespace HoytSoft.Common.Configuration.Internal {
	///<summary>Provides government standard AES (Rijndael) encryption.</summary>
	public class AES256 : IEncryption, IConfigurationSectionHandler {
		#region Private Variables
		private string passPhrase = null;
		private string initVector = null;
		private AESEnhanced aesKey = null;
		#endregion

		#region IEncryption
		///<summary>Encrypt text using the AES algorithm.</summary>
		public string Encrypt(string PlainText, object Settings) {
			this.checkSettings(ref Settings);
			if (this.passPhrase == null || this.initVector == null) return null;
			if (this.aesKey == null)
				this.aesKey = recreate();

			try {
				return this.aesKey.Encrypt(PlainText);
			} catch(CryptographicException ce) {
				//Bug in .net framework - if there's an exception, you have to reinitialize settings
				this.aesKey = recreate();
				throw ce;
			}
		}

		///<summary>Decrypt text using the AES algorithm.</summary>
		public string Decrypt(string EncryptedText, object Settings) {
			this.checkSettings(ref Settings);
			if (this.passPhrase == null || this.initVector == null) return null;
			if (this.aesKey == null)
				this.aesKey = recreate();

			try {
				return this.aesKey.Decrypt(EncryptedText);
			} catch (CryptographicException ce) {
				//Bug in .net framework - if there's an exception, you have to reinitialize settings
				this.aesKey = recreate();
				throw ce;
			}
		}
		#endregion

		#region Helper Functions
		protected AESEnhanced recreate() {
			return new AESEnhanced(this.passPhrase, this.initVector, -1, -1, 256, null, null, 3);
		}

		protected void checkSettings(ref object Settings) {
			//We can safely ignore settings here since the library will call Create when this
			//class is first instantiated and then keeps an instance of this object around...
			if (Settings is AESInfo) {
				AESInfo ai = Settings as AESInfo;
				if (ai.InitVector != this.initVector || ai.PassPhrase != this.passPhrase)
					this.aesKey = new AESEnhanced(ai.PassPhrase, ai.InitVector, -1, -1, 256, null, null, 3);

				this.initVector = ai.InitVector;
				this.passPhrase = ai.PassPhrase;
			}
		}
		#endregion

		///<summary>Used to load configuration information from an XML config file.</summary>
		/// <returns>An AESInfo object.</returns>
		public object Create(object parent, object configContext, System.Xml.XmlNode section) {
			System.Xml.XmlNode xnAES = null;

			if (section != null && (xnAES = section.SelectSingleNode("AES")) != null) {
				//Create a class to store settings info. and return it w/ any values we have
				//found for it...
				AESInfo ai = new AESInfo();
				if (xnAES.Attributes["initializationVector"] != null)
					ai.InitVector = xnAES.Attributes["initializationVector"].Value;
				if (xnAES.Attributes["passPhrase"] != null)
					ai.PassPhrase = xnAES.Attributes["passPhrase"].Value;

				//this.passPhrase = ai.PassPhrase;
				//this.initVector = ai.InitVector;
				return ai;
			}
			return null;
		}
	}

	#region Helper Classes
	///<summary>Necessary for holding information about the AES encryption algorithm.</summary>
	public class AESInfo {
		///<summary>
		/// Passphrase from which a pseudo-random password will be derived.
		/// The derived password will be used to generate the encryption key
		/// Passphrase can be any string. In this example we assume that the
		/// passphrase is an ASCII string. Passphrase value must be kept in
		/// secret.
		///</summary>
		public string PassPhrase = "";

		///<summary>
		/// Initialization vector (IV). This value is required to encrypt the
		/// first block of plaintext data. For RijndaelManaged class IV must be
		/// exactly 16 ASCII characters long. IV value does not have to be kept
		/// in secret. 
		///</summary>
		public string InitVector = "";
	}
	#endregion
}
