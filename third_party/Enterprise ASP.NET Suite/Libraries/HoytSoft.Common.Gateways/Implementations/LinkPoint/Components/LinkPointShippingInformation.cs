#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointShippingInformation : AddressInformation {
		#region Constants
		private const string
			XML_TAG_SHIPPING	= "shipping",
			XML_TAG_NAME		= "name",
			XML_TAG_ADDRESS1	= "address1",
			XML_TAG_ADDRESS2	= "address2",
			XML_TAG_CITY		= "city",
			XML_TAG_STATE		= "state",
			XML_TAG_ZIP			= "zip",
			XML_TAG_COUNTRY		= "country",
			XML_TAG_WEIGHT		= "weight",
			XML_TAG_ITEMS		= "items",
			XML_TAG_CARRIER		= "carrier",
			XML_TAG_TOTAL		= "total"
		;
		#endregion

		#region Variables
		private double? totalWeight, subTotal, tax;
		private int? totalItemCount, carrier;
		private double total = 0.0D;
		private LinkPointShippingMethod shippingMethod;
		#endregion

		#region Constructors
		public LinkPointShippingInformation(LinkPointShippingMethod ShippingMethod, double? TotalWeight, int? TotalItemCount, int? Carrier, double? SubTotal, double? Tax, AddressInformation from)
			: this(ShippingMethod, TotalWeight, TotalItemCount, Carrier, SubTotal, Tax, from.Name, from.Company, from.Address1, from.Address2, from.City, from.State, from.Zip, from.Country, from.Phone, from.Fax, from.Email) {
		}
		public LinkPointShippingInformation(LinkPointShippingMethod ShippingMethod, double? TotalWeight, int? TotalItemCount, int? Carrier, double? SubTotal, double? Tax, string Name, string Company, string Address1, string Address2, string City, string State, string Zip, Country Country, string Phone, string Fax, string Email)
			: base(Name, Company, Address1, Address2, City, State, Zip, Country, Phone, Fax, Email) {
			this.totalWeight = TotalWeight;
			this.subTotal = SubTotal;
			this.tax = Tax;
			this.totalItemCount = TotalItemCount;
			this.carrier = Carrier;
			this.shippingMethod = ShippingMethod;

			if (tax != null && tax.HasValue)
				this.total += tax.Value;
			if (subTotal != null && subTotal.HasValue)
				this.total += subTotal.Value;
		}
		#endregion

		#region Properties
		//In pounds (lbs)
		public double? TotalWeight { get { return this.totalWeight; } }
		public double? SubTotal { get { return this.subTotal; } }
		public double? Tax { get { return this.tax; } }
		public int? TotalItemCount { get { return this.totalItemCount; } }
		public int? Carrier { get { return this.carrier; } }
		public LinkPointShippingMethod ShippingMethod { get { return this.shippingMethod; } }
		#endregion

		#region Helper Methods
		public override bool IsRequired(AddressField Field) {
			if (Field == AddressField.State)
				return true;
			return false;
		}

		public bool IsRequired(LinkPointShippingField Field) {
			if (Field == LinkPointShippingField.State)
				return true;
			switch (this.shippingMethod) {
				case LinkPointShippingMethod.TotalNumberOfItems:
					switch (Field) {
						case LinkPointShippingField.TotalItemCount:
							return true;
					}
					break;
				case LinkPointShippingMethod.EachItemThenSum:
					switch (Field) {
						case LinkPointShippingField.TotalItemCount:
							return true;
					}
					break;
				case LinkPointShippingMethod.TotalWeight:
					switch (Field) {
						case LinkPointShippingField.TotalWeight:
							return true;
					}
					break;
				case LinkPointShippingMethod.EachItemWeightThenSum:
					switch (Field) {
						case LinkPointShippingField.TotalItemCount:
						case LinkPointShippingField.TotalWeight:
							return true;
					}
					break;
				case LinkPointShippingMethod.TotalPrice:
					switch (Field) {
						case LinkPointShippingField.SubTotal:
						case LinkPointShippingField.Tax:
							return true;
					}
					break;
			}
			return false;
		}

		public override bool IsRequiredAVS(AddressField Field) {
			if (Field == AddressField.State)
				return true;
			return false;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_SHIPPING);
			if (!string.IsNullOrEmpty(this.Name))
				xw.WriteElementString(XML_TAG_NAME, this.Name);
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
			if (this.carrier != null && this.carrier.HasValue)
				xw.WriteElementString(XML_TAG_CARRIER, this.carrier.Value.ToString());

			switch (this.shippingMethod) {
				case LinkPointShippingMethod.TotalNumberOfItems:
					if (this.totalItemCount != null && this.totalItemCount.HasValue)
						xw.WriteElementString(XML_TAG_ITEMS, this.totalItemCount.Value.ToString());
					break;
				case LinkPointShippingMethod.EachItemThenSum:
					if (this.totalItemCount != null && this.totalItemCount.HasValue)
						xw.WriteElementString(XML_TAG_ITEMS, this.totalItemCount.Value.ToString());
					break;
				case LinkPointShippingMethod.TotalWeight:
					if (this.totalWeight != null && this.totalWeight.HasValue)
						xw.WriteElementString(XML_TAG_WEIGHT, this.totalWeight.Value.ToString());
					break;
				case LinkPointShippingMethod.EachItemWeightThenSum:
					if (this.totalWeight != null && this.totalWeight.HasValue)
						xw.WriteElementString(XML_TAG_WEIGHT, this.totalWeight.Value.ToString());
					if (this.totalItemCount != null && this.totalItemCount.HasValue)
						xw.WriteElementString(XML_TAG_ITEMS, this.totalItemCount.Value.ToString());
					break;
				case LinkPointShippingMethod.TotalPrice:
					xw.WriteElementString(XML_TAG_TOTAL, this.total.ToString());
					break;
			}
			xw.WriteEndElement();
		}
		#endregion
	}
}
