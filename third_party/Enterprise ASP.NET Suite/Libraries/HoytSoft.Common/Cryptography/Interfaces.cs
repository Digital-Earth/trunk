#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Cryptography {
	///<summary>Standard interface for encryption/decryption for any flavor of algorithm.</summary>
	public interface IEncryption {
		///<summary>Method that any provider should implement.</summary>
		/// <returns>An encrypted string.</returns>
		string Encrypt(string PlainText, object Settings);

		///<summary>Method that any provider should implement.</summary>
		/// <returns>A decrypted string.</returns>
		string Decrypt(string EncryptedText, object Settings);
	}

	///<summary>Standard interface for secure hashing for any flavor of algorithm.</summary>
	public interface IHash {
		///<summary>Method that any hash provider should implement.</summary>
		/// <returns>Returns a hashed version of the text.</returns>
		string Hash(string Text, object Settings);
	}
}
