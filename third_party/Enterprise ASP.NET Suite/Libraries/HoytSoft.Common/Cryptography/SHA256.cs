#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Configuration;
using System.Security.Cryptography;
using System.Text;

namespace HoytSoft.Common.Cryptography {
	public class SHA256 {
		#region Constants
		public const string
			ALGORITHM_NAME = "HMACSHA256"
		;
		#endregion

		public static string Hash(string Text, byte[] Key) {
			if (Key == null)
				return Text;

			using (KeyedHashAlgorithm hash = new HMACSHA256(Key)/*KeyedHashAlgorithm.Create(ALGORITHM_NAME)*/) {
				return Convert.ToBase64String(hash.ComputeHash(Encoding.UTF8.GetBytes(Text)));
			}
		}
	}
}
