#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.IO;
using System.Xml;
using System.Web;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Net.Mail;
using System.Threading;
using System.Globalization;
using System.Reflection;

namespace HoytSoft.Common.Configuration {
	public class EmailTemplates {
		#region Constants
		///<summary>The default name of the XML file containing the e-mail templates, the name to use to specify something's default, and the name to use to specify nothing.</summary>
		public const string
			FILE_EMAIL_TEMPLATES_MASTERS	= "Web.email.master.config", 
			FILE_EMAIL_TEMPLATES_MESSAGES	= "Web.email.config", 
			DEFAULT_NAME					= "default",
			NONE_NAME						= "none" 
		;

		private const string
			XPATH_MASTER					= "Email/Master", 
			XPATH_MASTER_CULTURE			= "Culture", 
			XPATH_MASTER_HTML				= "HTML", 
			XPATH_MASTER_TEXT				= "Text",
 
			XPATH_MESSAGES					= "Email/Message", 
			XPATH_MESSAGES_CULTURE			= "Culture", 
			XPATH_MESSAGES_SUBJECT			= "Subject", 
			XPATH_MESSAGES_HTML				= "Body/HTML", 
			XPATH_MESSAGES_TEXT				= "Body/Text" 
		;

		private const string 
			XML_MASTER_ATTRIB_NAME		= "name", 
			XML_MASTER_CULTURE_NAME		= "name", 
			XML_MESSAGE_ATTRIB_NAME		= "name", 
			XML_MESSAGE_ATTRIB_MASTER	= "master", 
			XML_MESSAGE_CULTURE_NAME	= "name"
		;

		///<summary>Various variables that you may find in the templates</summary>
		public const string
			VAR_BASE_URL					= "{BaseUrl}", 
			VAR_FIRST_NAME					= "{FirstName}",
			VAR_LAST_NAME					= "{LastName}",
			VAR_COMPANY_NAME				= "{CompanyName}",
			VAR_EMAIL						= "{Email}",
			VAR_PHONE						= "{Phone}",
			VAR_FAX							= "{Fax}",
			VAR_ADDRESS1					= "{Address1}",
			VAR_ADDRESS2					= "{Address2}",
			VAR_CITY						= "{City}",
			VAR_STATE						= "{State}",
			VAR_ZIP							= "{Zip}", 
			VAR_HEAR						= "{Hear}",
			VAR_SUBSCRIBE					= "{Subscribe}", 
			VAR_LOGIN						= "{Login}",
			VAR_PASSWORD					= "{Password}", 
			VAR_MEMBER_FIRST_NAME			= "{MemberFirstName}", 
			VAR_MEMBER_LAST_NAME			= "{MemberLastName}", 
			VAR_MEMBER_EMAIL				= "{MemberEmail}", 
			VAR_MEMBER_FAX					= "{MemberFax}", 
			VAR_CLIENT_NAME					= "{ClientName}", 
			VAR_CLIENT_EMAIL				= "{ClientEmail}", 
			VAR_ID							= "{ID}", 
			VAR_SALES_ID					= "{SalesID}", 
			VAR_BILLING_FIRST_NAME			= "{BillingFirstName}",
			VAR_BILLING_LAST_NAME			= "{BillingLastName}",
			VAR_BILLING_COMPANY_NAME		= "{BillingCompanyName}",
			VAR_BILLING_EMAIL				= "{BillingEmail}",
			VAR_BILLING_PHONE				= "{BillingPhone}",
			VAR_BILLING_FAX					= "{BillingFax}",
			VAR_BILLING_ADDRESS1			= "{BillingAddress1}",
			VAR_BILLING_ADDRESS2			= "{BillingAddress2}",
			VAR_BILLING_CITY				= "{BillingCity}",
			VAR_BILLING_STATE				= "{BillingState}",
			VAR_BILLING_ZIP					= "{BillingZip}" 
		;
		#endregion

		#region Variables
		private static bool loaded;
		private static Dictionary<string, Message> dictMessages;
		private static Dictionary<string, Master> dictMasters;
		#endregion

		#region Constructors
		static EmailTemplates() {
			loaded = false;
			if (dictMessages == null)
				dictMessages = new Dictionary<string, Message>();
			if (dictMasters == null)
				dictMasters = new Dictionary<string, Master>();
			Load();
		}

