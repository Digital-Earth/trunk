#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Text;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace HoytSoft.Common.Gateways {
	public abstract class CreditCardInformation {
		#region Constants
		public const string
			REGEXP_VALIDATE_CREDITCARD		= @"^3(?:[47]\d([ -]?)\d{4}(?:\1\d{4}){2}|0[0-5]\d{11}|[68]\d{12})$|^4(?:\d\d\d)?([ -]?)\d{4}(?:\2\d{4}){2}$|^6011([ -]?)\d{4}(?:\3\d{4}){2}$|^5[1-5]\d\d([ -]?)\d{4}(?:\4\d{4}){2}$|^2014\d{11}$|^2149\d{11}$|^2131\d{11}$|^1800\d{11}$|^3\d{15}$"
		;
		#endregion

		#region Variables
		private string name;
		private string ccnumber, ccnumberOriginal, cvv2;
		private int expmonth, expyear;
		private DateTime ends;
		private static List<string> testingCCNumbers;
		private static Regex regExpCC = null;
		#endregion

		#region Constructors
		static CreditCardInformation() {
			RegexOptions RegExpOptions = RegexOptions.Compiled | RegexOptions.ECMAScript | RegexOptions.IgnoreCase;
			regExpCC = new Regex(REGEXP_VALIDATE_CREDITCARD, RegExpOptions);

			testingCCNumbers = new List<string>(14);
			testingCCNumbers.Add("4111111111111111"); //Visa 
			testingCCNumbers.Add("4012888888881881"); //Visa 
			testingCCNumbers.Add("5215521552155215"); //Mastercard
			testingCCNumbers.Add("5105105105105100"); //Mastercard
			testingCCNumbers.Add("378282246310005");  //American Exp, Corporate
			testingCCNumbers.Add("371449635398431");  //American Exp
			testingCCNumbers.Add("6011111111111117"); //Discover
			testingCCNumbers.Add("6011000990139424"); //Discover
			testingCCNumbers.Add("3530111333300000"); //JCB
			testingCCNumbers.Add("3566002020360505"); //JCB
			testingCCNumbers.Add("5424180279791765"); //Mastercard
			testingCCNumbers.Add("4005550000000019"); //Visa
			testingCCNumbers.Add("372700997251009");  //American Exp, Corporate
			testingCCNumbers.Add("6011000993010978"); //Discover
		}

		protected CreditCardInformation(string CardHolderName, string CCNumber, int ExpirationMonth, int ExpirationYear, string CVV2) {
			this.name = CardHolderName;
			this.ccnumberOriginal = CCNumber;
			this.ccnumber = numbersOnly(CCNumber);
			this.expmonth = ExpirationMonth;
			this.expyear = ExpirationYear;
			this.cvv2 = CVV2;
			this.ends = new DateTime(this.expyear, this.expmonth, DateTime.DaysInMonth(this.expyear, this.expmonth), 23, 59, 59);
		}
		#endregion

		#region Properties
		public string CardHolderName { get { return name; } }
		public string CCNumber { get { return ccnumber; } }
		public string CCNumberOriginal { get { return ccnumberOriginal; } }
		public string CVV2 { get { return cvv2; } }
		public int ExpirationYear { get { return expyear; } }
		public int ExpirationMonth { get { return expmonth; } }
		#endregion

		#region Helper Methods
		public bool Validate() {
			return (!HasExpired() && !IsTestCCNumber() && RegularExpressionValidate() && GetCreditCardType() != CreditCardType.Unknown && LuhnValidate());
		}

		public bool HasExpired(DateTime Date) {
			return (Date > this.ends);
		}

		public bool HasExpired() {
			return (DateTime.Now > this.ends);
		}

		public static bool StaticHasExpired(int Month, int Year) {
			DateTime ends = new DateTime(Year, Month, DateTime.DaysInMonth(Year, Month), 23, 59, 59);
			return (DateTime.Now > ends);
		}

		public List<string> GetTestCCNumbers() {
			return StaticGetTestCCNumbers();
		}

		public static List<string> StaticGetTestCCNumbers() {
			return testingCCNumbers;
		}

		public bool IsTestCCNumber() {
			return StaticIsTestCCNumber(this.ccnumber);
		}

		public bool IsTestCCNumber(string CCNumber) {
			return StaticIsTestCCNumber(CCNumber);
		}

		public bool RegularExpressionValidate() {
			return StaticRegularExpressionValidate(this.ccnumber);
		}

		public bool RegularExpressionValidate(string CCNumber) {
			return StaticRegularExpressionValidate(CCNumber);
		}

		///<summary>Perform's Luhn checksum validation on credit card number.</summary>
		public bool LuhnValidate() {
			return StaticLuhnValidate(this.ccnumber);
		}

		///<summary>Perform's Luhn checksum validation on credit card number.</summary>
		public bool LuhnValidate(string CCNumber) {
			return StaticLuhnValidate(CCNumber);
		}

		public static bool StaticIsTestCCNumber(string CCNumber) {
			if (CCNumber == null) return false;
			if (testingCCNumbers != null && testingCCNumbers.Contains(CCNumber.Trim()))
				return true;
			return false;
		}

		public CreditCardType GetCreditCardType() {
			return StaticGetCreditCardType(this.ccnumber);
		}

		public CreditCardType GetCreditCardType(string CCNumber) {
			return StaticGetCreditCardType(CCNumber);
		}

		public static CreditCardType StaticGetCreditCardType(string CCNumber) {
			if (CCNumber == null)
				return CreditCardType.Unknown;
			CCNumber = CCNumber.Trim();
			if (string.IsNullOrEmpty(CCNumber))
				return CreditCardType.Unknown;

			switch (CCNumber.Length) {
				case 13:
					return CheckFor13(CCNumber);
				case 14:
					return CheckFor14(CCNumber);
				case 15:
					return CheckFor15(CCNumber);
				case 16:
					return CheckFor16(CCNumber);
				default:
					return CreditCardType.Unknown;
			}
		}

		protected static CreditCardType CheckFor13(string CCNumber) {
			if (Convert.ToByte(CCNumber[0] + "") == 4)
				return CreditCardType.Visa;
			return CreditCardType.Unknown;
		}

		protected static CreditCardType CheckFor14(string CCNumber) {
			if (Convert.ToByte(CCNumber[0] + "") != 3)
				return CreditCardType.Unknown;
			byte at1 = Convert.ToByte(CCNumber[1] + "");
			if (at1 == 6 || at1 == 8)
				return CreditCardType.DinersClub;
			if (at1 == 0) {
				byte at2 = Convert.ToByte(CCNumber[2] + "");
				if (at2 >= 0 && at2 <= 5)
					return CreditCardType.DinersClub;
			}
			return CreditCardType.Unknown;
		}

		protected static CreditCardType CheckFor15(string CCNumber) {
			byte at0 = Convert.ToByte(CCNumber[0] + ""), at1 = Convert.ToByte(CCNumber[1] + "");
			if (at0 == 3 && (at1 == 4 || at1 == 7))
				return CreditCardType.AmericanExpress;
			int firstFour = at0 * 1000;
			firstFour += at1 * 100;
			firstFour += Convert.ToByte(CCNumber[2] + "") * 10;
			firstFour += Convert.ToByte(CCNumber[3] + "");
			if (firstFour == 2014 || firstFour == 2149)
				return CreditCardType.EnRoute;
			if (firstFour == 2131 || firstFour == 1800)
				return CreditCardType.JCB;
			return CreditCardType.Unknown;
		}

		protected static CreditCardType CheckFor16(string CCNumber) {
			byte a = Convert.ToByte(CCNumber[0] + "");
			byte b = Convert.ToByte(CCNumber[1] + "");
			switch (a) {
				case 5:
					if (b > 0 && b < 6)
						return CreditCardType.Mastercard;
					else
						return CreditCardType.Unknown;
				case 4:
					return CreditCardType.Visa;
				case 6:
					if (b == 0 && Convert.ToByte(CCNumber[2] + "") == 1 && Convert.ToByte(CCNumber[3] + "") == 1)
						return CreditCardType.Discover;
					else
						return CreditCardType.Unknown;
				case 3:
					return CreditCardType.JCB;
				default:
					return CreditCardType.Unknown;

			}
		}

		public static bool StaticRegularExpressionValidate(string CCNumber) {
			if (regExpCC == null)
				return false;
			return regExpCC.IsMatch(CCNumber);
		}

		///<summary>Perform's Luhn checksum validation on credit card number.</summary>
		public static bool StaticLuhnValidate(string CCNumber) {
			if (string.IsNullOrEmpty(CCNumber))
				return false;

			int length = CCNumber.Length;

			if (length < 13)
				return false;

			int sum = 0;
			int offset = length % 2;
			byte[] digits = new System.Text.ASCIIEncoding().GetBytes(CCNumber);

			for (int i = 0; i < length; i++) {
				digits[i] -= 48;
				if (((i + offset) % 2) == 0)
					digits[i] *= 2;

				sum += (digits[i] > 9) ? digits[i] - 9 : digits[i];
			}
			return ((sum % 10) == 0);
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
