#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Net.Sockets;
using System.Text;
using System.Collections;
using System.Diagnostics;
using System.Text.RegularExpressions;
using HoytSoft.Common.Net;

namespace HoytSoft.Common {
	///<summary>
	/// A great source of information regarding the SMTP protocol can be found at: 
	///		http://cr.yp.to/smtp.html
	/// A thank you for the idea of using the TcpClient class goes to Randy Charles Morin at: 
	///		http://www.csharphelp.com/archives2/archive449.html
	/// Kudos to Vasudevan Deepak Kumar for his article on e-mail address verification 
	/// (although I already knew how to do it beforehand):
	///		http://www.codeproject.com/aspnet/Valid_Email_Addresses.asp
	/// Another good SMTP class that I never used but thought was good anyway:
	///		http://www.eggheadcafe.com/articles/20030316.asp
	/// 
	/// POP3 Resources:
	///		http://www.kbcafe.com/articles/HowTo.POP3.CSharp.pdf
	///		http://www.codeproject.com/cs/internet/smtpauthlogin.asp
	///</summary>
	public class EmailVerification : TcpClient {
		#region Exceptions
		internal class VerificationException : Exception {
			public VerificationException(string Message) : base(Message) { }
		};
		#endregion

		#region Enums
		protected enum SMTPResponse : ushort {
			///<summary>211 System status, or system help reply</summary>
			SystemStatus								= 211,
			///<summary>214 Help message [Information on how to use the receiver or the meaning of a particular non-standard command; this reply is useful only to the human user]</summary>
			HelpMessage									= 214,
			///<summary>220 &lt;domain&gt; Service ready</summary>
			ServiceReady								= 220,
			///<summary>221 <domain> Service closing transmission channel</summary>
			ServiceClosingTransmissionChannel			= 221,
			///<summary>250 Requested mail action okay, completed</summary>
			RequestedActionCompleted					= 250,
			///<summary>251 User not local; will forward to &lt;forward-path&gt;</summary>
			UserNotLocalWillForward						= 251,
			///<summary>354 Start mail input; end with &lt;CRLF&gt;.&lt;CRLF&gt;</summary>
			StartMail									= 354,
			///<summary>421 <domain> Service not available, closing transmission channel [This may be a reply to any command if the service knows it must shut down]</summary>
			ServiceNotAvailable							= 421,
			///<summary>Requested mail action not taken: mailbox unavailable [E.g., mailbox busy]</summary>
			MailboxUnavailableOrBusy					= 450,
			///<summary>451 Requested action aborted: local error in processing</summary>
			RequestedActionAbortedLocalError			= 451,
			///<summary>452 Requested action not taken: insufficient system storage</summary>
			IRequestedActionAbortednsufficientStorage	= 452,
			///<summary>500 Syntax error, command unrecognized [This may include errors such as command line too long]</summary>
			SyntaxErrorUnrecognizedCommand				= 500,
			///<summary>501 Syntax error in parameters or arguments</summary>
			SyntaxErrorParameters						= 501,
			///<summary>502 Command not implemented</summary>
			UnrecognizedCommand							= 502,
			///<summary>503 Bad sequence of commands</summary>
			BadCommandSequence							= 503,
			///<summary>504 Command parameter not implemented</summary>
			UnimplementedCommandParameter				= 504,
			///<summary>550 Requested action not taken: mailbox unavailable [E.g., mailbox not found, no access]</summary>
			MailboxNotFound								= 550,
			///<summary>551 User not local; please try &lt;forward-path&gt;</summary>
			UserNotLocal								= 551,
			///<summary>552 Requested mail action aborted: exceeded storage allocation</summary>
			RequestedActionAbortedNoStorageAvailable	= 552,
			///<summary>553 Requested action not taken: mailbox name not allowed [E.g., mailbox syntax incorrect]</summary>
			RequestedActionAbortedMailboxNameUnknown	= 553,
			///<summary>554 Transaction failed</summary>
			TransactionFailed							= 554, 
			///<summary></summary>
			AuthChallenge								= 334,
			///<summary></summary>
			AuthSuccessful								= 235
		}
		#endregion

		#region Constants
		protected const int 
			DEFAULT_SMTP_PORT = 25
		;

		protected const string 
			SMTP_PROTOCOL_LINEENDING	= "\r\n", 
			SMTP_PROTOCOL_HELO			= "HELO", 
			SMTP_PROTOCOL_MAILFROM		= "MAIL FROM:", 
			SMTP_PROTOCOL_RCPTTO		= "RCPT TO:", 
			SMTP_PROTOCOL_AUTHLOGIN		= "AUTH LOGIN", 
			SMTP_PROTOCOL_QUIT			= "QUIT"
		;

		protected const string
			REG_EXP_RESPONSE_MSG		= @"\s*(?<Response>\d+)\s+(?<Message>.*)", 
			REG_EXP_GRP_RESPONSE		= @"Response", 
			REG_EXP_GRP_MESSAGE			= @"Message",

