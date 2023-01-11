using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HoytSoft.Common.Configuration {
	public class EmailVerification : Common.EmailVerification {
		#region Public Static Methods
		public static bool VerifyUsingConfiguration(string To) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerificationUsingConfiguration(To);
		}
		public static bool VerifyUsingConfiguration(int Port, string To) {
			using (EmailVerification ev = new EmailVerification())
				return ev.ExtendedVerificationUsingConfiguration(Port, To);
		}
		#endregion

		protected bool ExtendedVerificationUsingConfiguration(string To) {
			return ExtendedVerificationUsingConfiguration(Common.EmailVerification.DEFAULT_SMTP_PORT, To);
		}
		protected bool ExtendedVerificationUsingConfiguration(int Port, string To) {
			string from = "noreply@qqqqqqq.com";
			Internal.EmailSettings settings = Settings.From<Internal.EmailSettings>(Settings.Section.Email);
			if (settings != null)
				from = settings.From;
			return ExtendedVerification(Port, To, from);
		}
	}
}
