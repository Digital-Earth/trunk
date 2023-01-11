#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Security.Cryptography.X509Certificates;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public abstract class LinkPointGateway : AbstractGateway {
		#region Variables
		private Version version;
		#endregion

		#region Constructors
		protected LinkPointGateway(int VersionMajor, int VersionMinor) {
			this.version = new Version(VersionMajor, VersionMinor);
		}
		#endregion

		#region Abstract Methods
		public abstract LinkPointResponse CreditCardSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointOrderOptionsResultType OrderType, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCreditCardInformation CCInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items);
		public abstract LinkPointResponse VirtualCheckSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointOrderOptionsResultType OrderType, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCheckInformation CheckInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items);
		#endregion

		#region Properties
		public override Version Version {
			get { return this.version; }
		}
		#endregion

		#region Helper Methods
		public virtual LinkPointResponse CreditCardSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCreditCardInformation CCInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items) {
			return CreditCardSale(Host, Port, StoreNumber, StoreCertificate, LinkPointOrderOptionsResultType.Live, TransInfo, PaymentInfo, CCInfo, BillingInfo, ShippingInfo, Items);
		}

		public virtual LinkPointResponse VirtualCheckSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCheckInformation CheckInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items) {
			return VirtualCheckSale(Host, Port, StoreNumber, StoreCertificate, LinkPointOrderOptionsResultType.Live, TransInfo, PaymentInfo, CheckInfo, BillingInfo, ShippingInfo, Items);
		}
		#endregion
	}
}
