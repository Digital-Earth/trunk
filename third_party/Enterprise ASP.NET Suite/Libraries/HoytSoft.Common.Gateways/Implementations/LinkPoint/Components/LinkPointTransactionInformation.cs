#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointTransactionInformation {
		#region Constants
		private const string 
			XML_TAG_TRANS_DETAILS	= "transactiondetails", 
			XML_TAG_TRANS_ORIGIN	= "transactionorigin", 
			XML_TAG_OID				= "oid", 
			XML_TAG_REF_NUMBER		= "reference_number", 
			XML_TAG_PO_NUMBER		= "ponumber", 
			XML_TAG_RECURRING		= "recurring", 
			XML_TAG_TAXEXEMPT		= "taxexempt", 
			XML_TAG_TERMINAL_TYPE	= "terminaltype", 
			XML_TAG_IP				= "ip", 
			XML_TAG_TDATE			= "tdate"
		;
		#endregion

		#region Variables
		private LinkPointTransactionType transType;
		private LinkPointTransactionOrigin origin;
		private string orderID, purchaseOrderNumber, ip, tdate;
		private bool usesTax;
		#endregion

		#region Constructors
		public LinkPointTransactionInformation()
			: this(LinkPointTransactionOrigin.Internet) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionType TransactionType)
			: this(TransactionType, LinkPointTransactionOrigin.Internet, Guid.NewGuid().ToString("N"), string.Empty, false, string.Empty) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionType TransactionType, LinkPointTransactionOrigin Origin)
			: this(TransactionType, Origin, Guid.NewGuid().ToString("N"), string.Empty, false, string.Empty) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionOrigin Origin)
			: this(Origin, Guid.NewGuid().ToString("N")) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionOrigin Origin, string OrderID)
			: this(Origin, OrderID, string.Empty) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionOrigin Origin, string OrderID, string PurchaseOrderNumber)
			: this(Origin, OrderID, PurchaseOrderNumber, false) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionOrigin Origin, string OrderID, string PurchaseOrderNumber, bool UsesTax)
			: this(Origin, OrderID, PurchaseOrderNumber, UsesTax, string.Empty) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionOrigin Origin, string OrderID, string PurchaseOrderNumber, bool UsesTax, string IPAddress)
			: this(LinkPointTransactionType.Sale, Origin, OrderID, PurchaseOrderNumber, UsesTax, IPAddress) {
		}

		public LinkPointTransactionInformation(LinkPointTransactionType TransactionType, LinkPointTransactionOrigin Origin, string OrderID, string PurchaseOrderNumber, bool UsesTax, string IPAddress) {
			this.origin = Origin;
			this.orderID = OrderID;
			this.purchaseOrderNumber = PurchaseOrderNumber;
			this.usesTax = UsesTax;
			this.ip = IPAddress;
			this.transType = TransactionType;
		}
		#endregion

		#region Properties
		public LinkPointTransactionType TransactionType { get { return this.transType; } }
		public LinkPointTransactionOrigin Origin { get { return this.origin; } }
		public string OrderID { get { return this.orderID; } }
		public string PurchaseOrderNumber { get { return this.purchaseOrderNumber; } }
		public bool UsesTax { get { return this.usesTax; } }
		public bool IsTaxExempt { get { return !this.usesTax; } }
		public string IPAddress { get { return this.ip; } }
		public string TDate { get { return this.tdate; } }
		#endregion

		#region Helper Methods
		internal void setTDate(string value) {
			this.tdate = value;
		}

		public string GetTransactionTypeString() {
			return this.GetTransactionTypeString(this.transType);
		}

		public string GetTransactionTypeString(LinkPointTransactionType TransactionType) {
			switch (TransactionType) {
				case LinkPointTransactionType.Sale:
					return "SALE";
				case LinkPointTransactionType.PreAuthorization:
					return "PREAUTH";
				case LinkPointTransactionType.Void:
					return "VOID";
				case LinkPointTransactionType.Credit:
					return "CREDIT";
				case LinkPointTransactionType.PostAuthorization:
					return "POSTAUTH";
				case LinkPointTransactionType.CalculateShipping:
					return "CALCSHIPPING";
				case LinkPointTransactionType.CalculateTax:
					return "CALCTAX";
			}
			return string.Empty;
		}

		public string GetRecurringString() {
			return "No";
		}

		public string GetTerminalTypeString() {
			return "UNSPECIFIED";
		}

		public string GetTaxExemptString() {
			if (this.usesTax)
				return "N";
			else
				return "Y";
		}

		public string GetOriginString() {
			switch (this.origin) {
				case LinkPointTransactionOrigin.Internet:
					return "ECI";
				case LinkPointTransactionOrigin.MailOrder:
					return "MAIL";
				case LinkPointTransactionOrigin.MailOrTelephoneOrder:
					return "MOTO";
				case LinkPointTransactionOrigin.TelephoneOrder:
					return "TELEPHONE";
				case LinkPointTransactionOrigin.Retail:
					return "RETAIL";
			}
			return string.Empty;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_TRANS_DETAILS);

			xw.WriteElementString(XML_TAG_TRANS_ORIGIN, this.GetOriginString());
			
			if (this.transType != LinkPointTransactionType.PostAuthorization)
				xw.WriteElementString(XML_TAG_OID, this.orderID);
			else if (!string.IsNullOrEmpty(this.orderID))
				xw.WriteElementString(XML_TAG_OID, this.orderID);

			if (!string.IsNullOrEmpty(this.purchaseOrderNumber))
				xw.WriteElementString(XML_TAG_PO_NUMBER, this.purchaseOrderNumber);
			
			xw.WriteElementString(XML_TAG_TAXEXEMPT, this.GetTaxExemptString());
			xw.WriteElementString(XML_TAG_TERMINAL_TYPE, this.GetTerminalTypeString());
			
			if (!string.IsNullOrEmpty(this.ip))
				xw.WriteElementString(XML_TAG_IP, this.ip);

			//Reference number is ignored for now...
			
			xw.WriteElementString(XML_TAG_RECURRING, this.GetRecurringString());
			if (!string.IsNullOrEmpty(this.tdate))
				xw.WriteElementString(XML_TAG_TDATE, this.tdate);

			xw.WriteEndElement();
		}
		#endregion
	}
}
