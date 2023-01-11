#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common {
	public class Gateway {
		#region Enums
		///<summary>How often a periodic payment will be made.</summary>
		public enum Period : int {
			///<summary>Annual installments</summary>
			Annually = 0,
			///<summary>Monthly installments</summary>
			Monthly = 1,
			///<summary>Bimonthly (every other month) installments</summary>
			BiMonthly = 2,
			///<summary>Weekly installments</summary>
			Weekly = 3,
			///<summary>Biweekly (every other week) installments</summary>
			BiWeekly = 4,
			///<summary>Daily installments</summary>
			Daily = 5
		}
		#endregion

		#region Constants
		private const string
			EXC_MSG_PERIODICITY_RANGE	= "The periodicity must be > 0 and < 1000."
		;
		#endregion
	}

	#region Helper Classes
	public class PeriodInformation {
		private const string INVALID_PERIOD = "The period must be of format XN where X is d, m, or y and N is an integer from 1-999";
		private string origPeriod = null;
		private int periodCount = 0;
		private Gateway.Period period = Gateway.Period.Annually;

		public string OriginalPeriod { get { return this.origPeriod; } }
		public int PeriodCount { get { return this.periodCount; } }
		public Gateway.Period Period { get { return this.period; } }

		public PeriodInformation(string Period) {
			this.origPeriod = Period;
			Parse(this.origPeriod, out this.periodCount, out this.period);
		}

		public static void Parse(string OriginalPeriod, out int PeriodCount, out Gateway.Period Period) {
			string lc = OriginalPeriod.ToLower();
			int count = 0;
			Gateway.Period per;
			
			switch (lc) {
				case "monthly":
					per = Gateway.Period.Monthly;
					break;
				case "daily":
					per = Gateway.Period.Daily;
					break;
				case "yearly":
				case "annually":
					per = Gateway.Period.Annually;
					break;
				case "weekly":
					per = Gateway.Period.Weekly;
					break;
				case "biweekly":
					per = Gateway.Period.BiWeekly;
					break;
				case "bimonthly":
					per = Gateway.Period.BiMonthly;
					break;
				default:
					if (lc.Length < 2 || lc.Length > 4)
						throw new ArgumentException(INVALID_PERIOD);
					
					switch (lc[0]) {
						case 'd':
							per = Gateway.Period.Daily;
							break;
						case 'm':
							per = Gateway.Period.Monthly;
							break;
						case 'y':
							per = Gateway.Period.Annually;
							break;
						default:
							throw new ArgumentException("Invalid period. " + INVALID_PERIOD);
					}

					if (!int.TryParse(lc.Substring(1), out count))
						throw new ArgumentException("Invalid integer specified after the periodicity");
					if (count < 1 || count > 999)
						throw new ArgumentException("Invalid integer range. Must be between 1 and 999.");
					break;
			}
			PeriodCount = count;
			Period = per;
		}
	}
	#endregion

	#region Exceptions
	public class GatewayException : Exception {
		public GatewayException() : base() { }
		public GatewayException(string Message) : base(Message) { }
		public GatewayException(string Message, Exception InnerException) : base(Message, InnerException) { }
		public GatewayException(System.Runtime.Serialization.SerializationInfo Info, System.Runtime.Serialization.StreamingContext Context) : base(Info, Context) { }
	}

	public class GatewayTryAgainException : Exception {
		public GatewayTryAgainException() : base() { }
		public GatewayTryAgainException(string Message) : base(Message) { }
		public GatewayTryAgainException(string Message, Exception InnerException) : base(Message, InnerException) { }
		public GatewayTryAgainException(System.Runtime.Serialization.SerializationInfo Info, System.Runtime.Serialization.StreamingContext Context) : base(Info, Context) { }
	}
	#endregion
}