		protected static void Load() {
			if (loaded) return;
			loaded = true;
			Refresh();
		}
		#endregion

		#region Main Methods
		protected static string findBaseUrl(bool UseUnsecureBaseUrl) {
			if (UseUnsecureBaseUrl)
				return findUnsecureBaseUrl();
			else
				return findSecureBaseUrl();
		}

		protected static string findSecureBaseUrl() {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security != null)
				return security.SecureSite;
			else
				return string.Empty;
		}

		protected static string findUnsecureBaseUrl() {
			Internal.HttpSecuritySettings security = Settings.From<Internal.HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (security != null)
				return security.UnsecureSite;
			else
				return string.Empty;
		}

		private static void loadDocumentMasters(XmlDocument doc) {
			loadDocumentMasters(doc, true);
		}

		private static void loadDocumentMasters(XmlDocument doc, bool clear) {
			if (doc == null) return;

			XmlNode x = null;
			XmlNodeList xnl = null;
			string name = string.Empty;
			string html = string.Empty;
			string text = string.Empty;
			string cultureName = string.Empty;
			Master master = null;

			if (clear)
				dictMasters.Clear();

			if ((xnl = doc.SelectNodes(XPATH_MASTER)) != null && xnl.Count > 0) {
				foreach (XmlNode xn in xnl) {
					name = string.Empty;
					master = null;

					if (xn.Attributes[XML_MASTER_ATTRIB_NAME] != null && !string.IsNullOrEmpty(xn.Attributes[XML_MASTER_ATTRIB_NAME].InnerText))
						name = xn.Attributes[XML_MASTER_ATTRIB_NAME].InnerText;
					else
						name = DEFAULT_NAME;

					if (string.IsNullOrEmpty(name) || DEFAULT_NAME.Equals(name, StringComparison.InvariantCultureIgnoreCase))
						name = DEFAULT_NAME;

					XmlNodeList xnlCultures = null;
					if ((xnlCultures = xn.SelectNodes(XPATH_MASTER_CULTURE)) != null && xnlCultures.Count > 0) {
						//Create new master only if there's defined cultures for it.
						master = new Master(name);

						//Go through each culture and add them to the master's collection.
						foreach (XmlNode xnCulture in xnlCultures) {
							cultureName = string.Empty;
							html = string.Empty;
							text = string.Empty;

							//Pull out the culture name
							if (xnCulture.Attributes[XML_MASTER_CULTURE_NAME] != null && !string.IsNullOrEmpty(xnCulture.Attributes[XML_MASTER_CULTURE_NAME].InnerText))
								cultureName = xnCulture.Attributes[XML_MASTER_CULTURE_NAME].InnerText;
							else
								cultureName = DEFAULT_NAME;

							if (string.IsNullOrEmpty(cultureName) || DEFAULT_NAME.Equals(cultureName, StringComparison.InvariantCultureIgnoreCase))
								cultureName = DEFAULT_NAME; //StaticGetDefaultCultureName();

							//Now pull out the HTML and Text...
							if ((x = xnCulture.SelectSingleNode(XPATH_MASTER_HTML)) != null && !string.IsNullOrEmpty(x.InnerText))
								html = x.InnerText;
							if ((x = xnCulture.SelectSingleNode(XPATH_MASTER_TEXT)) != null && !string.IsNullOrEmpty(x.InnerText))
								text = x.InnerText;

							//Add it to our list of masters
							if (!string.IsNullOrEmpty(cultureName) && (!string.IsNullOrEmpty(html) || !string.IsNullOrEmpty(text)))
								master.AddCulture(cultureName, html, text);
						} //foreach
					} //if

					if (!string.IsNullOrEmpty(name) && master != null)
						dictMasters[name] = master;
				} //foreach
			} //if
		}

		private static void loadDocumentMessages(XmlDocument doc) {
			loadDocumentMessages(doc, true);
		}

