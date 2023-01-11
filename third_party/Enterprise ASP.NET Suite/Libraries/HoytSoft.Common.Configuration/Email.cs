#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Net;
using System.Text;
using System.Net.Mime;
using System.Net.Mail;

namespace HoytSoft.Common.Configuration {
	public class Email {
		#region Overloaded Methods
		public static bool Send(string Subject, string TextBody, string To, MailPriority Priority) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, null, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, null);
			else
				return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string TextBody, string To, MailPriority Priority, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, null, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, Attachments);
			else
				return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, Priority, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, MailPriority Priority) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, null);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, MailPriority Priority, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, Attachments);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, Priority, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, MailPriority Priority) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, null);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, MailPriority Priority, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, Priority, Attachments);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, Priority, Attachments);
		}

		public static bool Send(string Subject, string TextBody, string To) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, null, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, null);
			else
				return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string TextBody, string To, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, null, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, Attachments);
			else
				return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, null);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, settings.From, settings.FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, Attachments);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, null);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, Attachment[] Attachments) {
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, settings.Server, settings.Port, settings.Login, settings.Password, MailPriority.Normal, Attachments);
			else
				return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, MailPriority.Normal, Attachments);
		}

		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, MailPriority.Normal, null);
		}

		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, Attachment[] Attachments, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword) {
			return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, MailPriority.Normal, Attachments);
		}
		#endregion

		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, string SMTPServer, int SMTPPort, string SMTPLogin, string SMTPPassword, MailPriority Priority, Attachment[] Attachments) {
			try {
				Common.Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, SMTPServer, SMTPPort, SMTPLogin, SMTPPassword, Priority, Attachments);
			} catch (System.Net.Mail.SmtpException) {
				if (Settings.DebugMode)
					throw;
				return false;
			}
			return true;
		}
	}
}
