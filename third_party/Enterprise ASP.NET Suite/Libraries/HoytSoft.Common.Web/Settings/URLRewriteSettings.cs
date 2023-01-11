#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Text.RegularExpressions;
using System.Web;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class URLRewriteSettings : HoytSoft.Common.Settings.ISectionHandler {
		#region Constants
		public const string
			SETTINGS_SECTION				= "URLRewrite",
			ATTRIB_ENABLED					= "enabled",
			ATTRIB_FORMS_AUTHENTICATION		= "formsAuthentication", 
			ATTRIB_USE_HTTP_HANDLER_FACTORY = "useHttpHandlerFactory", 

			XML_REGULAR_EXPRESSIONS			= "RegularExpressions/Rule", 
			XML_REGULAR_EXPRESSIONS_FROM	= "From", 
			XML_REGULAR_EXPRESSIONS_TO		= "To"
		;
		#endregion

		#region Variables
		private bool enabled = false;
		private bool formsAuthentication = false;
		private bool useHttpHandlerFactory = false;
		private URLRewriteRegularExpressionRule[] regularExpressionRules = null;
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.URLRewrite; } }
		public string ConfigSectionName		{ get { return SETTINGS_SECTION; } }

		public bool Enabled					{ get { return this.enabled; } }
		public bool FormsAuthentication		{ get { return this.formsAuthentication; } }
		public bool UseHttpHandlerFactory	{ get { return this.useHttpHandlerFactory; } }

		public URLRewriteRegularExpressionRule[] RegularExpressionRules {
			get { return this.regularExpressionRules; }
		}
		#endregion

		public object Create(object Parent, object Context, XmlNode Section) {
			if (HttpContext.Current == null)
				return null;

			string testEnabled, testFormsAuth, testUseHttpHandlerFactory;
			bool bEnabled, bFormsAuth, bUseHttpHandlerFactory;

			if (Section.Attributes[ATTRIB_ENABLED] != null && !string.IsNullOrEmpty((testEnabled = Section.Attributes[ATTRIB_ENABLED].Value)) && bool.TryParse(testEnabled.ToLower(), out bEnabled))
				this.enabled = bEnabled;

			if (Section.Attributes[ATTRIB_FORMS_AUTHENTICATION] != null && !string.IsNullOrEmpty((testFormsAuth = Section.Attributes[ATTRIB_FORMS_AUTHENTICATION].Value)) && bool.TryParse(testFormsAuth.ToLower(), out bFormsAuth))
				this.formsAuthentication = bFormsAuth;

			if (Section.Attributes[ATTRIB_USE_HTTP_HANDLER_FACTORY] != null && !string.IsNullOrEmpty((testUseHttpHandlerFactory = Section.Attributes[ATTRIB_USE_HTTP_HANDLER_FACTORY].Value)) && bool.TryParse(testUseHttpHandlerFactory.ToLower(), out bUseHttpHandlerFactory))
				this.useHttpHandlerFactory = bUseHttpHandlerFactory;

			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;

				//Process regular expression rules...
				if ((xnl = Section.SelectNodes(XML_REGULAR_EXPRESSIONS)) != null && xnl.Count > 0) {
					//Parse out the regular expressions...
					string from, to;
					XmlNode xnRule = null;
					RegexOptions RegExpOptions = RegexOptions.Compiled | RegexOptions.ECMAScript | RegexOptions.IgnoreCase;
					List<URLRewriteRegularExpressionRule> lst = new List<URLRewriteRegularExpressionRule>();

					foreach (XmlNode xn in xnl) {
						from = string.Empty;
						to = string.Empty;

						if ((xnRule = xn.SelectSingleNode(XML_REGULAR_EXPRESSIONS_FROM)) != null)
							from = "^" + URLRewriteUtils.ResolveUrl(HttpContext.Current.Request.ApplicationPath, xnRule.InnerText) + "$";
						if ((xnRule = xn.SelectSingleNode(XML_REGULAR_EXPRESSIONS_TO)) != null)
							to = xnRule.InnerText;

						if (string.IsNullOrEmpty(from) || string.IsNullOrEmpty(to))
							continue;

						lst.Add(new URLRewriteRegularExpressionRule(from, to, new Regex(from, RegExpOptions)));
					}

					this.regularExpressionRules = lst.ToArray();
				}
			}
			return this;
		}

		#region Helper Classes
		#endregion
	}
}