		private static void loadDocumentMessages(XmlDocument doc, bool clear) {
			if (doc == null) return;

			XmlNode x = null;
			XmlNodeList xnl = null;
			string name = string.Empty;
			string html = string.Empty;
			string text = string.Empty;
			string subject = string.Empty;
			string master = string.Empty;
			string cultureName = string.Empty;
			Message msg = null;

			if (clear)
				dictMessages.Clear();

			if ((xnl = doc.SelectNodes(XPATH_MESSAGES)) != null && xnl.Count > 0) {
				foreach (XmlNode xn in xnl) {
					name = string.Empty;
					master = string.Empty;
					msg = null;

					#region read in the name
					if (xn.Attributes[XML_MESSAGE_ATTRIB_NAME] != null && !string.IsNullOrEmpty(xn.Attributes[XML_MESSAGE_ATTRIB_NAME].InnerText))
						name = xn.Attributes[XML_MESSAGE_ATTRIB_NAME].InnerText;
					else
						name = DEFAULT_NAME;

					if (string.IsNullOrEmpty(name) || DEFAULT_NAME.Equals(name, StringComparison.InvariantCultureIgnoreCase))
						name = DEFAULT_NAME;
					#endregion

					#region read in the master name
					if (xn.Attributes[XML_MESSAGE_ATTRIB_MASTER] != null && !string.IsNullOrEmpty(xn.Attributes[XML_MESSAGE_ATTRIB_MASTER].InnerText))
						master = xn.Attributes[XML_MESSAGE_ATTRIB_MASTER].InnerText;
					else
						master = DEFAULT_NAME;

					if (string.IsNullOrEmpty(master) || DEFAULT_NAME.Equals(master, StringComparison.InvariantCultureIgnoreCase))
						master = DEFAULT_NAME;
					#endregion

					//Loop through cultures
					XmlNodeList xnlCultures = null;
					if ((xnlCultures = xn.SelectNodes(XPATH_MESSAGES_CULTURE)) != null && xnlCultures.Count > 0) {
						//Find the message's master template.
						if (!string.IsNullOrEmpty(name) && !NONE_NAME.Equals(master, StringComparison.InvariantCultureIgnoreCase) && dictMasters != null && dictMasters.ContainsKey(master))
							msg = new Message(dictMasters[master], name);
						else
							continue;

						foreach (XmlNode xnCulture in xnlCultures) {
							html = string.Empty;
							text = string.Empty;
							subject = string.Empty;
							cultureName = string.Empty;

							#region pull out the culture name, subject, html, and text
							if (xnCulture.Attributes[XML_MESSAGE_CULTURE_NAME] != null && !string.IsNullOrEmpty(xnCulture.Attributes[XML_MESSAGE_CULTURE_NAME].InnerText))
								cultureName = xnCulture.Attributes[XML_MESSAGE_CULTURE_NAME].InnerText;
							else
								cultureName = DEFAULT_NAME;

							if (string.IsNullOrEmpty(cultureName) || DEFAULT_NAME.Equals(cultureName, StringComparison.InvariantCultureIgnoreCase))
								cultureName = DEFAULT_NAME;

							if ((x = xnCulture.SelectSingleNode(XPATH_MESSAGES_SUBJECT)) != null && !string.IsNullOrEmpty(x.InnerText))
								subject = x.InnerText;
							if ((x = xnCulture.SelectSingleNode(XPATH_MESSAGES_HTML)) != null && !string.IsNullOrEmpty(x.InnerText))
								html = x.InnerText;
							if ((x = xnCulture.SelectSingleNode(XPATH_MESSAGES_TEXT)) != null && !string.IsNullOrEmpty(x.InnerText))
								text = x.InnerText;
							#endregion

							msg.AddCulture(cultureName, subject, html, text);
						}
					}

					if (!string.IsNullOrEmpty(name) && msg != null)
						dictMessages.Add(name, msg);
				}
			}
		}
		#endregion

