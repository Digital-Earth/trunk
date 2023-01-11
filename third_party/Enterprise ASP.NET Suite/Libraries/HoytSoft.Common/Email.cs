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

namespace HoytSoft.Common {
	public class Email {
		#region Overloaded Methods
		public static bool Send(string Subject, string TextBody, string To, MailPriority Priority) {
			return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string TextBody, string To, MailPriority Priority, Attachment[] Attachments) {
			return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, Priority, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, MailPriority Priority) {
			return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, MailPriority Priority, Attachment[] Attachments) {
			return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, Priority, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, MailPriority Priority) {
			return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, Priority, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, MailPriority Priority, Attachment[] Attachments) {
			return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, Priority, Attachments);
		}
		public static bool Send(string Subject, string TextBody, string To) {
			return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string TextBody, string To, Attachment[] Attachments) {
			return Email.Send(Subject, null, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To) {
			return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, Attachment[] Attachments) {
			return Email.Send(Subject, HTMLBody, TextBody, To, To, To, null, 0, null, null, MailPriority.Normal, Attachments);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName) {
			return Email.Send(Subject, HTMLBody, TextBody, To, From, FromDisplayName, null, 0, null, null, MailPriority.Normal, null);
		}
		public static bool Send(string Subject, string HTMLBody, string TextBody, string To, string From, string FromDisplayName, Attachment[] Attachments) {
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
			SmtpClient smtpClient = new SmtpClient();
			MailMessage message = new MailMessage();

			try {
				//You can specify the host name or ipaddress of your server
				//Default in IIS will be localhost 
				if (!string.IsNullOrEmpty(SMTPServer))
					smtpClient.Host = SMTPServer;

				//Default port will be 25
				if (SMTPPort > 0)
					smtpClient.Port = SMTPPort;

				if (!string.IsNullOrEmpty(SMTPLogin) && !string.IsNullOrEmpty(SMTPPassword))
					smtpClient.Credentials = new System.Net.NetworkCredential(SMTPLogin, SMTPPassword);
				else
					smtpClient.Credentials = CredentialCache.DefaultNetworkCredentials;


				//From address will be given as a MailAddress Object
				message.From = new MailAddress(From, FromDisplayName);
				message.Subject = Subject;
				message.Priority = Priority;

				//Body can be Html or text format
				//Use text by default - if e-mail viewer supports HTML, it will pick it...
				message.IsBodyHtml = false;

				if (message.AlternateViews.Count > 0) 
					message.AlternateViews.Clear();

				message.To.Add(To);

				if (!string.IsNullOrEmpty(HTMLBody))
					message.AlternateViews.Add(AlternateView.CreateAlternateViewFromString(HTMLBody, Encoding.UTF8, MediaTypeNames.Text.Html));

				if (Attachments != null)
					foreach (Attachment a in Attachments)
						message.Attachments.Add(a);
				
				message.Body = TextBody;
				smtpClient.Send(message);
			} catch (System.Net.Mail.SmtpException) {
				throw;
			}
			return true;
		}
	}
}
