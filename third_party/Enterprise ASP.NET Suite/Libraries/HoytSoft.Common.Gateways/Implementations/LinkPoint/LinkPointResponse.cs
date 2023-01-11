#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public abstract class LinkPointResponse : AbstractResponse {
		#region Variables
		protected DateTime time;
		protected string reference;
		protected string orderID;
		protected double tax, shipping;
		protected string errorNumber = string.Empty;
		protected string errorMessage = string.Empty;
		protected byte score;
		protected LinkPointError error = LinkPointError.None;
		protected LinkPointAVSResponse avsAddress = LinkPointAVSResponse.Unknown;
		protected LinkPointAVSResponse avsZip = LinkPointAVSResponse.Unknown;
		protected LinkPointCVV2Response cvv2Response = LinkPointCVV2Response.Unknown;
		#endregion

		#region Constructors
		public LinkPointResponse()
			: base() {
		}

		public LinkPointResponse(bool Success)
			: base(Success) {
		}
		#endregion

		#region Properties
		public DateTime Time { get { return this.time; } }
		public double Tax { get { return this.tax; } }
		public double Shipping { get { return this.shipping; } }
		public string Reference { get { return this.reference; } }
		public string OrderID { get { return this.orderID; } }
		public string ErrorNumber { get { return this.errorNumber; } }
		public string ErrorMessage { get { return this.errorMessage; } }
		public bool Declined { get { return !this.Success; } }
		public byte Score { get { return this.score; } }
		public byte MaximumPossibleScore { get { return 99; } }
		public byte MinimumPossibleScore { get { return 0; } }
		public bool HasScore { get { return (this.score <= this.MaximumPossibleScore && this.score >= this.MinimumPossibleScore); } }
		public bool IsScoreLow { get { return (this.score < 70); } }
		public LinkPointError Error { get { return this.error; } }
		public LinkPointAVSResponse AVSAddress { get { return this.avsAddress; } }
		public LinkPointAVSResponse AVSZipCode { get { return this.avsZip; } }
		public LinkPointCVV2Response CVV2 { get { return this.cvv2Response; } }
		#endregion

		public abstract bool VerifyAVS();
	}
}