		#region Public Static Methods
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh() {
			RefreshMasters();
			RefreshMessages();
		}

		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages() {
			//Go through all email messages in the various subfolders and add them in...
			string dir = Path.GetDirectoryName((HttpContext.Current != null ? HttpContext.Current.Server.MapPath("~/" + FILE_EMAIL_TEMPLATES_MESSAGES) : new DirectoryInfo(Assembly.GetEntryAssembly().Location).FullName));
			RefreshMessages(new DirectoryInfo(dir).GetFiles(FILE_EMAIL_TEMPLATES_MESSAGES, SearchOption.AllDirectories));
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages(FileInfo[] Files) {
			if (Files == null)
				return;

			dictMessages.Clear();

			XmlDocument doc = null;
			for (int i = Files.Length - 1; i >= 0; --i) {
				doc = new XmlDocument();
				doc.Load(Files[i].FullName);
				loadDocumentMessages(doc, false);
			}
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages(Stream Stream) {
			if (Stream == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(Stream);
			loadDocumentMessages(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages(XmlReader XmlReader) {
			if (XmlReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(XmlReader);
			loadDocumentMessages(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages(TextReader TextReader) {
			if (TextReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(TextReader);
			loadDocumentMessages(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMessages(string FileName) {
			FileInfo fi = new FileInfo(FileName);
			//Debug.WriteLine(fi.FullName);
			if (!File.Exists(FileName))
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(FileName);
			loadDocumentMessages(doc);
		}

		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddMessages(Stream Stream) {
			if (Stream == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(Stream);
			loadDocumentMessages(doc, false);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddMessages(XmlReader XmlReader) {
			if (XmlReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(XmlReader);
			loadDocumentMessages(doc, false);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddMessages(TextReader TextReader) {
			if (TextReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(TextReader);
			loadDocumentMessages(doc, false);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddMessages(string FileName) {
			FileInfo fi = new FileInfo(FileName);
			//Debug.WriteLine(fi.FullName);
			if (!File.Exists(FileName))
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(FileName);
			loadDocumentMessages(doc, false);
		}

		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters() {
			//Go through all email master templates in the various subfolders and add them in...
			string dir = Path.GetDirectoryName((HttpContext.Current != null ? HttpContext.Current.Server.MapPath("~/" + FILE_EMAIL_TEMPLATES_MASTERS) : new DirectoryInfo(Assembly.GetEntryAssembly().Location).FullName));
			RefreshMasters(new DirectoryInfo(dir).GetFiles(FILE_EMAIL_TEMPLATES_MASTERS, SearchOption.AllDirectories));
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters(FileInfo[] Files) {
			if (Files == null)
				return;

			dictMasters.Clear();

			XmlDocument doc = null;
			for (int i = Files.Length - 1; i >= 0; --i) {
				doc = new XmlDocument();
				doc.Load(Files[i].FullName);
				loadDocumentMasters(doc, false);
			}
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters(Stream Stream) {
			if (Stream == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(Stream);
			loadDocumentMasters(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters(XmlReader XmlReader) {
			if (XmlReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(XmlReader);
			loadDocumentMasters(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters(TextReader TextReader) {
			if (TextReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(TextReader);
			loadDocumentMasters(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void RefreshMasters(string FileName) {
			FileInfo fi = new FileInfo(FileName);
			//Debug.WriteLine(fi.FullName);
			if (!File.Exists(FileName))
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(FileName);
			loadDocumentMasters(doc);
		}

		public static string StaticGetDefaultCultureName() {
			CultureInfo ci = null;
			Internal.I18NSettings i18n = null;

			if ((i18n = Settings.From<Internal.I18NSettings>(Settings.Section.I18N)) != null)
				if ((ci = CultureInfo.GetCultureInfo(i18n.DefaultCulture)) != null)
					return ci.Name;
			return Thread.CurrentThread.CurrentCulture.Name;
		}

		public static bool StaticContainsName(string Name) {
			if (dictMessages == null || dictMessages.Count <= 0 || !dictMessages.ContainsKey(Name))
				return false;
			return true;
		}

		public static string StaticGetMessageSubject(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			return dictMessages[Name].FindDefaultSubject();
		}

		public static string StaticGetMessageHTML(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			return dictMessages[Name].FindDefaultHTML();
		}

		public static string StaticGetMessageText(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			return dictMessages[Name].FindDefaultText();
		}

		public static string StaticGetMasterName(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			if (dictMessages[Name].Master == null)
				return string.Empty;
			return dictMessages[Name].Master.Name;
		}

		public static string StaticGetMasterHTML(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			if (dictMessages[Name].Master == null)
				return "{Content}";
			return dictMessages[Name].Master.FindDefaultHTML();
		}

		public static string StaticGetMasterText(string Name) {
			if (dictMessages == null || !dictMessages.ContainsKey(Name))
				return string.Empty;
			if (dictMessages[Name].Master == null)
				return "{Content}";
			return dictMessages[Name].Master.FindDefaultText();
		}

		public static void StaticPrepareMessage(string EmailMessageName, string BaseUrl, string[] Variables, string[] VariableValues, out string Subject, out string HTML, out string Text) {
			if (!loaded)
				Load();

			if (dictMessages == null || !dictMessages.ContainsKey(EmailMessageName))
				RefreshMessages();

			if (dictMessages == null || !dictMessages.ContainsKey(EmailMessageName))
				throw new ArgumentException("Unable to find template named '" + EmailMessageName + "'");

			Message m = dictMessages[EmailMessageName];
			if (m == null)
				throw new ArgumentNullException("Template is null at '" + EmailMessageName + "'");

			m.BuildMessage(BaseUrl, Variables, VariableValues, out Subject, out HTML, out Text);
		}

		///<summary></summary>
		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, Attachments, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, Attachments);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, Attachment[] Attachments) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, Attachments);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, MailPriority Priority, Attachment[] Attachments) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, Priority, Attachments);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, MailPriority Priority) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, Priority);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, MailPriority Priority, Attachment[] Attachments) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, Priority, Attachments);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, MailPriority Priority) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, Priority);
		}

		public static bool StaticSendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword, MailPriority Priority, Attachment[] Attachments) {
			string subject, html, text;
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out subject, out html, out text);
			return Email.Send(subject, html, text, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, Priority, Attachments);
		}
		#endregion

		#region Public Methods
		#region Send Message
		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, Attachments, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, Priority, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, MailPriority Priority) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, Priority);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, Priority, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, MailPriority Priority) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, Priority);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, string BaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, BaseUrl, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, Priority, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, Attachments, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, Priority, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, MailPriority Priority) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, Priority);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, Priority, Attachments);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, MailPriority Priority) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, Priority);
		}

		public bool SendMessage(string[] Variables, string[] VariableValues, string EmailMessageName, bool UseUnsecureBaseUrl, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword, MailPriority Priority, Attachment[] Attachments) {
			return StaticSendMessage(Variables, VariableValues, EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, Priority, Attachments);
		}
		#endregion

		public void PrepareMessage(string EmailMessageName, string BaseUrl, string[] Variables, string[] VariableValues, out string Subject, out string HTML, out string Text) {
			StaticPrepareMessage(EmailMessageName, BaseUrl, Variables, VariableValues, out Subject, out HTML, out Text);
		}

		public void PrepareMessage(string EmailMessageName, bool UseUnsecureBaseUrl, string[] Variables, string[] VariableValues, out string Subject, out string HTML, out string Text) {
			StaticPrepareMessage(EmailMessageName, findBaseUrl(UseUnsecureBaseUrl), Variables, VariableValues, out Subject, out HTML, out Text);
		}

		public bool ContainsName(string Name) {
			return StaticContainsName(Name);
		}

		public bool ContainsKey(string Name) {
			return StaticContainsName(Name);
		}

		public string GetMessageSubject(string Name) {
			return StaticGetMessageSubject(Name);
		}

		public string GetMessageHTML(string Name) {
			return StaticGetMessageHTML(Name);
		}

		public string GetMessageText(string Name) {
			return StaticGetMessageText(Name);
		}

		public string GetMasterName(string Name) {
			return StaticGetMasterName(Name);
		}

		public string GetMasterHTML(string Name) {
			return StaticGetMasterHTML(Name);
		}

		public string GetMasterText(string Name) {
			return StaticGetMasterText(Name);
		}

		public string GetDefaultCultureName() {
			return StaticGetDefaultCultureName();
		}
		#endregion

		#region Helper Classes
		private class Master {
			#region Variables
			public string Name;
			public Dictionary<string, MasterCulture> Cultures = new Dictionary<string, MasterCulture>();
			#endregion

			#region Constructors
			public Master(string Name) {
				this.Name = Name;
			}
			#endregion

			#region Public Methods
			public string FindDefaultHTML() {
				MasterCulture mc = FindCulture();
				if (mc != null)
					return mc.HTML;
				else
					return "{Content}";
			}

			public string FindDefaultText() {
				MasterCulture mc = FindCulture();
				if (mc != null)
					return mc.Text;
				else
					return "{Content}";
			}

			public MasterCulture FindCulture() {
				return FindCulture(Thread.CurrentThread.CurrentCulture);
			}

			public MasterCulture FindCulture(string CultureName) {
				if (!Cultures.ContainsKey(CultureName))
					return null;
				return Cultures[CultureName];
			}

			public MasterCulture FindCulture(CultureInfo ci) {
				MasterCulture mc = null;
				while (ci != null && !string.IsNullOrEmpty(ci.Name) && (mc = FindCulture(ci.Name)) == null)
					ci = ci.Parent;
				if (mc == null)
					mc = FindCulture(DEFAULT_NAME);
				return mc;
			}

			public bool CultureExists(string CultureName) {
				return Cultures.ContainsKey(CultureName);
			}

			public bool RemoveCulture(string CultureName) {
				if (Cultures.ContainsKey(CultureName))
					return true;
				return Cultures.Remove(CultureName);
			}

			public MasterCulture AddCulture(string CultureName, string HTML, string Text) {
				//Find out if we already have this culture defined and if so, return it.
				if (Cultures.ContainsKey(CultureName)) {
					MasterCulture mc = Cultures[CultureName];
					mc.HTML = HTML;
					mc.Text = Text;
					return mc;
				}

				//Create a new one and return that.
				MasterCulture ret = new MasterCulture(CultureName, HTML, Text);
				this.Cultures.Add(CultureName, ret);
				return ret;
			}

			public void AssignMaster(string BaseURL, string Subject, ref string HTML, ref string Text) {
				MasterCulture mc = FindCulture();
				if (mc == null)
					throw new ArgumentOutOfRangeException("Invalid culture");
				mc.AssignMaster(BaseURL, Subject, ref HTML, ref Text);
			}

			public void AssignMaster(CultureInfo CInfo, string BaseURL, string Subject, ref string HTML, ref string Text) {
				MasterCulture mc = FindCulture(CInfo);
				if (mc == null)
					throw new ArgumentOutOfRangeException("Invalid culture");
				mc.AssignMaster(BaseURL, Subject, ref HTML, ref Text);
			}
			#endregion
		}

		private class MasterCulture {
			#region Constants
			private const string
				VAR_BASEURL = "{BaseUrl}",
				VAR_SUBJECT = "{Subject}",
				VAR_CONTENT = "{Content}"
			;
			#endregion

			#region Variables
			public string CultureName;
			public string HTML;
			public string Text;
			#endregion

			#region Constructors
			public MasterCulture(string CultureName, string HTML, string Text) {
				this.CultureName = CultureName;
				this.HTML = HTML;
				this.Text = Text;
			}
			#endregion

			#region Helper Methods
			private string replaceVars(string BaseURL, string Subject, string Content, string Text) {
				return Text
					.Replace(VAR_BASEURL, BaseURL)
					.Replace(VAR_SUBJECT, Subject)
					.Replace(VAR_CONTENT, Content)
				;
			}
			#endregion

			#region Public Methods
			public void AssignMaster(string BaseURL, string Subject, ref string HTML, ref string Text) {
				if (!string.IsNullOrEmpty(HTML))
					HTML = this.replaceVars(BaseURL, Subject, HTML, this.HTML);
				if (!string.IsNullOrEmpty(Text))
					Text = this.replaceVars(BaseURL, Subject, Text, this.Text);
			}
			#endregion
		}

		private class Message {
			#region Variables
			public string name;
			public Master master;
			public Dictionary<string, MessageCulture> cultures = new Dictionary<string, MessageCulture>();
			#endregion

			#region Constructors
			public Message(Master Master, string Name) {
				this.name = Name;
				this.master = Master;
			}
			#endregion

			#region Properties
			public string Name { get { return this.name; } }
			public Master Master { get { return this.master; } }
			public Dictionary<string, MessageCulture> Cultures { get { return this.cultures; } }
			#endregion

			#region Public Methods
			public string FindDefaultSubject() {
				MessageCulture mc = FindCulture();
				if (mc != null)
					return mc.Subject;
				else
					return string.Empty;
			}

			public string FindDefaultHTML() {
				MessageCulture mc = FindCulture();
				if (mc != null)
					return mc.HTML;
				else
					return "{Content}";
			}

			public string FindDefaultText() {
				MessageCulture mc = FindCulture();
				if (mc != null)
					return mc.Text;
				else
					return "{Content}";
			}

			public MessageCulture FindCulture() {
				return FindCulture(Thread.CurrentThread.CurrentCulture);
			}

			public MessageCulture FindCulture(string CultureName) {
				if (!cultures.ContainsKey(CultureName))
					return null;
				return cultures[CultureName];
			}

			public MessageCulture FindCulture(CultureInfo ci) {
				MessageCulture mc = null;
				while (ci != null && !string.IsNullOrEmpty(ci.Name) && (mc = FindCulture(ci.Name)) == null)
					ci = ci.Parent;
				if (mc == null)
					mc = FindCulture(DEFAULT_NAME);
				return mc;
			}

			public bool CultureExists(string CultureName) {
				return cultures.ContainsKey(CultureName);
			}

			public bool RemoveCulture(string CultureName) {
				if (cultures.ContainsKey(CultureName))
					return true;
				return cultures.Remove(CultureName);
			}

			public MessageCulture AddCulture(string CultureName, string Subject, string HTML, string Text) {
				//Find out if we already have this culture defined and if so, return it.
				if (cultures.ContainsKey(CultureName)) {
					MessageCulture mc = cultures[CultureName];
					mc.subject = Subject;
					mc.html = HTML;
					mc.text = Text;
					return mc;
				}

				//Create a new one and return that.
				MessageCulture ret = new MessageCulture(CultureName, Subject, HTML, Text);
				this.cultures.Add(CultureName, ret);
				return ret;
			}

			///<summary>Cycles through the variables and inserts them into Content</summary>
			public string loadVars(string Content, string BaseUrl, string[] Vars, string[] Values) {
				MessageCulture mc = FindCulture();
				if (mc == null)
					throw new ArgumentOutOfRangeException("Invalid culture");

				return mc.loadVars(Content, BaseUrl, Vars, Values);
			}

			public void BuildMessage(string BaseUrl, string[] Variables, string[] VariableValues, out string NewSubject, out string NewHTML, out string NewText) {
				MessageCulture mc = FindCulture();
				if (mc == null)
					throw new ArgumentOutOfRangeException("Invalid culture");

				NewSubject = mc.loadVars(mc.Subject, BaseUrl, Variables, VariableValues);
				NewHTML = mc.loadVars(mc.HTML, BaseUrl, Variables, VariableValues);
				NewText = mc.loadVars(mc.Text, BaseUrl, Variables, VariableValues);

				if (this.Master != null)
					this.Master.AssignMaster(BaseUrl, mc.Subject, ref NewHTML, ref NewText);
			}
			#endregion
		}

		private class MessageCulture {
			#region Variables
			public string cultureName;
			public string subject;
			public string html;
			public string text;
			#endregion

			#region Constructors
			public MessageCulture(string CultureName, string Subject, string HTML, string Text) {
				this.cultureName	= CultureName;
				this.subject		= Subject;
				this.html			= HTML;
				this.text			= Text;
			}
			#endregion

			#region Properties
			public string CultureName { get { return this.cultureName; } }
			public string Subject { get { return this.subject; } }
			public string HTML { get { return this.html; } }
			public string Text { get { return this.text; } }
			#endregion

			#region Helper Methods
			///<summary>Cycles through the variables and inserts them into Content</summary>
			public string loadVars(string Content, string BaseUrl, string[] Vars, string[] Values) {
				if (Vars == null || Values == null || Vars.Length != Values.Length)
					throw new ArgumentException("Variables must each have an assigned value and cannot be null");
				
				int len = Vars.Length;
				for(int i = 0; i < len; i++)
					Content = Content.Replace(Vars[i], Values[i]);
				return Content.Replace("{BaseUrl}", BaseUrl);
			}
			#endregion
		}
		#endregion
	}
}
