#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2006, 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.IO;
using System.Collections.Generic;
using System.Web;
using System.Diagnostics;
using System.Web.Configuration;
using System.Web.UI;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	///<summary>See: http://msdn2.microsoft.com/en-us/library/ms972974.aspx </summary>
	internal class URLRewriteListener : AbstractApplicationListener, IHttpHandlerFactory {
		#region Variables
		private URLRewriteSettings settings = null;
		#endregion

		#region ApplicationObject Methods
		public override void AppStart() {
			base.AppStart();
			this.settings = Settings.From<URLRewriteSettings>(Settings.Section.URLRewrite);
			//Utils.RegisterHttpHandler("*.123", "HoytSoft.Common.URLRewrite, HoytSoft.Common", "*", false);
		}

		///<summary>
		///	So, when should URL rewriting be performed in an HTTP module? It depends on what type of 
		/// authentication you're employing. If you're not using any authentication, then it doesn't 
		/// matter if URL rewriting happens in BeginRequest, AuthenticateRequest, or 
		/// AuthorizeRequest. If you are using forms authentication and are not using Windows 
		/// authentication, place the URL rewriting in the AuthorizeRequest event handler. Finally, 
		/// if you are using Windows authentication, schedule the URL rewriting during the 
		/// BeginRequest or AuthenticateRequest events. 
		///</summary>
		public override void AppBeginRequest() {
			base.AppBeginRequest();
			if (this.settings == null || HttpContext.Current == null || !this.settings.Enabled || this.settings.UseHttpHandlerFactory || this.settings.FormsAuthentication)
				return;

			rewrite(HttpContext.Current, HttpContext.Current.Request.Path);
			////System.Diagnostics.Debug.WriteLine(HttpContext.Current.Request.RawUrl);
			//string url = null, path = null;
			//int index = 0;
			//if ((url = this.vdSettings.Find(HttpContext.Current.Request.Path, ref index, ref path)) == null || path == null)
			//    return;

			//string modRequestFile = path + HttpContext.Current.Request.Path.Substring(index + url.Length).Replace('/', System.IO.Path.DirectorySeparatorChar);

			//try {
			//    Utils.SendFile(modRequestFile, HttpContext.Current.Response);
			//} catch (FileNotFoundException) {
			//    HttpContext.Current.Response.End();
			//}
		}

		///<summary>
		///	So, when should URL rewriting be performed in an HTTP module? It depends on what type of 
		/// authentication you're employing. If you're not using any authentication, then it doesn't 
		/// matter if URL rewriting happens in BeginRequest, AuthenticateRequest, or 
		/// AuthorizeRequest. If you are using forms authentication and are not using Windows 
		/// authentication, place the URL rewriting in the AuthorizeRequest event handler. Finally, 
		/// if you are using Windows authentication, schedule the URL rewriting during the 
		/// BeginRequest or AuthenticateRequest events. 
		///</summary>
		public override void AppAuthorizeRequest() {
			base.AppAuthorizeRequest();
			if (this.settings == null || HttpContext.Current == null || !this.settings.Enabled || this.settings.UseHttpHandlerFactory || !this.settings.FormsAuthentication)
				return;

			rewrite(HttpContext.Current, HttpContext.Current.Request.Path);
		}
		#endregion

		#region IHttpHandlerFactory Members
		public IHttpHandler GetHandler(HttpContext Context, string Verb, string URL, string PathTranslated) {
			return rewriteWithHandler(Context, Verb, URL, PathTranslated);
		}

		public void ReleaseHandler(IHttpHandler Handler) {
		}
		#endregion

		private void rewrite(HttpContext Context, string Path) {
			URLRewriteRegularExpressionRule[] rules = settings.RegularExpressionRules;
			if (rules == null || rules.Length <= 0)
				return;

			for (int i = 0; i < rules.Length; i++) {
				if (rules[i].RegularExpression.IsMatch(Path)) {
					string sendToUrl = URLRewriteUtils.ResolveUrl(Context.Request.ApplicationPath, rules[i].RegularExpression.Replace(Path, rules[i].To));
					URLRewriteUtils.RewriteUrl(Context, sendToUrl);
					break;
				}
			}
		}

		private IHttpHandler rewriteWithHandler(HttpContext Context, string Verb, string URL, string PathTranslated) {
			URLRewriteSettings mySettings = Settings.From<URLRewriteSettings>(Settings.Section.URLRewrite);
			if (mySettings == null || !mySettings.Enabled || !mySettings.UseHttpHandlerFactory)
				return PageParser.GetCompiledPageInstance(URL, PathTranslated, Context);

			URLRewriteRegularExpressionRule[] rules = mySettings.RegularExpressionRules;
			if (rules == null || rules.Length <= 0)
				return PageParser.GetCompiledPageInstance(URL, PathTranslated, Context);

			for (int i = 0; i < rules.Length; i++) {
				if (rules[i].RegularExpression.IsMatch(URL)) {
					string sendToUrl = URLRewriteUtils.ResolveUrl(Context.Request.ApplicationPath, rules[i].RegularExpression.Replace(URL, rules[i].To));
					string sendToUrlLessQString;
					URLRewriteUtils.RewriteUrl(Context, sendToUrl, out sendToUrlLessQString, out PathTranslated);
					return PageParser.GetCompiledPageInstance(sendToUrlLessQString, PathTranslated, Context);
				}
			}

			return PageParser.GetCompiledPageInstance(URL, PathTranslated, Context);
		}

		#region Helper Classes
		private class RequestFile {
			public byte[] Data;
			public string ContentType;
			public string FileName;

			public RequestFile(string FileName, string ContentType, byte[] Data) {
				this.FileName = FileName;
				this.ContentType = ContentType;
				this.Data = Data;
			}
		}
		#endregion
	}
}
