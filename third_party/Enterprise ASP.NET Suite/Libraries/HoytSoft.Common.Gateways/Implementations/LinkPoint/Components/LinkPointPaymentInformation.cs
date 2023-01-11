#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointPaymentInformation {
		#region Constants
		private const string
			XML_TAG_PAYMENT		= "payment", 
			XML_TAG_SUBTOTAL	= "subtotal",
			XML_TAG_TAX			= "tax",
			XML_TAG_VATTAX		= "vattax",
			XML_TAG_SHIPPING	= "shipping",
			XML_TAG_CHARGETOTAL = "chargetotal"
		;
		#endregion

		#region Variables
		private double? subTotal, tax, vatTax, shipping;
		private double total;
		#endregion

		#region Constructors
		public LinkPointPaymentInformation(double Total)
			: this(null, null, Total) {
		}
		public LinkPointPaymentInformation(double? SubTotal, double? Tax, double Total)
			: this(SubTotal, Tax, null, Total) {
		}
		public LinkPointPaymentInformation(double? SubTotal, double? Tax, double? Shipping, double Total)
			: this(SubTotal, Tax, null, Shipping, Total) {
		}
		public LinkPointPaymentInformation(double? SubTotal, double? Tax, double? VATTax, double? Shipping, double Total) {
			this.subTotal = SubTotal;
			this.tax = Tax;
			this.vatTax = VATTax;
			this.shipping = Shipping;
			this.total = Total;
		}
		#endregion

		#region Properties
		public double? SubTotal { get { return this.subTotal; } }
		public double? Tax { get { return this.tax; } }
		public double? VATTax { get { return this.vatTax; } }
		public double? Shipping { get { return this.shipping; } }
		public double Total { get { return this.total; } }
		#endregion

		#region Helper Methods
		public bool IsRequired(LinkPointPaymentField Field) {
			switch (Field) {
				case LinkPointPaymentField.Total:
					return true;
			}
			return false;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_PAYMENT);

			if (this.subTotal != null && this.subTotal.HasValue)
				xw.WriteElementString(XML_TAG_SUBTOTAL, this.subTotal.Value.ToString());
			if (this.tax != null && this.tax.HasValue) {
				if (this.tax.Value <= 0.0D)
					xw.WriteElementString(XML_TAG_TAX, "0");
				else
					xw.WriteElementString(XML_TAG_TAX, this.tax.Value.ToString());
			}
			if (this.vatTax != null && this.vatTax.HasValue)
				xw.WriteElementString(XML_TAG_VATTAX, this.vatTax.Value.ToString());
			if (this.shipping != null && this.shipping.HasValue)
				xw.WriteElementString(XML_TAG_SHIPPING, this.shipping.Value.ToString());
			xw.WriteElementString(XML_TAG_CHARGETOTAL, this.total.ToString());
			xw.WriteEndElement();
		}
		#endregion
	}
}
