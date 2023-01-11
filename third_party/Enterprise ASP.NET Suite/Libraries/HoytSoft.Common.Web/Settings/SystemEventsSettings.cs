#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;
using HoytSoft.Common;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class SystemEventsSettings : Settings.ISectionHandler {
		#region Constants
		public const string 
			SETTINGS_SECTION = "SystemEvents";
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.SystemEvents; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		#endregion

		#region IConfigurationSectionHandler
		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					foreach (XmlNode xn in xnl) {
						if (xn.Attributes["type"] != null && !string.IsNullOrEmpty(xn.Attributes["type"].Value)) {
							Type t = null;
							if ((t = Type.GetType(xn.Attributes["type"].Value, false, true)) != null) {
								Type[] tt = t.GetInterfaces();
								foreach (Type ti in tt) {
									if (ti.IsInterface && ti.FullName == typeof(ISystemEvent).FullName) {
										ISystemEvent obj = (ISystemEvent)Activator.CreateInstance(t);
										SystemEvents.AddObject(obj);
									}
								}
							}
						}
					}
				}
			}
			return this;
		}
		#endregion
	}
}
