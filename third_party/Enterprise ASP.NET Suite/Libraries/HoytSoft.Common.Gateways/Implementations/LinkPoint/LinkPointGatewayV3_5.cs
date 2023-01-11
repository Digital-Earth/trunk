#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Text;
using System.Security.Cryptography.X509Certificates;
using System.IO;
using System.Xml;
using System.Diagnostics;
using System.Net.Security;
using System.Net.Sockets;

namespace HoytSoft.Common.Gateways.LinkPoint {
	internal sealed class LinkPointGatewayV3_5 : LinkPointGateway {
		#region Constants
		private const string
			XML_TAG_ORDER			= "order",
			XML_TAG_ORDEROPTIONS	= "orderoptions",
			XML_TAG_ORDERTYPE		= "ordertype",
			XML_TAG_RESULT			= "result", 
			XML_TAG_MERCHANTINFO	= "merchantinfo",
			XML_TAG_CONFIGFILE		= "configfile"
		;
		#endregion

		#region Constructors
		public LinkPointGatewayV3_5()
			: base(3, 5) {
		}
		#endregion

		#region Helper Methods
		private static string getOrderOptionsResultTypeString(LinkPointOrderOptionsResultType ResultType) {
			switch (ResultType) {
				case LinkPointOrderOptionsResultType.Live:
					return "LIVE";
				case LinkPointOrderOptionsResultType.Good:
					return "GOOD";
				case LinkPointOrderOptionsResultType.Duplicate:
					return "DUPLICATE";
				case LinkPointOrderOptionsResultType.Decline:
					return "DECLINE";
			}
			return string.Empty;
		}

		private static void initializeXML(XmlWriter xw, string StoreNumber, LinkPointTransactionInformation TransInfo, LinkPointOrderOptionsResultType ResultType) {
			if (TransInfo != null)
				initializeXML(xw, StoreNumber, TransInfo.GetTransactionTypeString(), getOrderOptionsResultTypeString(ResultType));
			else
				initializeXML(xw, StoreNumber, "SALE", getOrderOptionsResultTypeString(ResultType));
		}

		private static void initializeXML(XmlWriter xw, string StoreNumber, string TransactionType, string ResultType) {
			xw.WriteStartElement(XML_TAG_ORDEROPTIONS);
			xw.WriteElementString(XML_TAG_ORDERTYPE, TransactionType);
			xw.WriteElementString(XML_TAG_RESULT, ResultType);
			xw.WriteEndElement();

			xw.WriteStartElement(XML_TAG_MERCHANTINFO);
			xw.WriteElementString(XML_TAG_CONFIGFILE, StoreNumber);
			xw.WriteEndElement();
		}

		private static LinkPointResponseV3_5 processRequest(string Host, int Port, X509Certificate StoreCertificate, string PostData, Common.Net.CertificateValidation Validation) {
			try {
				string response = Common.Net.HttpRequest.RawSSLRequest(Host, Port, PostData, Validation, new X509Certificate[] { StoreCertificate });
				return new LinkPointResponseV3_5(response);
			} catch (SocketException se) {
				return new LinkPointResponseV3_5(string.Format("<r_csp></r_csp><r_time>{0}</r_time><r_ref></r_ref><r_error>SGS-999999: {1}</r_error><r_ordernum></r_ordernum><r_message>CONNECTION ERROR</r_message><r_code></r_code><r_tdate></r_tdate><r_score></r_score><r_authresponse></r_authresponse><r_approved>DECLINED</r_approved><r_avs></r_avs>", LinkPointResponseV3_5.StaticFormatAsResponseTime(DateTime.Now), se.SocketErrorCode));
			} catch (Exception) {
			}
			return null;
		}
		#endregion

		public override LinkPointResponse CreditCardSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointOrderOptionsResultType OrderType, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCreditCardInformation CCInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items) {
			if (PaymentInfo == null)
				throw new ArgumentNullException("PaymentInfo");
			if (CCInfo == null)
				throw new ArgumentNullException("CCInfo");

			StringWriter sw = null;
			XmlTextWriter xtw = null;
			try {
				sw = new StringWriter();
				xtw = new XmlTextWriter(sw);
				xtw.Formatting = Formatting.Indented;
				xtw.IndentChar = '\t';
				xtw.Indentation = 1;
				xtw.QuoteChar = '\"';

				if (TransInfo == null)
					TransInfo = new LinkPointTransactionInformation(LinkPointTransactionType.Sale, LinkPointTransactionOrigin.Internet);

				xtw.WriteStartElement(XML_TAG_ORDER);
				initializeXML(xtw, StoreNumber, TransInfo, OrderType);
				
				if (TransInfo != null)
					TransInfo.WriteXML(xtw);

				PaymentInfo.WriteXML(xtw);
				CCInfo.WriteXML(xtw);
				
				if (ShippingInfo != null)
					ShippingInfo.WriteXML(xtw);
				if (BillingInfo != null)
					BillingInfo.WriteXML(xtw);
				if (Items != null)
					Items.WriteXML(xtw);

				xtw.WriteEndElement();

				xtw.Flush();

				return processRequest(Host, Port, StoreCertificate, sw.ToString(), Common.Net.CertificateValidation.AllowAll);

			} catch(Exception exc) {
				throw exc;
				//Debug.WriteLine(exc);
			} finally {
				if (sw != null) {
					sw.Close();
					sw.Dispose();
				}
				if (xtw != null)
					xtw.Close();
			}
		}

		public override LinkPointResponse VirtualCheckSale(string Host, int Port, string StoreNumber, X509Certificate StoreCertificate, LinkPointOrderOptionsResultType OrderType, LinkPointTransactionInformation TransInfo, LinkPointPaymentInformation PaymentInfo, LinkPointCheckInformation CheckInfo, LinkPointBillingInformation BillingInfo, LinkPointShippingInformation ShippingInfo, LinkPointItemCollection Items) {
			if (PaymentInfo == null)
				throw new ArgumentNullException("PaymentInfo");
			if (CheckInfo == null)
				throw new ArgumentNullException("CheckInfo");
			if (BillingInfo == null)
				throw new ArgumentNullException("BillingInfo");

			StringWriter sw = null;
			XmlTextWriter xtw = null;
			try {
				sw = new StringWriter();
				xtw = new XmlTextWriter(sw);
				xtw.Formatting = Formatting.Indented;
				xtw.IndentChar = '\t';
				xtw.Indentation = 1;
				xtw.QuoteChar = '\"';

				if (TransInfo == null)
					TransInfo = new LinkPointTransactionInformation(LinkPointTransactionType.Sale, LinkPointTransactionOrigin.Internet);

				xtw.WriteStartElement(XML_TAG_ORDER);
				initializeXML(xtw, StoreNumber, TransInfo, OrderType);

				if (TransInfo != null)
					TransInfo.WriteXML(xtw);

				PaymentInfo.WriteXML(xtw);
				CheckInfo.WriteXML(xtw);
				BillingInfo.WriteXML(xtw);

				if (ShippingInfo != null)
					ShippingInfo.WriteXML(xtw);
				if (Items != null)
					Items.WriteXML(xtw);

				xtw.WriteEndElement();

				xtw.Flush();

				return processRequest(Host, Port, StoreCertificate, sw.ToString(), Common.Net.CertificateValidation.AllowAll);

			} catch (Exception exc) {
				throw exc;
				//Debug.WriteLine(exc);
			} finally {
				if (sw != null) {
					sw.Close();
					sw.Dispose();
				}
				if (xtw != null)
					xtw.Close();
			}
		}
	}
}
