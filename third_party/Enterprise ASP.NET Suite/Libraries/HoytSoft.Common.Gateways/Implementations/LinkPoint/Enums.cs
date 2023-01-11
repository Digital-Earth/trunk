#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways.LinkPoint {
	
	public enum SupportedVersion : byte {
		Default = 0, 
		V3_5	= 1, 
		V4_0	= 2,
	}

	public enum LinkPointCVV2Response : byte {
		Unknown,

		Match,
		NoMatch
	}

	public enum LinkPointAVSResponse : byte {
		Unknown, 

		Match, 
		NoMatch,
		Unavailable
	}

	public enum LinkPointError : ushort {
		Unknown = 0, 
		None = 1, 

		CallAuthorizationCenter = 2, 
		ApprovedPreviously		= 3, 
		LockedOut				= 4, 
		CreditCardExpired		= 5, 
		ServerError				= 6, 
		InvalidCreditCardNumber = 7,
		//Same as above on purpose!
		InvalidRoutingNumber	= 7, 
		InvalidMerchant			= 8, 
		InvalidTransaction		= 9, 
		TryAgain				= 10, 
		Declined				= 11, 
		ConnectionError			= 12
	}

	public enum LinkPointOrderOptionsResultType : byte {
		Live,
		Good,
		Duplicate,
		Decline
	}

	public enum LinkPointTransactionType : byte {
		Sale, 
		PreAuthorization, 
		Void, 
		Credit, 
		PostAuthorization, 
		CalculateShipping, 
		CalculateTax
	}

	public enum LinkPointTransactionOrigin : byte {
		Internet, 
		MailOrder, 
		MailOrTelephoneOrder, 
		TelephoneOrder,
		Retail
	}

	public enum LinkPointPaymentField : byte {
		SubTotal, 
		Tax, 
		VATTax, 
		Shipping, 
		Total
	}

	public enum LinkPointCheckingAccountType : byte {
		Unknown, 

		PersonalChecking, 
		PersonalSavings, 
		BusinessChecking, 
		BusinessSavings
	}

	public enum LinkPointCheckField : byte {
		AccountHolderName,
		AccountNumber,
		RoutingNumber, 
		CheckNumber, 
		BankName, 
		BankState, 
		DriversLicenseNumber, 
		DriversLicenseState, 
		AccountType, 
		SSN
	}

	public enum LinkPointCreditCardField : byte {
		CCNumber,
		ExpirationMonth,
		ExpirationYear,
		CVV2,
		CVV2Indicator
	}

	public enum LinkPointCVV2Indicator : byte {
		Provided,
		NotProvided,
		Illegible,
		NotPresent,
		NoImprint
	}

	public enum LinkPointShippingMethod : byte {
		///<summary>1 - Charges are based on the total number of items</summary>
		TotalNumberOfItems		= 1,
		///<summary>2 - Calculates charges based on each item, then totaled</summary>
		EachItemThenSum			= 2,
		///<summary>3 - Charges are based on the total weight of the order</summary>
		TotalWeight				= 3,
		///<summary>4 - Charges are based on the weight of each item, then totaled</summary>
		EachItemWeightThenSum	= 4,
		///<summary>5 - Charges are based on the total price of the order</summary>
		TotalPrice				= 5
	}

	public enum LinkPointShippingField : byte {
		Name,
		Company,
		Address1,
		Address2,
		City,
		State,
		Zip,
		Country,
		Phone,
		Fax,
		Email,
		AddressNumber, 
		TotalWeight, 
		SubTotal, 
		Tax, 
		TotalItemCount, 
		Carrier
	}

	public enum LinkPointItemField : byte {
		ID, 
		Description, 
		Price, 
		Quantity, 
		Serial, 
		ESDType, 
		SoftFile, 
		Options
	}

	public enum LinkPointESDType : byte {
		None			= 0, 
		File			= 1, 
		KeyGenerator	= 2,
		Default			= None 
	}
	
}
