#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using HoytSoft.Common.Web;

namespace HoytSoft.Common.Configuration.Internal {
	public class AuthenticationSettings : Settings.ISectionHandler {
		#region Constants
		public const string
			SETTINGS_SECTION = "Authentication"
		;
		#endregion

		#region Variables
		private string callbackTypeName;
		private bool enabled;
		private Type callbackType;
		private IAuthenticationCallback callback;
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.Authentication; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }

		public bool Enabled { get { return this.enabled; } }
		public string CallbackTypeName { get { return this.callbackTypeName; } }
		public Type CallbackType { get { return this.callbackType; } }
		public IAuthenticationCallback Callback { get { return this.callback; } }
		#endregion

		public object Create(object Parent, object Context, XmlNode Section) {
			string sEnabled, sCallbackTypeName;

			if (Section.Attributes["enabled"] != null && !string.IsNullOrEmpty((sEnabled = Section.Attributes["enabled"].Value)))
				this.enabled = bool.Parse(sEnabled);

			if (Section.Attributes["callbackType"] != null && !string.IsNullOrEmpty((sCallbackTypeName = Section.Attributes["callbackType"].Value)))
				this.callbackTypeName = sCallbackTypeName;

			this.callbackType = Type.GetType(callbackTypeName, true, true);
			if (!(typeof(IAuthenticationCallback).IsAssignableFrom(this.callbackType)))
				throw new TypeLoadException(this.callbackTypeName + " does not implement interface " + typeof(IAuthenticationCallback).FullName);

			this.callback = (IAuthenticationCallback)Activator.CreateInstance(this.callbackType);

			return this;
		} //create

	} //class
} //namespace
