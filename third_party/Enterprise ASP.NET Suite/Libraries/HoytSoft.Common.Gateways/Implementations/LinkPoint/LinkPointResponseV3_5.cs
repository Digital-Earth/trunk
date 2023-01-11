#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using System.IO;
using System.Globalization;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public sealed class LinkPointResponseV3_5 : LinkPointResponse {
		#region Constants
		private const string
			XML_TAG_CSP					= "r_csp",
			XML_TAG_TIME				= "r_time",
			XML_TAG_REF					= "r_ref",
			XML_TAG_ERROR				= "r_error",
			XML_TAG_ORDER_NUMBER		= "r_ordernum",
			XML_TAG_MESSAGE				= "r_message",
			XML_TAG_CODE				= "r_code",
			XML_TAG_TDATE				= "r_tdate",
			XML_TAG_SCORE				= "r_score",
			XML_TAG_AUTH_RESPONSE		= "r_authresponse",
			XML_TAG_APPROVED			= "r_approved", 
			XML_TAG_AVS					= "r_avs", 
			XML_TAG_TAX					= "r_tax", 
			XML_TAG_SHIPPING			= "r_shipping" 
		;

		private const string 
			ERROR_START					= "SGS-", 
			ERROR_END					= ":"
		;

		private const string
			MESSAGE_APPROVED			= "APPROVED",
			MESSAGE_TRY_AGAIN			= "TRY AGAIN",
			MESSAGE_FAILURE				= "FAILURE",
			MESSAGE_DUPLICATE			= "DUPLICATE",
			MESSAGE_INVALID_MERCHANT	= "INVALID MERCHANT",
			MESSAGE_INVALID_TRANSACTION = "INVALID TRANSACTION", 
			MESSAGE_INVALID_CARD		= "INVALID CARD", 
			MESSAGE_EXPIRED_CARD		= "EXPIRED CARD", 
			MESSAGE_CALL_AUTH_CENTER	= "CALL AUTH CENTER",
			MESSAGE_DUPLICATE_APPROVED	= "DUPLICATE APPROVED", 
			MESSAGE_DECLINED			= "DECLINED", 
			MESSAGE_CONNECTION_ERROR	= "CONNECTION ERROR"
		;

		private const string 
			APPROVED_APPROVED			= "APPROVED", 
			APPROVED_FRAUD				= "FRAUD", 
			APPROVED_DECLINED			= "DECLINED"
		;
		#endregion

		#region Variables
		private string origResponse;
		private string origCSP, origTime, origRef, origError, origOrderID, origMessage, origCode, origTDate, origScore, origAuthResponse, origApproved, origAVS, origTax, origShipping;

		private static DateTimeFormatInfo timeFormatInfo;
		#endregion

		#region Constructors
		static LinkPointResponseV3_5() {
			timeFormatInfo = new DateTimeFormatInfo();
			timeFormatInfo.FullDateTimePattern = "ddd MMM dd HH:mm:ss yyyy";
		}

		public LinkPointResponseV3_5(string Response)
			: base() {
			this.origResponse = Response;
			parseResponse(this, Response);
		}
		#endregion

		#region Properties
		public string OriginalCSP { get { return this.origCSP; } }
		public string OriginalTime { get { return this.origTime; } }
		public string OriginalRef { get { return this.origRef; } }
		public string OriginalError { get { return this.origError; } }
		public string OriginalOrderID { get { return this.origOrderID; } }
		public string OriginalMessage { get { return this.origMessage; } }
		public string OriginalCode { get { return this.origCode; } }
		public string OriginalTDate { get { return this.origTDate; } }
		public string OriginalScore { get { return this.origScore; } }
		public string OriginalAuthResponse { get { return this.origAuthResponse; } }
		public string OriginalApproved { get { return this.origApproved; } }
		public string OriginalAVS { get { return this.origAVS; } }
		public string OriginalTax { get { return this.origTax; } }
		public string OriginalShipping { get { return this.origShipping; } }
		public string OriginalResponse { get { return this.origResponse; } }
		public bool HasAVS { get { return !string.IsNullOrEmpty(this.origAVS); } }
		#endregion

		#region Helper Methods
		public override bool VerifyAVS() {
			return IsValidTransactionFromAVS();
		}

		public bool IsValidTransactionFromAVS() {
			return StaticIsValidTransactionFromAVS(this);
		}

		public static bool StaticIsValidTransactionFromAVS(LinkPointResponseV3_5 Resp) {
			if (Resp == null) return false;
			if (!Resp.HasAVS) return true;
			if (Resp.CVV2 == LinkPointCVV2Response.NoMatch) return false;
			if (Resp.AVSAddress == LinkPointAVSResponse.NoMatch && Resp.AVSZipCode == LinkPointAVSResponse.NoMatch)
				return false;
			return true;
		}

		public static string StaticFormatAsResponseTime(DateTime Time) {
			return Time.ToString(timeFormatInfo.FullDateTimePattern, timeFormatInfo);
		}

		private static void parseAVS(LinkPointResponseV3_5 obj, string AVS) {
			if (string.IsNullOrEmpty(AVS))
				return;
			AVS = AVS.Trim();
			if (AVS.Length < 3)
				return;
			char addr = char.ToUpper(AVS[0]), zip = char.ToUpper(AVS[1]), cvv2 = char.ToUpper(AVS[2]);
			
			switch (addr) {
				case 'Y':
					obj.avsAddress = LinkPointAVSResponse.Match;
					break;
				case 'N':
					obj.avsAddress = LinkPointAVSResponse.NoMatch;
					break;
				case 'X':
					obj.avsAddress = LinkPointAVSResponse.Unavailable;
					break;
				default:
					obj.avsAddress = LinkPointAVSResponse.Unknown;
					break;
			}

			switch (zip) {
				case 'Y':
					obj.avsZip = LinkPointAVSResponse.Match;
					break;
				case 'N':
					obj.avsZip = LinkPointAVSResponse.NoMatch;
					break;
				case 'X':
					obj.avsZip = LinkPointAVSResponse.Unavailable;
					break;
				default:
					obj.avsZip = LinkPointAVSResponse.Unknown;
					break;
			}

			switch (cvv2) {
				case 'M':
					obj.cvv2Response = LinkPointCVV2Response.Match;
					break;
				case 'N':
					obj.cvv2Response = LinkPointCVV2Response.NoMatch;
					break;
				case 'Z':
				case 'P':
					obj.cvv2Response = LinkPointCVV2Response.Unknown;
					break;
				default:
					obj.cvv2Response = LinkPointCVV2Response.Unknown;
					break;
			}
		}

		private static void parseError(LinkPointResponseV3_5 obj, string Error, string Message, string Approved) {
			if (string.IsNullOrEmpty(Error) || !Error.StartsWith(ERROR_START))
				return;
			if (Error.Length < ERROR_START.Length + ERROR_END.Length)
				return;
			int endLoc = Error.IndexOf(ERROR_END, ERROR_START.Length);
			if (endLoc < 0)
				return;

			obj.errorNumber = Error.Substring(ERROR_START.Length, endLoc - ERROR_START.Length).Trim();
			obj.errorMessage = Error.Substring(endLoc + ERROR_END.Length).Trim();
			switch (Message) {
				case MESSAGE_APPROVED:
					obj.error = LinkPointError.None;
					break;
				case MESSAGE_CALL_AUTH_CENTER:
					obj.error = LinkPointError.CallAuthorizationCenter;
					break;
				case MESSAGE_DUPLICATE:
					obj.error = LinkPointError.LockedOut;
					break;
				case MESSAGE_DUPLICATE_APPROVED:
					obj.error = LinkPointError.ApprovedPreviously;
					break;
				case MESSAGE_EXPIRED_CARD:
					obj.error = LinkPointError.CreditCardExpired;
					break;
				case MESSAGE_FAILURE:
					obj.error = LinkPointError.ServerError;
					break;
				case MESSAGE_INVALID_CARD:
					obj.error = LinkPointError.InvalidCreditCardNumber;
					break;
				case MESSAGE_INVALID_MERCHANT:
					obj.error = LinkPointError.InvalidMerchant;
					break;
				case MESSAGE_INVALID_TRANSACTION:
					obj.error = LinkPointError.InvalidTransaction;
					break;
				case MESSAGE_TRY_AGAIN:
					obj.error = LinkPointError.TryAgain;
					break;
				case MESSAGE_DECLINED:
					obj.error = LinkPointError.Declined;
					break;
				case MESSAGE_CONNECTION_ERROR:
					obj.error = LinkPointError.ConnectionError;
					break;
				default:
					obj.error = LinkPointError.Unknown;
					break;
			}
		}

		private static void parseResponse(LinkPointResponseV3_5 obj, string Response) {
			//<r_csp>CSI</r_csp><r_time>Mon May 21 11:05:54 2007</r_time><r_ref>0003933727</r_ref><r_error></r_error><r_ordernum>80794882c8e249efa963f3e9a341a3d5</r_ordernum><r_message>APPROVED</r_message><r_code>1234560003933727:NNN :100017144878:</r_code><r_tdate>1179766616</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>APPROVED</r_approved><r_avs>NNN</r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:04:41 2007</r_time><r_ref></r_ref><r_error>SGS-002100: The server encountered a network error.</r_error><r_ordernum>1a0bd38a371a4ac0b1d41f93f8000190</r_ordernum><r_message>TRY AGAIN</r_message><r_code></r_code><r_tdate>1179766544</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:05:06 2007</r_time><r_ref></r_ref><r_error>SGS-002301: Transaction total amount is not valid. Please pass a valid amount.</r_error><r_ordernum>3f6844f5beb548ae94ea9c7ea8fb9da9</r_ordernum><r_message></r_message><r_code></r_code><r_tdate></r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved></r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:06:54 2007</r_time><r_ref></r_ref><r_error>SGS-002000: The server encountered an error.</r_error><r_ordernum>670c37f3c2cb474cacc7ce978c337952</r_ordernum><r_message>FAILURE</r_message><r_code></r_code><r_tdate>1179766677</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:08:19 2007</r_time><r_ref></r_ref><r_error>SGS-002000: The server encountered an error.</r_error><r_ordernum>bcbaf24d5cd54fca81d1cb2fd544aef5</r_ordernum><r_message>DUPLICATE</r_message><r_code></r_code><r_tdate>1179766761</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:26:10 2007</r_time><r_ref></r_ref><r_error>SGS-002000: The server encountered an error.</r_error><r_ordernum>0d0b1a85e5414be381d56fb3599be6b0</r_ordernum><r_message>INVALID MERCHANT</r_message><r_code></r_code><r_tdate>1179767832</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:46:33 2007</r_time><r_ref></r_ref><r_error>SGS-002300: There was an invalid transaction reported.</r_error><r_ordernum>af6b2620d4a0447d97cfc6ef15e01c10</r_ordernum><r_message>INVALID TRANSACTION</r_message><r_code></r_code><r_tdate>1179769055</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:47:04 2007</r_time><r_ref></r_ref><r_error>SGS-000001: D:Declined:NNN:</r_error><r_ordernum>13e95965914c48c397cbe53eda8ffc3f</r_ordernum><r_message>DECLINED</r_message><r_code></r_code><r_tdate>1179769086</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:47:23 2007</r_time><r_ref></r_ref><r_error>SGS-002300: The transaction has an invalid card number, MICR number or routing number.</r_error><r_ordernum>94fde71afbd34540b419d023e041886e</r_ordernum><r_message>INVALID CARD</r_message><r_code></r_code><r_tdate>1179769105</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:48:46 2007</r_time><r_ref></r_ref><r_error>SGS-002300: No credit card expiration year provided.</r_error><r_ordernum>8014dc604588415695632ae95a7aeb04</r_ordernum><r_message>EXPIRED CARD</r_message><r_code></r_code><r_tdate>1179769188</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:49:17 2007</r_time><r_ref></r_ref><r_error>SGS-000002: R:Referral (call voice center)</r_error><r_ordernum>6c1a8ff36d4a43f08915e0b5e903f051</r_ordernum><r_message>CALL AUTH CENTER</r_message><r_code></r_code><r_tdate>1179769219</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			//<r_csp></r_csp><r_time>Mon May 21 11:49:47 2007</r_time><r_ref></r_ref><r_error>SGS-002300: This transaction was previously approved.</r_error><r_ordernum>2d4bb37e778d4a1c8f9aabc889fc135e</r_ordernum><r_message>DUPLICATE APPROVED</r_message><r_code></r_code><r_tdate>1179769250</r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>
			obj.origResponse = Response;
			Response = "<Response>" + Response.Trim() + "</Response>";
			string tmpCSP = string.Empty, tmpTime = string.Empty, tmpRef = string.Empty, tmpError = string.Empty, tmpOrderNumber = string.Empty, tmpMessage = string.Empty, tmpCode = string.Empty, tmpTDate = string.Empty, tmpScore = string.Empty, tmpAuthResponse = string.Empty, tmpApproved = string.Empty, tmpAVS = string.Empty, tmpTax = string.Empty, tmpShipping = string.Empty;

			#region Read in XML
			XmlTextReader xtr = null;
			StringReader sr = null;

			try {
				sr = new StringReader(Response);
				xtr = new XmlTextReader(sr);
				
				while (xtr.Read()) {
					if (xtr.NodeType != XmlNodeType.Element)
						continue;

					switch (xtr.LocalName.ToLower()) {
						case XML_TAG_CSP:
							tmpCSP = xtr.ReadString();
							break;
						case XML_TAG_TIME:
							tmpTime = xtr.ReadString();
							break;
						case XML_TAG_REF:
							tmpRef = xtr.ReadString();
							break;
						case XML_TAG_ERROR:
							tmpError = xtr.ReadString();
							break;
						case XML_TAG_ORDER_NUMBER:
							tmpOrderNumber = xtr.ReadString();
							break;
						case XML_TAG_MESSAGE:
							tmpMessage = xtr.ReadString();
							break;
						case XML_TAG_CODE:
							tmpCode = xtr.ReadString();
							break;
						case XML_TAG_TDATE:
							tmpTDate = xtr.ReadString();
							break;
						case XML_TAG_SCORE:
							tmpScore = xtr.ReadString();
							break;
						case XML_TAG_AUTH_RESPONSE:
							tmpAuthResponse = xtr.ReadString();
							break;
						case XML_TAG_APPROVED:
							tmpApproved = xtr.ReadString();
							break;
						case XML_TAG_AVS:
							tmpAVS = xtr.ReadString();
							break;
						case XML_TAG_TAX:
							tmpTax = xtr.ReadString();
							break;
						case XML_TAG_SHIPPING:
							tmpShipping = xtr.ReadString();
							break;
					}
				}

			} finally {
				if (xtr != null) xtr.Close();
				if (sr != null) {
					sr.Close();
					sr.Dispose();
				}
			}

			obj.origTime			= tmpTime;
			obj.origTDate			= tmpTDate;
			obj.origScore			= tmpScore;
			obj.origRef				= tmpRef;
			obj.origOrderID			= tmpOrderNumber;
			obj.origMessage			= tmpMessage;
			obj.origError			= tmpError;
			obj.origCSP				= tmpCSP;
			obj.origCode			= tmpCode;
			obj.origAVS				= tmpAVS;
			obj.origAuthResponse	= tmpAuthResponse;
			obj.origApproved		= tmpApproved;
			obj.origTax				= tmpTax;
			obj.origShipping		= tmpShipping;
			#endregion
			
			DateTime respTime;
			if (!DateTime.TryParseExact(tmpTime, timeFormatInfo.FullDateTimePattern, timeFormatInfo, DateTimeStyles.AllowWhiteSpaces, out respTime))
				respTime = DateTime.Now;
			obj.time = respTime;
			obj.reference = tmpRef;
			obj.orderID = tmpOrderNumber;

			if (string.IsNullOrEmpty(tmpTax) || !double.TryParse(tmpTax, out obj.tax))
				obj.tax = 0.0D;
			if (string.IsNullOrEmpty(tmpShipping) || !double.TryParse(tmpShipping, out obj.tax))
				obj.shipping = 0.0D;
			if (string.IsNullOrEmpty(tmpScore) || !byte.TryParse(tmpScore, out obj.score))
				obj.score = (byte)(obj.MaximumPossibleScore + 1);


			if (!string.IsNullOrEmpty(tmpError))
				parseError(obj, tmpError, tmpMessage, tmpApproved);

			if (!string.IsNullOrEmpty(tmpAVS))
				parseAVS(obj, tmpAVS);

			if (obj.error == LinkPointError.None && APPROVED_APPROVED.Equals(tmpApproved, StringComparison.InvariantCultureIgnoreCase))
				obj.setSuccess(true);
			else
				obj.setSuccess(false);
		}
		#endregion
	}
}
