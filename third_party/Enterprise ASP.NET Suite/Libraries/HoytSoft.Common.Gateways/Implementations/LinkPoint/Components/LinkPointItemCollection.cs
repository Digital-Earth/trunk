#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointItemCollection : System.Collections.CollectionBase {
		#region Constants
		private const string
			XML_TAG_ITEMS = "items"
		;
		#endregion

		#region CollectionBase Methods
		public LinkPointItemInformation this[int index] {
			get { return ((LinkPointItemInformation)List[index]); }
			set { List[index] = value; }
		}

		public void Clear() {
			List.Clear();
		}

		public int Add(LinkPointItemInformation value) {
			return (List.Add(value));
		}

		public int IndexOf(LinkPointItemInformation value) {
			return (List.IndexOf(value));
		}

		public void Insert(int index, LinkPointItemInformation value) {
			List.Insert(index, value);
		}

		public void Remove(LinkPointItemInformation value) {
			List.Remove(value);
		}

		public void RemoveAt(int index) {
			List.RemoveAt(index);
		}

		public bool Contains(LinkPointItemInformation value) {
			// If value is not of type Int16, this will return false.
			return (List.Contains(value));
		}

		protected override void OnValidate(Object value) {
			if (value.GetType() != typeof(LinkPointItemInformation))
				throw new ArgumentException("value must be of type LinkPointItemInformation", "value");
		}
		#endregion

		#region Helper Methods
		public void WriteXML(XmlWriter xw) {
			if (this.Count > 0) {
				xw.WriteStartElement(XML_TAG_ITEMS);
				foreach(LinkPointItemInformation i in this)
					i.WriteXML(xw);
				xw.WriteEndElement();
			}
		}
		#endregion
	}
}