			REG_EXP_VALID_EMAIL			= @"(?<UserName>\w+([-+.']\w+)*)@(?<HostName>\w+([-.]\w+)*\.\w+([-.]\w+)*)", 
			REG_EXP_GRP_USERNAME		= @"UserName", 
			REG_EXP_GRP_HOSTNAME		= @"HostName"
		;
		#endregion

		#region Variables
		private static Regex regExpResponseMsg, regExpValidEmail;
		private System.Threading.AutoResetEvent mre = new System.Threading.AutoResetEvent(false);
		#endregion

		#region Constructors
		static EmailVerification() {
			regExpResponseMsg = new Regex(REG_EXP_RESPONSE_MSG, RegexOptions.IgnoreCase | RegexOptions.CultureInvariant | RegexOptions.Multiline | RegexOptions.ExplicitCapture | RegexOptions.Compiled);
			regExpValidEmail = new Regex(REG_EXP_VALID_EMAIL, RegexOptions.IgnoreCase | RegexOptions.CultureInvariant | RegexOptions.Multiline | RegexOptions.ExplicitCapture | RegexOptions.Compiled);
		}
		#endregion

		#region Helper Methods
		protected void Write(string Message) {
			byte[] buffer = new byte[this.SendBufferSize];
			buffer = Encoding.ASCII.GetBytes(Message);
			this.GetStream().Write(buffer, 0, buffer.Length);

			//if (Settings.DebugMode)
			//	Debug.WriteLine("Client: " + Message.Trim());
		}

		protected SMTPResponse Response() {
			byte[] serverbuff = new byte[this.ReceiveBufferSize];
			NetworkStream stream = GetStream();
			StringBuilder sb = new StringBuilder();
			
			//Read in msg from network...
			try {
				int count = this.ReceiveBufferSize;
				while (count == this.ReceiveBufferSize && (count = stream.Read(serverbuff, 0, serverbuff.Length)) != 0)
					sb.Append(Encoding.ASCII.GetString(serverbuff, 0, count));
			} catch (SocketException) {
				return SMTPResponse.ServiceNotAvailable;
			}

			string msg = sb.ToString();

			//if (Settings.DebugMode)
			//	Debug.WriteLine("Server: " + msg.Trim());

			Match m = regExpResponseMsg.Match(msg);
			if (m == null || !m.Success)
				return SMTPResponse.ServiceNotAvailable;

			string resp = string.Empty;

			if (m.Groups[REG_EXP_GRP_RESPONSE] != null && !string.IsNullOrEmpty((resp = m.Groups[REG_EXP_GRP_RESPONSE].Value.Trim()))) {
				string respMsg = string.Empty;
				if (m.Groups[REG_EXP_GRP_MESSAGE] != null)
					respMsg = m.Groups[REG_EXP_GRP_MESSAGE].Value.Trim();

				ushort respVal = 0;
				if (!ushort.TryParse(resp, out respVal))
					return SMTPResponse.ServiceNotAvailable;

				return (SMTPResponse)Enum.ToObject(typeof(SMTPResponse), respVal);
			}
			return SMTPResponse.ServiceNotAvailable;
		}

		protected bool Disconnect() {
			if (this.Connected) {
				try {
					this.Write(string.Format(SMTP_PROTOCOL_QUIT + "{0}", SMTP_PROTOCOL_LINEENDING));
					this.Response();
				} catch {
				}
				this.Client.Disconnect(true);
			}
			return false;
		}

		/*
		protected int CompareMXRecords(Utilities.DNS.MXRecord r1, Utilities.DNS.MXRecord r2) {
			if (r1.Preference < r2.Preference)
				return -1;
			else if (r1.Preference == r2.Preference)
				return 0;
			else
				return 1;
		} /**/
		#endregion

		#region Verification Methods
		protected bool ExtendedVerification(string To, string From) {
			return ExtendedVerification(DEFAULT_SMTP_PORT, To, From);
		}
		protected bool ExtendedVerification(int Port, string To, string From) {
			Match m = regExpValidEmail.Match(To);
			if (m == null || !m.Success)
				return false;

			if (m.Groups[REG_EXP_GRP_HOSTNAME] == null || m.Groups[REG_EXP_GRP_USERNAME] == null)
				return false;

			return ExtendedVerification(m.Groups[REG_EXP_GRP_HOSTNAME].Value, Port, null, null, To, From);
		}
		protected bool ExtendedVerification(string Domain, string To, string From) {
			return ExtendedVerification(Domain, DEFAULT_SMTP_PORT, null, null, To, From);
		}
		protected bool ExtendedVerification(string Domain, int Port, string To, string From) {
			return ExtendedVerification(Domain, Port, null, null, To, From);
		}
		protected bool ExtendedVerification(string Domain, string SMTPLogin, string SMTPPassword, string To, string From) {
			return ExtendedVerification(Domain, DEFAULT_SMTP_PORT, SMTPLogin, SMTPPassword, To, From);
		}

