#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Xml;

namespace HoytSoft.Common.Gateways.LinkPoint {
	public class LinkPointItemInformation {
		#region Constants
		private const string
			XML_TAG_ITEM		= "item", 
			XML_TAG_ID			= "id",
			XML_TAG_DESCRIPTION = "description",
			XML_TAG_PRICE		= "price",
			XML_TAG_QUANTITY	= "quantity",
			XML_TAG_SERIAL		= "serial",
			XML_TAG_ESDTYPE		= "esdtype",
			XML_TAG_SOFTFILE	= "softfile"
		;
		#endregion

		#region Variables
		private int quantity;
		private double price;
		private string id, description, serialNumber, esdFileName;
		private LinkPointESDType esdType;
		private LinkPointOptionsCollection options = null;
		#endregion

		#region Constructors
		public LinkPointItemInformation(double Price, int Quantity) : this(Guid.NewGuid().ToString("N"), Price, Quantity) {
		}
		public LinkPointItemInformation(string ID, double Price, int Quantity) : this(ID, string.Empty, Price, Quantity) {
		}
		public LinkPointItemInformation(string ID, string Description, double Price, int Quantity) : this(ID, Description, Price, Quantity, string.Empty) {
		}
		public LinkPointItemInformation(string ID, string Description, double Price, int Quantity, string SerialNumber) : this(ID, Description, Price, Quantity, SerialNumber, LinkPointESDType.Default, string.Empty) {
		}
		public LinkPointItemInformation(string ID, string Description, double Price, int Quantity, string SerialNumber, LinkPointESDType ESDType, string ESDFileName) {
			this.id = ID;
			this.description = Description;
			this.serialNumber = SerialNumber;
			this.esdFileName = ESDFileName;
			this.quantity = Quantity;
			this.price = Price;
			this.esdType = ESDType;
			if (this.description.Length > 120)
				throw new ArgumentOutOfRangeException("Description is limited to 120 characters");
		}
		#endregion

		#region Properties
		public string ID { get { return this.id; } }
		public string Description { get { return this.description; } }
		public string SerialNumber { get { return this.serialNumber; } }
		public string ESDFileName { get { return this.esdFileName; } }

		public double Price { get { return this.price; } }
		public int Quantity { get { return this.quantity; } }
		public LinkPointESDType ESDType { get { return this.esdType; } }
		#endregion

		#region Helper Methods
		public void AddOption(string Name, string Value) {
			if (this.options == null)
				this.options = new LinkPointOptionsCollection();
			this.options.Add(Name, Value);
		}

		public bool RemoveOption(string Name) {
			if (this.options == null || this.options[Name] == null)
				return false;
			this.options.Remove(Name);
			return true;
		}

		public bool ClearOptions() {
			if (this.options == null) return false;
			this.options.Clear();
			return true;
		}

		public string GetOption(string Name) {
			if (this.options == null)
				return null;
			return this.options[Name];
		}

		public string GetESDTypeString() {
			return this.GetESDTypeString(this.esdType);
		}

		public string GetESDTypeString(LinkPointESDType ESDType) {
			switch (ESDType) {
				case LinkPointESDType.File:
					return "softgood";
				case LinkPointESDType.KeyGenerator:
					return "key";
				default:
					return string.Empty;
			}
		}

		public bool IsRequired(LinkPointItemField Field) {
			if (Field == LinkPointItemField.ID || Field == LinkPointItemField.Price || Field == LinkPointItemField.Quantity)
				return true;
			switch (this.esdType) {
				case LinkPointESDType.File:
				case LinkPointESDType.KeyGenerator:
					switch (Field) {
						case LinkPointItemField.Description:
						case LinkPointItemField.ESDType:
						case LinkPointItemField.SoftFile:
							return true;
					}
					break;
				default:
					return false;
			}
			return false;
		}

		public void WriteXML(XmlWriter xw) {
			xw.WriteStartElement(XML_TAG_ITEM);

			xw.WriteElementString(XML_TAG_ID, this.id);
			if (!string.IsNullOrEmpty(this.description))
				xw.WriteElementString(XML_TAG_DESCRIPTION, this.description);
			xw.WriteElementString(XML_TAG_PRICE, this.price.ToString());
			xw.WriteElementString(XML_TAG_QUANTITY, this.quantity.ToString());
			if (!string.IsNullOrEmpty(this.serialNumber))
				xw.WriteElementString(XML_TAG_SERIAL, this.serialNumber);
			switch (this.esdType) {
				case LinkPointESDType.File:
				case LinkPointESDType.KeyGenerator:
					xw.WriteElementString(XML_TAG_ESDTYPE, this.GetESDTypeString());
					xw.WriteElementString(XML_TAG_SOFTFILE, this.esdFileName);
					break;
			}
			if (this.options != null && this.options.Count > 0)
				this.options.WriteXML(xw);
			xw.WriteEndElement();
		}
		#endregion
	}
}
