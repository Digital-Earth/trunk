#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using System.Configuration;
using HoytSoft.Common.Cryptography;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Configuration {
	///<summary>Allows access to cryptographic provider as defined in a config file.</summary>
	public class Cryptography {
		#region Variables
		private const string
			INIT_VECTOR = "bE+7!us!$vejexes",
			PASS_PHRASE = "FrA_r4yurucuf??$E$Wep@AME#uv-PhArekU_UXEspMku@#pu+abUBU-achaJ_bE", 
			HMAC_KEY = @"^)!&B}w?W-nbMJD)SQM4R^Uw{RYj5f_a;8)bV\>z|(2#cJa@n*d2_n*]\]G6]g{";
		private static IEncryption cryptoProvider = null;
		private static object oSettings = null;
		private static IHash hashProvider = null;
		private static object oHashSettings = null;
		#endregion

		static Cryptography() {
			ReturnInfo ri = Settings.From<ReturnInfo>(Settings.Section.Cryptography);

			if (ri != null && ri.CipherProvider != null) {
				cryptoProvider = ri.CipherProvider;
				oSettings = ri.CipherSettingsObj;
			}

			if (ri != null && ri.HashProvider != null) {
				hashProvider = ri.HashProvider;
				oHashSettings = ri.HashSettingsObj;
			}

			if (ri == null || ri.CipherProvider == null)
				cryptoProvider = new Internal.AES256();
			if (ri == null || ri.CipherSettingsObj == null) {
				AESInfo ai = new AESInfo();
				ai.InitVector = INIT_VECTOR;
				ai.PassPhrase = PASS_PHRASE;
				oSettings = ai;
			}

			if (ri == null || ri.HashProvider == null)
				hashProvider = new Internal.SHA256();
			if (ri == null || ri.HashSettingsObj == null) {
				HMACInfo hi = new HMACInfo(HMAC_KEY);
				oHashSettings = hi;
			}
		}

		///<summary>Encrypts data using a provider specified in a config file or AES by default.</summary>
		/// <param name="PlainText">The text you want to encrypt.</param>
		/// <returns>The encrypted PlainText.</returns>
		public static string Encrypt(string PlainText) {
			if (cryptoProvider != null)
				return cryptoProvider.Encrypt(PlainText, oSettings);
			return PlainText;
		}

		///<summary>Decrypts data using a provider specified in a config file or AES by default.</summary>
		/// <param name="EncryptedText">The text you want to decrypt.</param>
		/// <returns>The decrypted EncryptedText.</returns>
		public static string Decrypt(string EncryptedText) {
			if (cryptoProvider != null)
				return cryptoProvider.Decrypt(EncryptedText, oSettings);
			return EncryptedText;
		}

		///<summary>Method that any hash provider should implement.</summary>
		/// <returns>Returns a hashed version of the text.</returns>
		public static string Hash(string Text) {
			if (hashProvider != null)
				return hashProvider.Hash(Text, oHashSettings);
			return Text;
		}
	}

	
}
