#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointStandardErrorMessages {
		#region Constants
		private const string 
			MSG_SUCCESS				= "The transaction was approved and a debit has been made to your account. Thank you!", 
			MSG_APPROVED_PREVIOUSLY = "This transaction has already been made. Please make sure you are not resubmitting the same transaction.", 
			MSG_CALL_AUTH_CENTER	= "We are not able to process the transaction at this time. Please call our customer support line for more information.", 
			MSG_CC_EXPIRED			= "This credit card number has already expired. Please select a new card and try again.", 
			MSG_DECLINED			= "The transaction was declined by our system because there may be incongruent information. Please ensure that the billing information corresponds to the card or check used.", 
			MSG_INVALID_NUMBER		= "The credit card, routing, or account numbers used were invalid. Please check these fields and try again.", 
			MSG_INVALID_MERCHANT	= "An error has occurred in our system. Please contact customer support. Error: INVALID MERCHANT",
			MSG_INVALID_TRANSACTION = "An error has occurred in our system. Please contact customer support. Error: INVALID TRANSACTION", 
			MSG_LOCKED_OUT			= "Our records indicate that you have recently run a transaction through our system using this information. If you accidentally submitted this more than once, then nothing needs to be done. If you would like to resubmit, please wait a few minutes and try again.", 
			MSG_SERVER_ERROR		= "An error has occurred in our system. Please contact customer support. Error: SERVER ERROR",
			MSG_UNKNOWN				= "An error has occurred in our system. Please contact customer support. Error: UNKNOWN", 
			MSG_TRY_AGAIN			= "A possible error occurred in our system. Please resubmit your information.", 
			MSG_CONNECTION_ERROR	= "There was an error when attempting to connect to our payment gateway. Please try again or contact our customer support for additional help."
		;
		#endregion

		#region Variables
		private static Dictionary<LinkPointError, string> messages;
		#endregion

		#region Constructors
		static LinkPointStandardErrorMessages() {
			if (messages == null)
				messages = new Dictionary<LinkPointError, string>();
			messages.Clear();

			//Add all the messages...
			messages.Add(LinkPointError.None, MSG_SUCCESS);
			messages.Add(LinkPointError.ApprovedPreviously,			MSG_APPROVED_PREVIOUSLY);
			messages.Add(LinkPointError.CallAuthorizationCenter,	MSG_CALL_AUTH_CENTER);
			messages.Add(LinkPointError.CreditCardExpired,			MSG_CC_EXPIRED);
			messages.Add(LinkPointError.Declined,					MSG_DECLINED);
			//messages.Add(LinkPointError.InvalidRoutingNumber,		MSG_);
			messages.Add(LinkPointError.InvalidCreditCardNumber,	MSG_INVALID_NUMBER);
			messages.Add(LinkPointError.InvalidMerchant,			MSG_INVALID_MERCHANT);
			messages.Add(LinkPointError.InvalidTransaction,			MSG_INVALID_TRANSACTION);
			messages.Add(LinkPointError.LockedOut,					MSG_LOCKED_OUT);
			messages.Add(LinkPointError.ServerError,				MSG_SERVER_ERROR);
			messages.Add(LinkPointError.TryAgain,					MSG_TRY_AGAIN);
			messages.Add(LinkPointError.Unknown,					MSG_UNKNOWN);
			messages.Add(LinkPointError.ConnectionError,			MSG_CONNECTION_ERROR);
		}
		#endregion

		#region Public Methods
		public static string FromResponse(LinkPointResponse Response) {
			if (Response == null)
				return string.Empty;

			return FromError(Response.Error);
		}

		public static string FromError(LinkPointError Error) {
			if (messages == null || !messages.ContainsKey(Error))
				return string.Empty;
			return messages[Error];
		}
		#endregion
	}
}
