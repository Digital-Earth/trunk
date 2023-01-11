#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Text;
using System.Text.RegularExpressions;

namespace HoytSoft.Common.Gateways {
	public abstract class CheckInformation {
		#region Constants
		public const string
			REGEXP_VALIDATE_ROUTING_NUMBER = @"\d{9}"
		;
		#endregion

		#region Variables
		private string name, account, routing;
		private static Regex regExpABA = null;
		#endregion

		#region Constructors
		static CheckInformation() {
			RegexOptions RegExpOptions = RegexOptions.Compiled | RegexOptions.ECMAScript | RegexOptions.IgnoreCase;
			regExpABA = new Regex(REGEXP_VALIDATE_ROUTING_NUMBER, RegExpOptions);
		}

		protected CheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber) {
			this.name = AccountHolderName;
			this.account = AccountNumber;
			this.routing = RoutingNumber;
		}
		#endregion

		#region Properties
		public string AccountHolderName { get { return name; } }
		public string AccountNumber { get { return account; } }
		public string RoutingNumber { get { return routing; } }
		#endregion

		#region Helper Methods
		public bool Validate() {
			return (RegularExpressionValidateRoutingNumber() && ABAValidate());
		}

		public bool RegularExpressionValidateRoutingNumber() {
			return StaticRegularExpressionValidateRoutingNumber(this.routing);
		}

		public bool RegularExpressionValidateRoutingNumber(string RoutingNumber) {
			return StaticRegularExpressionValidateRoutingNumber(RoutingNumber);
		}

		///<summary>Performs checksum validation on American Banker's Association routing numbers.</summary>
		public bool ABAValidate() {
			return StaticABAValidate(this.routing);
		}

		///<summary>Performs checksum validation on American Banker's Association routing numbers.</summary>
		public bool ABAValidate(string RoutingNumber) {
			return StaticABAValidate(RoutingNumber);
		}

		public static bool StaticRegularExpressionValidateRoutingNumber(string RoutingNumber) {
			if (regExpABA == null)
				return false;
			return regExpABA.IsMatch(RoutingNumber);
		}

		///<summary>Performs checksum validation on American Banker's Association routing numbers.</summary>
		public static bool StaticABAValidate(string RoutingNumber) {
			if (string.IsNullOrEmpty(RoutingNumber))
				return false;

			int len = RoutingNumber.Length;
			if (len != 9)
				return false;
			//Convert to a number...
			byte[] digits = System.Text.Encoding.ASCII.GetBytes(RoutingNumber);
			for (int i = 0; i < len; i++)
				digits[i] -= 48;
			int sum =
				(digits[0] * 3) + (digits[1] * 7) + digits[2] +
				(digits[3] * 3) + (digits[4] * 7) + digits[5] +
				(digits[6] * 3) + (digits[7] * 7) + digits[8]
			;
			return (sum != 0 && (sum % 10) == 0);
		}

		///<summary>Strips the text of all non-numeric characters</summary>
		protected static string numbersOnly(string Text) {
			if (string.IsNullOrEmpty(Text)) return Text;
			StringBuilder sb = new StringBuilder(Text.Length);
			for (int i = 0; i < Text.Length; i++)
				if (char.IsDigit(Text[i]))
					sb.Append(Text[i]);
			return sb.ToString();
		}
		#endregion

		public abstract bool IsRequired(CreditCardField Field);
	}
}
