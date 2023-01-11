#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Collections.Generic;
using System.Web.Security;
using System.Diagnostics;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	#region Public Enums
	public enum CustomErrorsMode : byte {
		RemoteOnly = 0,
		On = 1,
		Off = 2
	}
	#endregion

	internal class CustomErrorsListener : AbstractApplicationListener {
		#region Constants
		private const string
			EVENT_LOG_WEBSITES = "Websites",
			EVENT_LOG_APPLICATION = "Application";
		#endregion

		#region Variables
		private CustomErrorsSettings ceSettings = null;
		#endregion

		#region Constructors
		static CustomErrorsListener() {
		}
		#endregion

		public override void AppStart() {
			this.ceSettings = Settings.From<CustomErrorsSettings>(Settings.Section.CustomErrors);
		}

		public override void AppError() {
			//Just process as normal if custom errors are off...
			if (this.ceSettings == null || this.ceSettings.Mode == CustomErrorsMode.Off)
				return;

			//Is this a remote or local call?
			if (this.ceSettings.Mode == CustomErrorsMode.RemoteOnly && !HttpContext.Current.Request.IsLocal)
				return;

			
			if (HttpContext.Current != null) {
				Exception e = HttpContext.Current.Server.GetLastError();
				int statusCode = HttpContext.Current.Response.StatusCode;
				string redir = this.ceSettings.DefaultRedirect;

				if (e != null && !(e is HttpUnhandledException) && e is HttpException) {
					if (e.Message.EndsWith("does not exist.", StringComparison.InvariantCultureIgnoreCase))
						statusCode = 404;
					//else if 
				}

				//Find the redirection page for this status code...
				if (this.ceSettings.StatusCodeRedirects != null && this.ceSettings.StatusCodeRedirects.ContainsKey(statusCode))
					redir = this.ceSettings.StatusCodeRedirects[statusCode];

				//Log the error if we're setup to do that...
				if ((this.ceSettings.UseEventLog || this.ceSettings.UseErrorEmail) && e != null && e is HttpUnhandledException && e.InnerException != null) {
					
					System.Text.StringBuilder sb = new System.Text.StringBuilder(
						"Request: " + 
						HttpContext.Current.Request.RawUrl + 
						"\r\n\r\n" + 
						e.InnerException.ToString() +
						"\r\n"
					);

					if (HttpContext.Current.Request.QueryString.Count > 0) {
						sb.AppendLine();
						sb.AppendLine("QUERYSTRING VARIABLES");
						sb.AppendLine("----------------------------------");
						for (int i = 0; i < HttpContext.Current.Request.QueryString.Count; i++)
							sb.AppendLine(HttpContext.Current.Request.QueryString.GetKey(i) + ": " + HttpContext.Current.Request.QueryString.Get(i));
					}

					if (HttpContext.Current.Request.Form.Count > 0) {
						sb.AppendLine();
						sb.AppendLine("FORM VARIABLES");
						sb.AppendLine("----------------------------------");
						for (int i = 0; i < HttpContext.Current.Request.Form.Count; i++)
							sb.AppendLine(HttpContext.Current.Request.Form.GetKey(i) + ": " + HttpContext.Current.Request.Form.Get(i));
					}

					if (HttpContext.Current.Request.Cookies.Count > 0) {
						sb.AppendLine();
						sb.AppendLine("COOKIES");
						sb.AppendLine("----------------------------------");
						for (int i = 0; i < HttpContext.Current.Request.Cookies.Count; i++)
							sb.AppendLine(HttpContext.Current.Request.Cookies.GetKey(i) + ": " + HttpContext.Current.Request.Cookies.Get(i).Value);
					}

					string msg = sb.ToString();
					
					if (this.ceSettings.UseEventLog) {
						if (!EventLog.SourceExists(this.ceSettings.EventLogSource))
							EventLog.CreateEventSource(this.ceSettings.EventLogSource, EVENT_LOG_APPLICATION);
						EventLog log = new EventLog(EVENT_LOG_APPLICATION, ".", this.ceSettings.EventLogSource);
						log.WriteEntry(msg, EventLogEntryType.Error);
					}

					if (this.ceSettings.UseErrorEmail) {
						Utils.SendSystemErrorMessage(this.ceSettings.ErrorEmailAddress, "Website error", msg);
					}
				} 

				//If we haven't defined a place to redirect to, then let execution continue as normal...
				if (string.IsNullOrEmpty(redir))
					return;

				HttpContext.Current.Response.Clear();
				try {
					HttpContext.Current.Response.StatusCode = statusCode;
				} catch {
				}

				//Add in the error path...
				if (HttpContext.Current.Request.QueryString["aspxerrorpath"] == null) {
					if (redir.IndexOf('?') < 0) redir += "?";
					else redir += "&";
					redir += "aspxerrorpath=" + HttpUtility.UrlEncode(HttpContext.Current.Request.Path);
				}

				//Do a transfer - hides the error page from the user...
				HttpContext.Current.Server.Transfer(redir, true);
				HttpContext.Current.Server.ClearError();
				return;
			}
		}

		#region Helper Classes
		#endregion
	}
}
