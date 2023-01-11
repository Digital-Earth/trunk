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
using System.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class EmailSettings : HoytSoft.Common.Settings.ISectionHandler {
		public const string 
			DEFAULT						= "default",
			SETTINGS_SECTION			= "Email",
			ATTRIB_SMTP_SERVER			= "smtpServer", 
			ATTRIB_SMTP_PORT			= "smtpServerPort", 
			ATTRIB_SMTP_LOGIN			= "smtpServerLogin", 
			ATTRIB_SMTP_PASSWORD		= "smtpServerPassword", 
			ATTRIB_EMAIL_FROM			= "emailFrom", 
			ATTRIB_EMAIL_DISPLAYNAME	= "emailFromDisplayName";
		private string server = "", login = "", password = "", from = "", displayName = "";
		private int port = 25;

		public Settings.Section SectionName	{ get { return Settings.Section.Email; } }
		public string ConfigSectionName		{ get { return SETTINGS_SECTION; } }
		public string Server				{ get { return this.server; } }
		public string Login					{ get { return this.login; } }
		public string Password				{ get { return this.password; } }
		public string From					{ get { return this.from; } }
		public string FromDisplayName		{ get { return this.displayName; } }
		public int    Port					{ get { return this.port; } }

		public object Create(object Parent, object Context, XmlNode Section) {
			string testServer, testPort, testLogin, testPassword, testFrom, testDisplayName;
			int iTestPort = 25;

			if (Section.Attributes[ATTRIB_SMTP_SERVER] != null && !string.IsNullOrEmpty((testServer = Section.Attributes[ATTRIB_SMTP_SERVER].Value)))
				this.server = testServer;

			if (Section.Attributes[ATTRIB_SMTP_LOGIN] != null && !string.IsNullOrEmpty((testLogin = Section.Attributes[ATTRIB_SMTP_LOGIN].Value)))
				this.login = testLogin;

			if (Section.Attributes[ATTRIB_SMTP_PASSWORD] != null && !string.IsNullOrEmpty((testPassword = Section.Attributes[ATTRIB_SMTP_PASSWORD].Value)))
				this.password = testPassword;

			if (Section.Attributes[ATTRIB_EMAIL_FROM] != null && !string.IsNullOrEmpty((testFrom = Section.Attributes[ATTRIB_EMAIL_FROM].Value)))
				this.from = testFrom;

			if (Section.Attributes[ATTRIB_EMAIL_DISPLAYNAME] != null && !string.IsNullOrEmpty((testDisplayName = Section.Attributes[ATTRIB_EMAIL_DISPLAYNAME].Value)))
				this.displayName = testDisplayName;

			if (Section.Attributes[ATTRIB_SMTP_PORT] != null && !string.IsNullOrEmpty((testPort = Section.Attributes[ATTRIB_SMTP_PORT].Value)) && int.TryParse(testPort, out iTestPort))
				this.port = iTestPort;

			return this;
		}

	} //class
} //namespace
