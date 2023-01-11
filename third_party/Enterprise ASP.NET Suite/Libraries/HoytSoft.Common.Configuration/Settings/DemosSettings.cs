#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using System.Collections.Generic;

namespace HoytSoft.Common.Configuration.Internal {

	///<summary>Reads the settings in from the web.config file.</summary>
	public class DemosSettings : HoytSoft.Common.Settings.ISectionHandler {
		#region Constants
		private const string
			SETTINGS_SECTION_NAME = "Demos"
		;
		private const string
			SETTINGS_XPATH_PRESENTERS = @"Presenters/Presenter",
			SETTINGS_XPATH_PRESENTERS_TAG = @"Presenters"
		;
		private const string
			SETTINGS_ATTRIBS_PRESENTER_NAME = "name",
			SETTINGS_ATTRIBS_PRESENTER_LOGIN = "login",
			SETTINGS_ATTRIBS_PRESENTER_PASSWORD = "password",
			SETTINGS_ATTRIBS_PRESENTER_SESSIONID = "sessionID",
			SETTINGS_ATTRIBS_PRESENTERS_LOCKFOLDER = "lockFolder"
		;
		#endregion

		#region Variables
		private static DemosSettings src;
		private Dictionary<string, HoytSoft.Common.Configuration.Demos.User> users = new Dictionary<string, HoytSoft.Common.Configuration.Demos.User>(5);
		private string lockFolder;
		#endregion

		#region Constructors
		static DemosSettings() {
		}
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.Demos; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION_NAME; } }
		public string PresentersLockFolder { get { return this.lockFolder; } }
		public Dictionary<string, HoytSoft.Common.Configuration.Demos.User> Users { get { return this.users; } }
		#endregion

		#region Section Handler
		public object Create(object Parent, object Context, System.Xml.XmlNode Section) {
			DemosSettings ret = new DemosSettings();
			if (Section == null) return ret;
			XmlNodeList xnl = null;
			XmlNode xnTag = null;

			if ((xnTag = Section.SelectSingleNode(SETTINGS_XPATH_PRESENTERS_TAG)) != null) {
				if (xnTag.Attributes[SETTINGS_ATTRIBS_PRESENTERS_LOCKFOLDER] == null || string.IsNullOrEmpty(xnTag.Attributes[SETTINGS_ATTRIBS_PRESENTERS_LOCKFOLDER].Value))
					throw new XmlException("Attribute '" + SETTINGS_ATTRIBS_PRESENTERS_LOCKFOLDER + "' is missing from a presenter's tag in the web.config file.");
				ret.lockFolder = xnTag.Attributes[SETTINGS_ATTRIBS_PRESENTERS_LOCKFOLDER].Value;
			} else ret.lockFolder = "~/locks/";

			if ((xnl = Section.SelectNodes(SETTINGS_XPATH_PRESENTERS)) != null && xnl.Count > 0) {
				string name, login, password, sessionID;
				name = login = password = sessionID = null;

				foreach (XmlNode xn in xnl) {
					#region Check the tag's values
					if (xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_NAME] == null || string.IsNullOrEmpty(xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_NAME].Value))
						throw new XmlException("Attribute '" + SETTINGS_ATTRIBS_PRESENTER_NAME + "' is missing from a presenter's tag in the web.config file.");
					if (xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_LOGIN] == null || string.IsNullOrEmpty(xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_LOGIN].Value))
						throw new XmlException("Attribute '" + SETTINGS_ATTRIBS_PRESENTER_LOGIN + "' is missing from a presenter's tag in the web.config file.");
					if (xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_PASSWORD] == null || string.IsNullOrEmpty(xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_PASSWORD].Value))
						throw new XmlException("Attribute '" + SETTINGS_ATTRIBS_PRESENTER_PASSWORD + "' is missing from a presenter's tag in the web.config file.");
					if (xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_SESSIONID] == null || string.IsNullOrEmpty(xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_SESSIONID].Value))
						throw new XmlException("Attribute '" + SETTINGS_ATTRIBS_PRESENTER_SESSIONID + "' is missing from a presenter's tag in the web.config file.");

					name = xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_NAME].Value;
					login = xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_LOGIN].Value;
					password = xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_PASSWORD].Value;
					sessionID = xn.Attributes[SETTINGS_ATTRIBS_PRESENTER_SESSIONID].Value.Replace("-", "");
					#endregion

					ret.users.Add(login, new HoytSoft.Common.Configuration.Demos.User(name, login, password, sessionID));
				}
			}
			return ret;
		}
		#endregion
	}

}