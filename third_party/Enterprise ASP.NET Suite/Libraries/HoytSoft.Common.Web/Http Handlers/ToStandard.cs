#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web.HttpHandlers {
	///<summary>Transfers you to an unsecure connection.</summary>
	public class ToStandard : IHttpHandler {
		public void ProcessRequest(HttpContext Context) {
			HttpResponse Response = Context.Response;
			HttpRequest Request = Context.Request;

			if (!string.IsNullOrEmpty(Request["url"])) {
				HttpSecuritySettings httpS = Settings.From<HttpSecuritySettings>(Settings.Section.HttpSecurity);

				if (httpS != null) {
					string url = Request["url"].Trim();
					string includeQuery = Request["includeQuery"];
					if (includeQuery != null)
						includeQuery = includeQuery.Trim();
					url = httpS.FindPathToUnsecureSite(url, false);

					if (!string.IsNullOrEmpty(url)) {
						Uri uri = new Uri(url);

						if (!string.IsNullOrEmpty(includeQuery)) {
							bool include = false;
							if (bool.TryParse(includeQuery, out include) && include) {
								Response.Redirect(url, true);
							} else {
								Response.Redirect(uri.GetLeftPart(UriPartial.Path), true);
							}
						} else {
							Response.Redirect(uri.AbsoluteUri, true);
						}
					}
				}
			}
		}

		public bool IsReusable {
			get { return true; }
		}
	}
}
