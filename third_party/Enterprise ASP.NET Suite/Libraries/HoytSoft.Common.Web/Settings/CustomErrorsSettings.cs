#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class CustomErrorsSettings : HoytSoft.Common.Settings.ISectionHandler {
		private CustomErrorsMode mode = CustomErrorsMode.RemoteOnly;
		private Dictionary<int, string> statusCodeRedirects;
		private string defaultRedirect = string.Empty, eventLogSource = string.Empty, errorEmailAddress = string.Empty;
		private bool useEventLog = false, useErrorEmail = false;

		public Settings.Section SectionName { get { return Settings.Section.CustomErrors; } }
		public string ConfigSectionName { get { return "CustomErrors"; } }
		public CustomErrorsMode Mode { get { return this.mode; } }
		public string DefaultRedirect { get { return this.defaultRedirect; } }
		public Dictionary<int, string> StatusCodeRedirects { get { return this.statusCodeRedirects; } }
		public bool UseEventLog { get { return this.useEventLog; } }
		public string EventLogSource { get { return this.eventLogSource; } }
		public bool UseErrorEmail { get { return this.useErrorEmail; } }
		public string ErrorEmailAddress { get { return this.errorEmailAddress; } }

		public object Create(object Parent, object Context, XmlNode Section) {
			string defaultRedir, testMode, testUseEventLog, testEventLogSource, testUseErrorEmail, testErrorEmailAddress;

			if (Section.Attributes["mode"] != null && !string.IsNullOrEmpty((testMode = Section.Attributes["mode"].Value)))
				this.mode = (CustomErrorsMode)Enum.Parse(typeof(CustomErrorsMode), testMode, true);

			if (Section.Attributes["defaultRedirect"] != null && !string.IsNullOrEmpty((defaultRedir = Section.Attributes["defaultRedirect"].Value)))
				this.defaultRedirect = defaultRedir;

			if (Section.Attributes["useEventLog"] != null && !string.IsNullOrEmpty((testUseEventLog = Section.Attributes["useEventLog"].Value)))
				this.useEventLog = bool.Parse(testUseEventLog);

			if (Section.Attributes["eventLogSource"] != null && !string.IsNullOrEmpty((testEventLogSource = Section.Attributes["eventLogSource"].Value)))
				this.eventLogSource = testEventLogSource;

			if (Section.Attributes["useErrorEmail"] != null && !string.IsNullOrEmpty((testUseErrorEmail = Section.Attributes["useErrorEmail"].Value)))
				this.useErrorEmail = bool.Parse(testUseErrorEmail);

			if (Section.Attributes["errorEmailAddress"] != null && !string.IsNullOrEmpty((testErrorEmailAddress = Section.Attributes["errorEmailAddress"].Value)))
				this.errorEmailAddress = testErrorEmailAddress;

			if (string.IsNullOrEmpty(this.eventLogSource))
				this.eventLogSource = 
					 System.Web.HttpContext.Current.Request.Url.Host + 
					(System.Web.HttpContext.Current.Request.Url.IsDefaultPort ? "" : ":" + System.Web.HttpContext.Current.Request.Url.Port) +
					(System.Web.HttpContext.Current.Request.ApplicationPath == "/" ? "/" : System.Web.HttpContext.Current.Request.ApplicationPath + "/");

			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					string statusCode = null;
					string redirect = null;
					int iStatusCode = 0;

					foreach (XmlNode xn in xnl) {
						if (
							   xn.Attributes["statusCode"] != null
							&& xn.Attributes["redirect"] != null
							&& !string.IsNullOrEmpty((statusCode = xn.Attributes["statusCode"].Value))
							&& !string.IsNullOrEmpty((redirect = xn.Attributes["redirect"].Value))
						) {
							if (!int.TryParse(statusCode, out iStatusCode))
								throw new XmlException("The statusCode " + statusCode + " is not a valid integer");

							if (this.statusCodeRedirects == null)
								this.statusCodeRedirects = new Dictionary<int, string>();

							this.statusCodeRedirects.Add(iStatusCode, redirect);
						}
					}
				}
			}
			return this;
		} //create

	} //class
} //namespace
