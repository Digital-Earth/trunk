#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointCheckInformation : CheckInformation {
		#region Constants
		private const string 
			XML_TAG_TELECHECK		= "telecheck",
			XML_TAG_ROUTING_NUMBER	= "routing",
			XML_TAG_ACCOUNT_NUMBER	= "account",
			XML_TAG_CHECK_NUMBER	= "checknumber",
			XML_TAG_BANK_NAME		= "bankname",
			XML_TAG_BANK_STATE		= "bankstate",
			XML_TAG_DL_NUMBER		= "dl",
			XML_TAG_DL_STATE		= "dlstate",
			XML_TAG_VOID			= "void",
			XML_TAG_ACCOUNT_TYPE	= "accounttype",
			XML_TAG_SSN				= "ssn"
		;
		#endregion

		#region Variables
		private string bankName, bankState, driversLicenseNumber, driversLicenseState, checkNumber, ssn;
		private LinkPointCheckingAccountType accountType;
		private bool voidingCheck;
		private USState myBankState, myDLState;
		#endregion

		#region Constructors
		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, USState BankState, string DriversLicenseNumber, USState DriversLicenseState, LinkPointCheckingAccountType AccountType)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, USStateInformation.StaticGetAbbreviation(BankState), DriversLicenseNumber, USStateInformation.StaticGetAbbreviation(DriversLicenseState), AccountType, string.Empty, string.Empty) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, USState BankState, string DriversLicenseNumber, USState DriversLicenseState, LinkPointCheckingAccountType AccountType, string SSN)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, USStateInformation.StaticGetAbbreviation(BankState), DriversLicenseNumber, USStateInformation.StaticGetAbbreviation(DriversLicenseState), AccountType, string.Empty, SSN) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, USState BankState, string DriversLicenseNumber, USState DriversLicenseState, LinkPointCheckingAccountType CheckingAccountType, string CheckNumber, string SSN)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, USStateInformation.StaticGetAbbreviation(BankState), DriversLicenseNumber, USStateInformation.StaticGetAbbreviation(DriversLicenseState), CheckingAccountType, CheckNumber, SSN, false) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, string BankState, string DriversLicenseNumber, string DriversLicenseState, LinkPointCheckingAccountType AccountType)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, BankState, DriversLicenseNumber, DriversLicenseState, AccountType, string.Empty, string.Empty) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, string BankState, string DriversLicenseNumber, string DriversLicenseState, LinkPointCheckingAccountType AccountType, string SSN)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, BankState, DriversLicenseNumber, DriversLicenseState, AccountType, string.Empty, SSN) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, string BankState, string DriversLicenseNumber, string DriversLicenseState, LinkPointCheckingAccountType CheckingAccountType, string CheckNumber, string SSN)
			: this(AccountHolderName, AccountNumber, RoutingNumber, BankName, BankState, DriversLicenseNumber, DriversLicenseState, CheckingAccountType, CheckNumber, SSN, false) {
		}

		public LinkPointCheckInformation(string AccountHolderName, string AccountNumber, string RoutingNumber, string BankName, string BankState, string DriversLicenseNumber, string DriversLicenseState, LinkPointCheckingAccountType CheckingAccountType, string CheckNumber, string SSN, bool VoidingCheck)
			: base(AccountHolderName, AccountNumber, RoutingNumber) {
			this.bankName				= BankName;
			this.bankState				= BankState;
			this.driversLicenseNumber	= DriversLicenseNumber;
			this.driversLicenseState	= DriversLicenseState;
			this.checkNumber			= CheckNumber;
			this.ssn					= SSN;
			this.accountType			= CheckingAccountType;
			this.voidingCheck			= VoidingCheck;
			this.myDLState				= USState.Unknown;
			this.myBankState			= USState.Unknown;

			if (!string.IsNullOrEmpty(BankState) && BankState.Trim().Length == 2)
				this.myBankState = USStateInformation.StaticGetUSStateFromAbbreviation(BankState.Trim());
			if (!string.IsNullOrEmpty(DriversLicenseState) && DriversLicenseState.Trim().Length == 2)
				this.myDLState = USStateInformation.StaticGetUSStateFromAbbreviation(DriversLicenseState.Trim());
		}
		#endregion

		#region Properties
		public bool VoidingCheck { get { return this.voidingCheck; } }
		public string BankName { get { return this.bankName; } }
		public string BankState { get { return this.bankState; } }
		public string DriversLicenseNumber { get { return this.driversLicenseNumber; } }
		public string DriversLicenseState { get { return this.driversLicenseState; } }
		public string CheckNumber { get { return this.checkNumber; } }
		public string SSN { get { return this.ssn; } }
		public LinkPointCheckingAccountType CheckingAccountType { get { return this.accountType; } }

		public USState USBankState { get { return myBankState; } }
		public string BankStateFullName { get { return this.GetBankStateFullName(); } }
		public string BankStateAbbreviation { get { return this.GetBankStateAbbreviation(); } }

		public USState USDriversLicenseState { get { return myDLState; } }
		public string DriversLicenseStateFullName { get { return this.GetDriversLicenseStateFullName(); } }
		public string DriversLicenseStateAbbreviation { get { return this.GetDriversLicenseStateAbbreviation(); } }
		#endregion

		#region Helper Methods
		public LinkPointCheckingAccountType FromAccountTypeList(string Value) {
			return StaticFromAccountTypeList(Value);
		}

		public static LinkPointCheckingAccountType StaticFromAccountTypeList(string Value) {
			if (string.IsNullOrEmpty(Value))
				return LinkPointCheckingAccountType.Unknown;

			byte enumVal = 0;
			if (byte.TryParse(Value, out enumVal)) {
				try {
					return (LinkPointCheckingAccountType)Enum.ToObject(typeof(LinkPointCheckingAccountType), enumVal);
				} catch {
					return LinkPointCheckingAccountType.Unknown;
				}
			} else {
				return LinkPointCheckingAccountType.Unknown;
			}
		}

		public virtual System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> GetAccountTypeList() {
			return StaticGetAccountTypeList();
		}

		public static System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> StaticGetAccountTypeList() {
			System.Collections.Generic.List<System.Web.UI.WebControls.ListItem> lst = new System.Collections.Generic.List<System.Web.UI.WebControls.ListItem>(4);
			lst.Add(new System.Web.UI.WebControls.ListItem("Personal Checking", ((byte)LinkPointCheckingAccountType.PersonalChecking).ToString()));
			lst.Add(new System.Web.UI.WebControls.ListItem("Personal Savings", ((byte)LinkPointCheckingAccountType.PersonalSavings).ToString()));
			lst.Add(new System.Web.UI.WebControls.ListItem("Business Checking", ((byte)LinkPointCheckingAccountType.BusinessChecking).ToString()));
			lst.Add(new System.Web.UI.WebControls.ListItem("Business Savings", ((byte)LinkPointCheckingAccountType.BusinessSavings).ToString()));
			return lst;
		}

		public string GetBankStateFullName() {
			if (this.myBankState != USState.Unknown)
				return USStateInformation.StaticGetFullName(this.myBankState);
			return this.bankState;
		}

		public string GetBankStateAbbreviation() {
			if (this.myBankState != USState.Unknown)
				return USStateInformation.StaticGetAbbreviation(this.myBankState);
			return this.bankState;
		}

		public string GetDriversLicenseStateFullName() {
			if (this.myDLState != USState.Unknown)
				return USStateInformation.StaticGetFullName(this.myDLState);
			return this.driversLicenseState;
		}

		public string GetDriversLicenseStateAbbreviation() {
			if (this.myDLState != USState.Unknown)
				return USStateInformation.StaticGetAbbreviation(this.myDLState);
			return this.driversLicenseState;
		}

		public override bool IsRequired(CreditCardField Field) {
			return true;
		}

		public bool IsRequired(LinkPointCheckField Field) {
			switch (Field) {
				//These are the only 2 fields that are NOT required...
				case LinkPointCheckField.CheckNumber:
				case LinkPointCheckField.SSN:
					return false;
				default:
					return true;
			}
		}

		public string GetCheckingAccountTypeString() {
			switch (this.accountType) {
				case LinkPointCheckingAccountType.PersonalChecking:
					return "pc";
				case LinkPointCheckingAccountType.PersonalSavings:
					return "ps";
				case LinkPointCheckingAccountType.BusinessChecking:
					return "bc";
				case LinkPointCheckingAccountType.BusinessSavings:
					return "bs";
			}
			return string.Empty;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_TELECHECK);

			xw.WriteElementString(XML_TAG_ROUTING_NUMBER, numbersOnly(this.RoutingNumber));
			xw.WriteElementString(XML_TAG_ACCOUNT_NUMBER, numbersOnly(this.AccountNumber));
			if (!string.IsNullOrEmpty(this.checkNumber))
				xw.WriteElementString(XML_TAG_CHECK_NUMBER, numbersOnly(this.checkNumber));
			xw.WriteElementString(XML_TAG_BANK_NAME, this.bankName);
			xw.WriteElementString(XML_TAG_BANK_STATE, this.GetBankStateAbbreviation());
			xw.WriteElementString(XML_TAG_DL_NUMBER, this.driversLicenseNumber);
			xw.WriteElementString(XML_TAG_DL_STATE, this.GetDriversLicenseStateAbbreviation());
			if (this.voidingCheck)
				xw.WriteElementString(XML_TAG_VOID, "1");
			xw.WriteElementString(XML_TAG_ACCOUNT_TYPE, this.GetCheckingAccountTypeString());
			if (!string.IsNullOrEmpty(this.ssn))
				xw.WriteElementString(XML_TAG_SSN, this.ssn);

			xw.WriteEndElement();
		}
		#endregion
	}
}
