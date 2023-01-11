#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways {
	public class AbstractResponse {
		#region Variables
		private bool success;
		#endregion

		#region Constructors
		protected AbstractResponse() : this(false) {
		}

		protected AbstractResponse(bool Success) {
			this.success = Success;
		}
		#endregion

		#region Properties
		public bool Success {
			get { return this.success; }
		}
		#endregion 

		#region Helper Methods
		protected void setSuccess(bool value) {
			this.success = value;
		}
		#endregion
	}
}
