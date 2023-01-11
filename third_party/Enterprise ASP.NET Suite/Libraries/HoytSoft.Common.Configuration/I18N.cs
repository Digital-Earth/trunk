#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.IO;
using System.Xml;
using System.Web;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Net.Mail;

namespace HoytSoft.Common.Configuration {
	public class I18N {
		#region Constants
		///<summary>The default name of the XML file containing the e-mail templates, the name to use to specify something's default, and the name to use to specify nothing.</summary>
		public const string
			FILE_WEB_I18N					= "Web.i18n.config"
		;

		private const string
			XPATH_SUPPORTED_CULTURES		= "I18N/SupportedCultures/Culture"
		;

		private const string 
			XML_ATTRIB_NAME					= "name",
			XML_ATTRIB_LCID					= "lcid",
			XML_ATTRIB_COUNTRY_ABBREV		= "countryAbbreviationISO3166",
			XML_ATTRIB_LANGUAGE				= "language",
			XML_ATTRIB_REGION				= "region",
			XML_ATTRIB_COUNTRY				= "country"
		;
		#endregion

		#region Variables
		private static bool loaded;
		private static List<SupportedCulture> lstSupportedCultures;
		#endregion

		#region Constructors
		static I18N() {
			loaded = false;
			Load();
		}

		protected static void Load() {
			if (loaded) return;
			loaded = true;
			Refresh();
		}
		#endregion

		#region Main Methods
		private static void loadDocument(XmlDocument doc) {
			if (doc == null) return;

			XmlNode x = null;
			XmlNodeList xnl = null;
			string name = string.Empty;
			int lcid = 0;
			string countryAbbrev = string.Empty;
			string language = string.Empty;
			string region = string.Empty;
			string country = string.Empty;

			if (lstSupportedCultures != null)
				lstSupportedCultures.Clear();

			if ((xnl = doc.SelectNodes(XPATH_SUPPORTED_CULTURES)) != null && xnl.Count > 0) {
				foreach(XmlNode xn in xnl) {
					name = string.Empty;
					lcid = 0;
					countryAbbrev = string.Empty;
					language = string.Empty;
					region = string.Empty;
					country = string.Empty;

					//Now pull out the attributes...
					if (xn.Attributes[XML_ATTRIB_NAME] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_NAME].InnerText))
						name = xn.Attributes[XML_ATTRIB_NAME].InnerText;
					if (xn.Attributes[XML_ATTRIB_LCID] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_LCID].InnerText) && int.TryParse(xn.Attributes[XML_ATTRIB_LCID].InnerText, out lcid))
						lcid = lcid;
					if (xn.Attributes[XML_ATTRIB_COUNTRY_ABBREV] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_COUNTRY_ABBREV].InnerText))
						countryAbbrev = xn.Attributes[XML_ATTRIB_COUNTRY_ABBREV].InnerText;
					if (xn.Attributes[XML_ATTRIB_LANGUAGE] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_LANGUAGE].InnerText))
						language = xn.Attributes[XML_ATTRIB_LANGUAGE].InnerText;
					if (xn.Attributes[XML_ATTRIB_REGION] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_REGION].InnerText))
						region = xn.Attributes[XML_ATTRIB_REGION].InnerText;
					if (xn.Attributes[XML_ATTRIB_COUNTRY] != null && !string.IsNullOrEmpty(xn.Attributes[XML_ATTRIB_COUNTRY].InnerText))
						country = xn.Attributes[XML_ATTRIB_COUNTRY].InnerText;

					//Add it to our list of dictionaries
					if (!string.IsNullOrEmpty(name) && lcid > 0 && !string.IsNullOrEmpty(countryAbbrev) && !string.IsNullOrEmpty(language) && !string.IsNullOrEmpty(region) && !string.IsNullOrEmpty(country)) {
						if (lstSupportedCultures == null)
							lstSupportedCultures = new List<SupportedCulture>();
						lstSupportedCultures.Add(new SupportedCulture(name, lcid, countryAbbrev, language, region, country));
					}
				}
			}
		}
		#endregion

		#region Public Static Methods
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh() {
			if (HttpContext.Current != null)
				Refresh(HttpContext.Current.Server.MapPath(FILE_WEB_I18N));
			else
				Refresh(FILE_WEB_I18N);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh(Stream Stream) {
			if (Stream == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(Stream);
			loadDocument(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh(XmlReader XmlReader) {
			if (XmlReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(XmlReader);
			loadDocument(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh(TextReader TextReader) {
			if (TextReader == null)
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(TextReader);
			loadDocument(doc);
		}
		///<summary></summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Refresh(string FileName) {
			FileInfo fi = new FileInfo(FileName);
			//Debug.WriteLine(fi.FullName);
			if (!File.Exists(FileName))
				return;
			XmlDocument doc = new XmlDocument();
			doc.Load(FileName);
			loadDocument(doc);
		}

		public static SupportedCulture FindSupportedCultureByLCID(int LCID) {
			if (lstSupportedCultures == null)
				return null;
			foreach (SupportedCulture sc in lstSupportedCultures)
				if (sc.LCID == LCID)
					return sc;
			return null;
		}

		public static SupportedCulture FindSupportedCultureByName(string Name) {
			if (lstSupportedCultures == null || string.IsNullOrEmpty(Name))
				return null;
			foreach (SupportedCulture sc in lstSupportedCultures)
				if (Name.Equals(sc.Name, StringComparison.InvariantCultureIgnoreCase))
					return sc;
			return null;
		}

		public static List<SupportedCulture> FindSupportedCulturesByRegion(string Region) {
			List<SupportedCulture> lst = new List<SupportedCulture>();
			if (string.IsNullOrEmpty(Region) || lstSupportedCultures == null || lstSupportedCultures.Count <= 0)
				return lst;

			foreach (SupportedCulture sc in lstSupportedCultures)
				if (Region.Equals(sc.Region, StringComparison.InvariantCultureIgnoreCase))
					lst.Add(sc);

			return lst;
		}
		#endregion

		#region Public Methods
		#endregion

		#region Public Static Properties
		public static List<SupportedCulture> SupportedCultures {
			get {
				if (lstSupportedCultures == null)
					Refresh();
				return lstSupportedCultures;
			}
		}
		#endregion
	}

	public class SupportedCulture {
		#region Variables
		private string name;
		private int lcid;
		private string countryAbbrev;
		private string language;
		private string region;
		private string country;
		#endregion

		#region Constructors
		public SupportedCulture(string Name, int LCID, string CountryAbbreviationISO3166, string Language, string Region, string Country) {
			this.name = Name;
			this.lcid = LCID;
			this.countryAbbrev = CountryAbbreviationISO3166;
			this.language = Language;
			this.region = Region;
			this.country = Country;
		}
		#endregion

		#region Properties
		public string Name {
			get { return name; }
			set { name = value; }
		}

		public int LCID {
			get { return lcid; }
			set { lcid = value; }
		}

		public string CountryAbbreviationISO3166 {
			get { return countryAbbrev; }
			set { countryAbbrev = value; }
		}

		public string Language {
			get { return language; }
			set { language = value; }
		}

		public string Region {
			get { return region; }
			set { region = value; }
		}

		public string Country {
			get { return country; }
			set { country = value; }
		}
		#endregion
	}
}
