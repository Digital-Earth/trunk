#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Web;

namespace HoytSoft.Common.Web {
	///<summary>Courtesy http://msdn2.microsoft.com/en-us/library/ms972974.aspx </summary>
	internal class URLRewriteUtils {
		#region RewriteUrl
		/// <summary>
		/// Rewrite's a URL using <b>HttpContext.RewriteUrl()</b>.
		/// </summary>
		/// <param name="context">The HttpContext object to rewrite the URL to.</param>
		/// <param name="sendToUrl">The URL to rewrite to.</param>
		internal static void RewriteUrl(HttpContext context, string sendToUrl) {
			string x, y;
			RewriteUrl(context, sendToUrl, out x, out y);
		}

		/// <summary>
		/// Rewrite's a URL using <b>HttpContext.RewriteUrl()</b>.
		/// </summary>
		/// <param name="context">The HttpContext object to rewrite the URL to.</param>
		/// <param name="sendToUrl">The URL to rewrite to.</param>
		/// <param name="sendToUrlLessQString">Returns the value of sendToUrl stripped of the querystring.</param>
		/// <param name="filePath">Returns the physical file path to the requested page.</param>
		internal static void RewriteUrl(HttpContext context, string sendToUrl, out string sendToUrlLessQString, out string filePath) {
			// see if we need to add any extra querystring information
			if (context.Request.QueryString.Count > 0) {
				if (sendToUrl.IndexOf('?') != -1)
					sendToUrl += "&" + context.Request.QueryString.ToString();
				else
					sendToUrl += "?" + context.Request.QueryString.ToString();
			}

			// first strip the querystring, if any
			string queryString = String.Empty;
			sendToUrlLessQString = sendToUrl;
			if (sendToUrl.IndexOf('?') > 0) {
				sendToUrlLessQString = sendToUrl.Substring(0, sendToUrl.IndexOf('?'));
				queryString = sendToUrl.Substring(sendToUrl.IndexOf('?') + 1);
			}

			// grab the file's physical path
			filePath = string.Empty;
			filePath = context.Server.MapPath(sendToUrlLessQString);

			// rewrite the path...
			context.RewritePath(sendToUrlLessQString, String.Empty, queryString);

			// NOTE!  The above RewritePath() overload is only supported in the .NET Framework 1.1
			// If you are using .NET Framework 1.0, use the below form instead:
			// context.RewritePath(sendToUrl);
		}
		#endregion

		/// <summary>
		/// Converts a URL into one that is usable on the requesting client.
		/// </summary>
		/// <remarks>Converts ~ to the requesting application path.  Mimics the behavior of the 
		/// <b>Control.ResolveUrl()</b> method, which is often used by control developers.</remarks>
		/// <param name="appPath">The application path.</param>
		/// <param name="url">The URL, which might contain ~.</param>
		/// <returns>A resolved URL.  If the input parameter <b>url</b> contains ~, it is replaced with the
		/// value of the <b>appPath</b> parameter.</returns>
		internal static string ResolveUrl(string appPath, string url) {
			if (url.Length == 0 || url[0] != '~')
				return url;		// there is no ~ in the first character position, just return the url
			else {
				if (url.Length == 1)
					return appPath;  // there is just the ~ in the URL, return the appPath
				if (url[1] == '/' || url[1] == '\\') {
					// url looks like ~/ or ~\
					if (appPath.Length > 1)
						return appPath + "/" + url.Substring(2);
					else
						return "/" + url.Substring(2);
				} else {
					// url looks like ~something
					if (appPath.Length > 1)
						return appPath + "/" + url.Substring(1);
					else
						return appPath + url.Substring(1);
				}
			}
		}
	}
}
