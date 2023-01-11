#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointCreditCardInformation : CreditCardInformation {
		#region Constants
		private const string
			XML_TAG_CC					= "creditcard",
			XML_TAG_CC_NUMBER			= "cardnumber",
			XML_TAG_CC_EXPMONTH			= "cardexpmonth",
			XML_TAG_CC_EXPYEAR			= "cardexpyear", 
			XML_TAG_CC_CVV2				= "cvmvalue",
			XML_TAG_CC_CVV2_INDICATOR	= "cvmindicator"
		;
		#endregion

		#region Variables
		private LinkPointCVV2Indicator indicator;
		#endregion

		#region Constructors
		public LinkPointCreditCardInformation(string CardHolderName, string CCNumber, int ExpirationMonth, int ExpirationYear) : this(CardHolderName, CCNumber, ExpirationMonth, ExpirationYear, string.Empty, LinkPointCVV2Indicator.NotProvided) {
		}

		public LinkPointCreditCardInformation(string CardHolderName, string CCNumber, int ExpirationMonth, int ExpirationYear, string CVV2) : this(CardHolderName, CCNumber, ExpirationMonth, ExpirationYear, CVV2, LinkPointCVV2Indicator.Provided) {
		}

		public LinkPointCreditCardInformation(string CardHolderName, string CCNumber, int ExpirationMonth, int ExpirationYear, string CVV2, LinkPointCVV2Indicator Indicator) : base(CardHolderName, CCNumber, ExpirationMonth, ExpirationYear, CVV2) {
			this.indicator = Indicator;
		}
		#endregion

		#region Properties
		public LinkPointCVV2Indicator CVV2Indicator { get { return this.indicator; } }
		#endregion

		#region Helper Methods
		public override bool IsRequired(CreditCardField Field) {
			return true;
		}

		public bool IsRequired(LinkPointCreditCardField Field) {
			if (Field == LinkPointCreditCardField.CVV2Indicator) {
				if (string.IsNullOrEmpty(this.CVV2))
					return false;
				else
					return true;
			}
			return true;
		}

		public string GetIndicatorString() {
			switch (this.indicator) {
				case LinkPointCVV2Indicator.Provided:
					return "provided";
				case LinkPointCVV2Indicator.NotProvided:
					return "not_provided";
				case LinkPointCVV2Indicator.Illegible:
					return "illegible";
				case LinkPointCVV2Indicator.NotPresent:
					return "not_present";
				case LinkPointCVV2Indicator.NoImprint:
					return "no_imprint";
			}
			return string.Empty;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_CC);
			xw.WriteElementString(XML_TAG_CC_NUMBER, numbersOnly(this.CCNumber));
			xw.WriteElementString(XML_TAG_CC_EXPMONTH, this.ExpirationMonth.ToString("00"));
			//Must be in "07" (2008) or "99" (1999) format...
			if (this.ExpirationYear < 100 && this.ExpirationYear >= 0) {
				xw.WriteElementString(XML_TAG_CC_EXPYEAR, this.ExpirationYear.ToString("00"));
			} else {
				string year = this.ExpirationYear.ToString();
				year = year.Substring(year.Length - 2);
				xw.WriteElementString(XML_TAG_CC_EXPYEAR, year);
			}
			if (!string.IsNullOrEmpty(this.CVV2)) {
				xw.WriteElementString(XML_TAG_CC_CVV2, numbersOnly(this.CVV2));
				xw.WriteElementString(XML_TAG_CC_CVV2_INDICATOR, this.GetIndicatorString());
			}
			xw.WriteEndElement();
		}
		#endregion
	}
}
