#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointOptionsCollection : System.Collections.Specialized.NameValueCollection {
		#region Constants
		private const string
			XML_TAG_OPTIONS = "options",
			XML_TAG_OPTION	= "option",
			XML_TAG_NAME	= "name", 
			XML_TAG_VALUE	= "value" 
		;
		#endregion

		#region Helper Methods
		public void WriteXML(XmlWriter xw) {
			if (this.Count <= 0) return;

			xw.WriteStartElement(XML_TAG_OPTIONS);
			int len = this.Count;
			for (int i = 0; i < len; i++) {
				xw.WriteStartElement(XML_TAG_OPTION);
				xw.WriteElementString(XML_TAG_NAME, this.GetKey(i));
				xw.WriteElementString(XML_TAG_VALUE, this.Get(i));
				xw.WriteEndElement();
			}
			xw.WriteEndElement();
		}
		#endregion
	}
}
