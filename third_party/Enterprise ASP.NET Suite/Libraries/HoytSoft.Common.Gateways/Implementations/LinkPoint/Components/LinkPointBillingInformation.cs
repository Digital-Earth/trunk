#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointBillingInformation : AddressInformation {
		#region Constants
		private const string
			XML_TAG_BILLING		= "billing",
			XML_TAG_NAME		= "name",
			XML_TAG_COMPANY		= "company",
			XML_TAG_ADDRESS1	= "address1",
			XML_TAG_ADDRESS2	= "address2",
			XML_TAG_CITY		= "city",
			XML_TAG_STATE		= "state",
			XML_TAG_ZIP			= "zip",
			XML_TAG_COUNTRY		= "country",
			XML_TAG_PHONE		= "phone",
			XML_TAG_FAX			= "fax",
			XML_TAG_EMAIL		= "email",
			XML_TAG_ADDRNUM		= "addrnum"
		;
		#endregion

		#region Constructors
		public LinkPointBillingInformation(AddressInformation from)
			: base(from) {
		}
		public LinkPointBillingInformation(string Name, string Company, string Address1, string Address2, string City, USState State, string Zip, string Phone, string Fax, string Email)
			: base(Name, Company, Address1, Address2, City, State, Zip, Phone, Fax, Email) {
		}
		public LinkPointBillingInformation(string Name, string Company, string Address1, string Address2, string City, string State, string Zip, Country Country, string Phone, string Fax, string Email)
			: base(Name, Company, Address1, Address2, City, State, Zip, Country, Phone, Fax, Email) {
		}
		#endregion

		#region Helper Methods
		public override bool IsRequired(AddressField Field) {
			return false;
		}

		public override bool IsRequiredAVS(AddressField Field) {
			switch (Field) {
				case AddressField.Name:
				case AddressField.Zip:
				case AddressField.Address1: //because they use Address1 for the address number...
				case AddressField.AddressNumber:
					return true;
					break;
			}
			return false;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_BILLING);

			if (!string.IsNullOrEmpty(this.Name))
				xw.WriteElementString(XML_TAG_NAME, this.Name);
			if (!string.IsNullOrEmpty(this.Company))
				xw.WriteElementString(XML_TAG_COMPANY, this.Company);
			if (!string.IsNullOrEmpty(this.Address1))
				xw.WriteElementString(XML_TAG_ADDRESS1, this.Address1);
			if (!string.IsNullOrEmpty(this.Address2))
				xw.WriteElementString(XML_TAG_ADDRESS2, this.Address2);
			if (!string.IsNullOrEmpty(this.City))
				xw.WriteElementString(XML_TAG_CITY, this.City);
			if (!string.IsNullOrEmpty(this.State))
				xw.WriteElementString(XML_TAG_STATE, this.GetStateAbbreviation());
			if (!string.IsNullOrEmpty(this.Zip))
				xw.WriteElementString(XML_TAG_ZIP, this.Zip);
			if (this.Country != Country.Unknown)
				xw.WriteElementString(XML_TAG_COUNTRY, new LinkPointCountry(this.Country).GetAbbreviation());
			if (!string.IsNullOrEmpty(this.Phone))
				xw.WriteElementString(XML_TAG_PHONE, numbersOnly(this.Phone));
			if (!string.IsNullOrEmpty(this.Fax))
				xw.WriteElementString(XML_TAG_FAX, numbersOnly(this.Fax));
			if (!string.IsNullOrEmpty(this.Email))
				xw.WriteElementString(XML_TAG_EMAIL, this.Email);

			//For linkpoint, you just pass in address1 for the address number...
			//Don't ask me why! (c:
			if (!string.IsNullOrEmpty(this.Address1))
				xw.WriteElementString(XML_TAG_ADDRNUM, this.Address1);
			
			xw.WriteEndElement();
		}
		#endregion
	}
}