		protected bool ExtendedVerification(string Domain, int Port, string SMTPLogin, string SMTPPassword, string To, string From) {
			//Sort all the MX records based on preference starting at lowest and working up...
			string[] hosts = DNS.GetMXRecords(Domain);
			if (hosts != null && hosts.Length > 0) {
				foreach (string host in hosts)
					if (ExtendedVerificationImpl(host, Port, SMTPLogin, SMTPPassword, To, From))
						return true;
				return false;
			} else {
				return ExtendedVerificationImpl(Domain, Port, SMTPLogin, SMTPPassword, To, From);
			}
		}

		private void ConnectStarted(IAsyncResult ar) {
			if (ar.IsCompleted) {
				this.mre.Set();
				try {
					this.EndConnect(ar);
				} catch {
				}
			}
		}

		protected bool ExtendedVerificationImpl(string Domain, int Port, string Login, string Password, string To, string From) {
			bool valid = false;

			if (this.Client == null)
				return false;

			this.ReceiveBufferSize = 1024;
			this.SendBufferSize = 1024;
			this.SendTimeout = 5000;
			this.ReceiveTimeout = 5000;
			//Disconnect...
			if (this.Connected)
				this.Disconnect();

			SMTPResponse response = SMTPResponse.ServiceNotAvailable;

			try {
				//Create a connection to the SMTP server...
				mre.Reset();
				this.BeginConnect(Domain, Port, new AsyncCallback(this.ConnectStarted), this.Client);
				if (!mre.WaitOne(5000, false) && !this.Connected) {
					return this.Disconnect();
				}

				if (!this.Connected)
					return this.Disconnect();
				
				//System.Net.Security.SslStream s;s.

				//Find out what the server said...
				if ((response = this.Response()) != SMTPResponse.ServiceReady)
					return this.Disconnect();

				//HELO hoytsoft.org\r\n
				this.Write(string.Format(SMTP_PROTOCOL_HELO + " {0}{1}", Domain, SMTP_PROTOCOL_LINEENDING));
				if ((response = this.Response()) != SMTPResponse.RequestedActionCompleted)
					return this.Disconnect();

				if (Login != null && Password != null) {
					//SmtpServer.SendData("auth login\r\n");
					//AUTH LOGIN\r\n
					this.Write(string.Format(SMTP_PROTOCOL_AUTHLOGIN + "{0}", SMTP_PROTOCOL_LINEENDING));
					if ((response = this.Response()) == SMTPResponse.AuthChallenge) {
						//[LOGIN], base64 encoded
						this.Write(Convert.ToBase64String(Encoding.ASCII.GetBytes(Login)) + SMTP_PROTOCOL_LINEENDING);
						if ((response = this.Response()) != SMTPResponse.AuthChallenge)
							return this.Disconnect();
						//[PASSWORD], base64 encoded
						this.Write(Convert.ToBase64String(Encoding.ASCII.GetBytes(Password)) + SMTP_PROTOCOL_LINEENDING);
						if ((response = this.Response()) != SMTPResponse.AuthSuccessful)
							return this.Disconnect();
					}
				}

				//MAIL FROM:<noreply@mydomain.com>\r\n
				this.Write(string.Format(SMTP_PROTOCOL_MAILFROM + "<{0}>{1}", From, SMTP_PROTOCOL_LINEENDING));
				if ((response = this.Response()) != SMTPResponse.RequestedActionCompleted)
					return this.Disconnect();

				//RCPT TO:<david@hoytsoft.org>\r\n
				this.Write(string.Format(SMTP_PROTOCOL_RCPTTO + "<{0}>{1}", To, SMTP_PROTOCOL_LINEENDING));
				if ((response = this.Response()) != SMTPResponse.RequestedActionCompleted && response != SMTPResponse.UserNotLocalWillForward)
					return this.Disconnect();

				//If, by this point, we were able to successfully specify who we want to send the message to, 
				//then we can hang up because the server has told us that he knows who the person is, 
				//or at least where to forward it on to.
				//
				//Note that we do not send a DATA section because we don't want to actually send a message, 
				//just verify that the e-mail address is correct and recognized.
				valid = true;
			} catch (Exception e) {
				//if (Settings.DebugMode)
				//	Debug.WriteLine(e);
				throw e;
			} finally {
				this.Disconnect();
			}
			return valid;
		}
		#endregion

		#region Public Static Methods
		public static bool Verify(string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(To, From);
		}
		public static bool Verify(int Port, string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(Port, To, From);
		}
		public static bool Verify(string Domain, string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(Domain, To, From);
		}
		public static bool Verify(string Domain, int Port, string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(Domain, Port, To, From);
		}
		public static bool Verify(string Domain, string SMTPLogin, string SMTPPassword, string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(Domain, SMTPLogin, SMTPPassword, To, From);
		}
		public static bool Verify(string Domain, int Port, string SMTPLogin, string SMTPPassword, string To, string From) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerification(Domain, Port, SMTPLogin, SMTPPassword, To, From);
		}
		#endregion
	}
}
