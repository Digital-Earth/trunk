#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways {
	public abstract class CountryInformation {
		#region Variables
		private static System.Collections.Generic.Dictionary<Country, string> fullNames;
		private static System.Collections.Generic.Dictionary<Country, string> abbreviations;
		private Country myCountry;
		#endregion

		#region Constructors
		static CountryInformation() {
			#region Add Full Names
			fullNames = new System.Collections.Generic.Dictionary<Country, string>(210);
			fullNames.Add(Country.Algeria, "Algeria");
			fullNames.Add(Country.Burundi, "Burundi");
			fullNames.Add(Country.CentralAfricanRepublic, "Central African Republic");
			fullNames.Add(Country.Congo, "Congo");
			fullNames.Add(Country.EquatorialGuinea, "Equatorial Guinea");
			fullNames.Add(Country.Egypt, "Egypt");
			fullNames.Add(Country.Ghana, "Ghana");
			fullNames.Add(Country.Kenya, "Kenya");
			fullNames.Add(Country.Madagascar, "Madagascar");
			fullNames.Add(Country.Mayotte, "Mayotte");
			fullNames.Add(Country.Malawi, "Malawi");
			fullNames.Add(Country.Nigeria, "Nigeria");
			fullNames.Add(Country.SaoTomeAndPrincipe, "Sao Tome and Principe");
			fullNames.Add(Country.Somalia, "Somalia");
			fullNames.Add(Country.Swaziland, "Swaziland");
			fullNames.Add(Country.Uganda, "Uganda");
			fullNames.Add(Country.Zambia, "Zambia");
			fullNames.Add(Country.Benin, "Benin");
			fullNames.Add(Country.Cameroon, "Cameroon");
			fullNames.Add(Country.Chad, "Chad");
			fullNames.Add(Country.CoteDIvoire, "Cote D'Ivoire");
			fullNames.Add(Country.Eritrea, "Eritrea");
			fullNames.Add(Country.Gabon, "Gabon");
			fullNames.Add(Country.Guinea, "Guinea");
			fullNames.Add(Country.Lesotho, "Lesotho");
			fullNames.Add(Country.Mali, "Mali");
			fullNames.Add(Country.Morocco, "Morocco");
			fullNames.Add(Country.Namibia, "Namibia");
			fullNames.Add(Country.Reunion, "Reunion");
			fullNames.Add(Country.Senegal, "Senegal");
			fullNames.Add(Country.SouthAfrica, "South Africa");
			fullNames.Add(Country.Tanzania, "Tanzania");
			fullNames.Add(Country.WesternSahara, "Western Sahara");
			fullNames.Add(Country.Zimbabwe, "Zimbabwe");
			fullNames.Add(Country.BurkinaFaso, "Burkina Faso");
			fullNames.Add(Country.CapeVerde, "Cape Verde");
			fullNames.Add(Country.Comoros, "Comoros");
			fullNames.Add(Country.Djibouti, "Djibouti");
			fullNames.Add(Country.Ethiopia, "Ethiopia");
			fullNames.Add(Country.Gambia, "Gambia");
			fullNames.Add(Country.GuineaBissau, "Guinea-Bissau");
			fullNames.Add(Country.Liberia, "Liberia");
			fullNames.Add(Country.Mauritania, "Mauritania");
			fullNames.Add(Country.Mozambique, "Mozambique");
			fullNames.Add(Country.Niger, "Niger");
			fullNames.Add(Country.StHelena, "St. Helena");
			fullNames.Add(Country.SierraLeone, "Sierra Leone");
			fullNames.Add(Country.Sudan, "Sudan");
			fullNames.Add(Country.Togo, "Togo");
			fullNames.Add(Country.Zaire, "Zaire");
			fullNames.Add(Country.Antarctica, "Antarctica");
			fullNames.Add(Country.Afghanistan, "Afghanistan");
			fullNames.Add(Country.Brunei, "Brunei");
			fullNames.Add(Country.HongKong, "Hong Kong");
			fullNames.Add(Country.Japan, "Japan");
			fullNames.Add(Country.Laos, "Laos");
			fullNames.Add(Country.Maldives, "Maldives");
			fullNames.Add(Country.Pakistan, "Pakistan");
			fullNames.Add(Country.Russia, "Russia");
			fullNames.Add(Country.SriLanka, "Sri Lanka");
			fullNames.Add(Country.Thailand, "Thailand");
			fullNames.Add(Country.Vietnam, "Vietnam");
			fullNames.Add(Country.Bangladesh, "Bangladesh");
			fullNames.Add(Country.Cambodia, "Cambodia");
			fullNames.Add(Country.India, "India");
			fullNames.Add(Country.Kazakhstan, "Kazakhstan");
			fullNames.Add(Country.Macau, "Macau");
			fullNames.Add(Country.Mongolia, "Mongolia");
			fullNames.Add(Country.Philippines, "Philippines");
			fullNames.Add(Country.Seychelles, "Seychelles");
			fullNames.Add(Country.Taiwan, "Taiwan");
			fullNames.Add(Country.Turkmenistan, "Turkmenistan");
			fullNames.Add(Country.Bhutan, "Bhutan");
			fullNames.Add(Country.China, "China");
			fullNames.Add(Country.Indonesia, "Indonesia");
			fullNames.Add(Country.Kyrgyzstan, "Kyrgyzstan");
			fullNames.Add(Country.Malaysia, "Malaysia");
			fullNames.Add(Country.Nepal, "Nepal");
			fullNames.Add(Country.RepublicOfKorea, "Republic of Korea");
			fullNames.Add(Country.Singapore, "Singapore");
			fullNames.Add(Country.Tajikistan, "Tajikistan");
			fullNames.Add(Country.Uzbekistan, "Uzbekistan");
			fullNames.Add(Country.AmericanSamoa, "American Samoa");
			fullNames.Add(Country.Fiji, "Fiji");
			fullNames.Add(Country.Kiribati, "Kiribati");
			fullNames.Add(Country.NewCaledonia, "New Caledonia");
			fullNames.Add(Country.Palau, "Palau");
			fullNames.Add(Country.SolomonIslands, "Solomon Islands");
			fullNames.Add(Country.Vanuatu, "Vanuatu");
			fullNames.Add(Country.Australia, "Australia");
			fullNames.Add(Country.FrenchPolynesia, "French Polynesia");
			fullNames.Add(Country.MarshallIslands, "Marshall Islands");
			fullNames.Add(Country.NewZealand, "New Zealand");
			fullNames.Add(Country.PapuaNewGuinea, "Papua New Guinea");
			fullNames.Add(Country.Tonga, "Tonga");
			fullNames.Add(Country.FederatedStatesOfMicronesia, "Federated States of Micronesia");
			fullNames.Add(Country.Guam, "Guam");
			fullNames.Add(Country.Nauru, "Nauru");
			fullNames.Add(Country.NorthernMarianaIslands, "Northern Mariana Islands");
			fullNames.Add(Country.Pitcairn, "Pitcairn");
			fullNames.Add(Country.Tuvalu, "Tuvalu");
			fullNames.Add(Country.Anguilla, "Anguilla");
			fullNames.Add(Country.Bahamas, "Bahamas");
			fullNames.Add(Country.BritishVirginIslands, "British Virgin Islands");
			fullNames.Add(Country.DominicanRepublic, "Dominican Republic");
			fullNames.Add(Country.Haiti, "Haiti");
			fullNames.Add(Country.NetherlandsAntilles, "Netherlands Antilles");
			fullNames.Add(Country.StLucia, "St. Lucia");
			fullNames.Add(Country.TurksandCaicosIslands, "Turks and Caicos Islands");
			fullNames.Add(Country.AntiquaandBarbuda, "Antiqua and Barbuda");
			fullNames.Add(Country.Barbados, "Barbados");
			fullNames.Add(Country.CaymanIslands, "Cayman Islands");
			fullNames.Add(Country.Grenada, "Grenada");
			fullNames.Add(Country.Jamaica, "Jamaica");
			fullNames.Add(Country.PuertoRico, "Puerto Rico");
			fullNames.Add(Country.StVincentAndTheGrenadines, "St. Vincent and the Grenadines");
			fullNames.Add(Country.Aruba, "Aruba");
			fullNames.Add(Country.Bermuda, "Bermuda");
			fullNames.Add(Country.Dominica, "Dominica");
			fullNames.Add(Country.Guadeloupe, "Guadeloupe");
			fullNames.Add(Country.Martinique, "Martinique");
			fullNames.Add(Country.StKittsandNevis, "St. Kitts and Nevis");
			fullNames.Add(Country.TrinidadAndTobago, "Trinidad and Tobago");
			fullNames.Add(Country.Belize, "Belize");
			fullNames.Add(Country.Guatemala, "Guatemala");
			fullNames.Add(Country.Panama, "Panama");
			fullNames.Add(Country.CostaRica, "Costa Rica");
			fullNames.Add(Country.Honduras, "Honduras");
			fullNames.Add(Country.ElSalvador, "El Salvador");
			fullNames.Add(Country.Nicaragua, "Nicaragua");
			fullNames.Add(Country.Albania, "Albania");
			fullNames.Add(Country.Austria, "Austria");
			fullNames.Add(Country.Belgium, "Belgium");
			fullNames.Add(Country.Cyprus, "Cyprus");
			fullNames.Add(Country.Estonia, "Estonia");
			fullNames.Add(Country.France, "France");
			fullNames.Add(Country.Gibraltar, "Gibraltar");
			fullNames.Add(Country.Hungary, "Hungary");
			fullNames.Add(Country.Italy, "Italy");
			fullNames.Add(Country.Lithuania, "Lithuania");
			fullNames.Add(Country.MetropolitanFrance, "Metropolitan France");
			fullNames.Add(Country.Norway, "Norway");
			fullNames.Add(Country.Romania, "Romania");
			fullNames.Add(Country.Spain, "Spain");
			fullNames.Add(Country.Switzerland, "Switzerland");
			fullNames.Add(Country.Ukraine, "Ukraine");
			fullNames.Add(Country.Yugoslavia, "Yugoslavia");
			fullNames.Add(Country.Andorra, "Andorra");
			fullNames.Add(Country.Azerbaijan, "Azerbaijan");
			fullNames.Add(Country.Bulgaria, "Bulgaria");
			fullNames.Add(Country.CzechRepublic, "Czech Republic");
			fullNames.Add(Country.FaroeIslands, "Faroe Islands");
			fullNames.Add(Country.Georgia, "Georgia");
			fullNames.Add(Country.Greece, "Greece");
			fullNames.Add(Country.Iceland, "Iceland");
			fullNames.Add(Country.Latvia, "Latvia");
			fullNames.Add(Country.Luxembourg, "Luxembourg");
			fullNames.Add(Country.Moldova, "Moldova");
			fullNames.Add(Country.Poland, "Poland");
			fullNames.Add(Country.Slovakia, "Slovakia");
			fullNames.Add(Country.SvalbardAndJanMayenIslands, "Svalbard and Jan Mayen Islands");
			fullNames.Add(Country.TheFormerYugoslavRepublicOfMacedonia, "The former Yugoslav Republic of Macedonia");
			fullNames.Add(Country.UnitedKingdom, "United Kingdom");
			fullNames.Add(Country.Armenia, "Armenia");
			fullNames.Add(Country.Belarus, "Belarus");
			fullNames.Add(Country.Croatia, "Croatia");
			fullNames.Add(Country.Denmark, "Denmark");
			fullNames.Add(Country.Finland, "Finland");
			fullNames.Add(Country.Germany, "Germany");
			fullNames.Add(Country.Greenland, "Greenland");
			fullNames.Add(Country.Ireland, "Ireland");
			fullNames.Add(Country.Liechtenstein, "Liechtenstein");
			fullNames.Add(Country.Malta, "Malta");
			fullNames.Add(Country.Netherlands, "Netherlands");
			fullNames.Add(Country.Portugal, "Portugal");
			fullNames.Add(Country.Slovenia, "Slovenia");
			fullNames.Add(Country.Sweden, "Sweden");
			fullNames.Add(Country.Turkey, "Turkey");
			fullNames.Add(Country.VaticanCity, "Vatican City");
			fullNames.Add(Country.Israel, "Israel");
			fullNames.Add(Country.Lebanon, "Lebanon");
			fullNames.Add(Country.SaudiArabia, "Saudi Arabia");
			fullNames.Add(Country.Yemen, "Yemen");
			fullNames.Add(Country.Jordan, "Jordan");
			fullNames.Add(Country.Oman, "Oman");
			fullNames.Add(Country.Syria, "Syria");
			fullNames.Add(Country.Kuwait, "Kuwait");
			fullNames.Add(Country.Qatar, "Qatar");
			fullNames.Add(Country.UnitedArabEmirates, "United Arab Emirates");
			fullNames.Add(Country.Canada, "Canada");
			fullNames.Add(Country.Mexico, "Mexico");
			fullNames.Add(Country.UnitedStates, "United States");
			fullNames.Add(Country.Argentina, "Argentina");
			fullNames.Add(Country.Chile, "Chile");
			fullNames.Add(Country.FalklandIslands, "Falkland Islands");
			fullNames.Add(Country.Paraguay, "Paraguay");
			fullNames.Add(Country.Suriname, "Suriname");
			fullNames.Add(Country.Bolivia, "Bolivia");
			fullNames.Add(Country.Colombia, "Colombia");
			fullNames.Add(Country.FrenchGuiana, "French Guiana");
			fullNames.Add(Country.Uruguay, "Uruguay");
			fullNames.Add(Country.Brazil, "Brazil");
			fullNames.Add(Country.Equador, "Equador");
			fullNames.Add(Country.Guyana, "Guyana");
			fullNames.Add(Country.Peru, "Peru");
			fullNames.Add(Country.Venezuela, "Venezuela");
			fullNames.Add(Country.Bahrain, "Bahrain");
			fullNames.Add(Country.BouvetIslands, "Bouvet Islands");
			fullNames.Add(Country.BritishIndianOceanTerritory, "British Indian Ocean Territory");
			#endregion

			#region Add Abbreviations
			abbreviations = new System.Collections.Generic.Dictionary<Country, string>(210);
			abbreviations.Add(Country.Algeria, "DZ");
			abbreviations.Add(Country.Burundi, "BI");
			abbreviations.Add(Country.CentralAfricanRepublic, "CF");
			abbreviations.Add(Country.Congo, "CG");
			abbreviations.Add(Country.EquatorialGuinea, "GQ");
			abbreviations.Add(Country.Egypt, "EG");
			abbreviations.Add(Country.Ghana, "GH");
			abbreviations.Add(Country.Kenya, "KE");
			abbreviations.Add(Country.Madagascar, "MG");
			abbreviations.Add(Country.Mayotte, "YT");
			abbreviations.Add(Country.Malawi, "MW");
			abbreviations.Add(Country.Nigeria, "NG");
			abbreviations.Add(Country.SaoTomeAndPrincipe, "ST");
			abbreviations.Add(Country.Somalia, "SO");
			abbreviations.Add(Country.Swaziland, "SZ");
			abbreviations.Add(Country.Uganda, "UG");
			abbreviations.Add(Country.Zambia, "ZM");
			abbreviations.Add(Country.Benin, "BJ");
			abbreviations.Add(Country.Cameroon, "CM");
			abbreviations.Add(Country.Chad, "TD");
			abbreviations.Add(Country.CoteDIvoire, "CI");
			abbreviations.Add(Country.Eritrea, "ER");
			abbreviations.Add(Country.Gabon, "GA");
			abbreviations.Add(Country.Guinea, "GN");
			abbreviations.Add(Country.Lesotho, "LS");
			abbreviations.Add(Country.Mali, "ML");
			abbreviations.Add(Country.Morocco, "MA");
			abbreviations.Add(Country.Namibia, "NA");
			abbreviations.Add(Country.Reunion, "RE");
			abbreviations.Add(Country.Senegal, "SN");
			abbreviations.Add(Country.SouthAfrica, "ZA");
			abbreviations.Add(Country.Tanzania, "TZ");
			abbreviations.Add(Country.WesternSahara, "EH");
			abbreviations.Add(Country.Zimbabwe, "ZW");
			abbreviations.Add(Country.BurkinaFaso, "BF");
			abbreviations.Add(Country.CapeVerde, "CV");
			abbreviations.Add(Country.Comoros, "KM");
			abbreviations.Add(Country.Djibouti, "DJ");
			abbreviations.Add(Country.Ethiopia, "ET");
			abbreviations.Add(Country.Gambia, "GM");
			abbreviations.Add(Country.GuineaBissau, "GW");
			abbreviations.Add(Country.Liberia, "LR");
			abbreviations.Add(Country.Mauritania, "MR");
			abbreviations.Add(Country.Mozambique, "MZ");
			abbreviations.Add(Country.Niger, "NE");
			abbreviations.Add(Country.StHelena, "SH");
			abbreviations.Add(Country.SierraLeone, "SL");
			abbreviations.Add(Country.Sudan, "SD");
			abbreviations.Add(Country.Togo, "TG");
			abbreviations.Add(Country.Zaire, "ZR");
			abbreviations.Add(Country.Antarctica, "AQ");
			abbreviations.Add(Country.Afghanistan, "AF");
			abbreviations.Add(Country.Brunei, "BN");
			abbreviations.Add(Country.HongKong, "HK");
			abbreviations.Add(Country.Japan, "JP");
			abbreviations.Add(Country.Laos, "LA");
			abbreviations.Add(Country.Maldives, "MV");
			abbreviations.Add(Country.Pakistan, "PK");
			abbreviations.Add(Country.Russia, "RU");
			abbreviations.Add(Country.SriLanka, "LK");
			abbreviations.Add(Country.Thailand, "TH");
			abbreviations.Add(Country.Vietnam, "VN");
			abbreviations.Add(Country.Bangladesh, "BD");
			abbreviations.Add(Country.Cambodia, "KH");
			abbreviations.Add(Country.India, "IN");
			abbreviations.Add(Country.Kazakhstan, "KZ");
			abbreviations.Add(Country.Macau, "MO");
			abbreviations.Add(Country.Mongolia, "MN");
			abbreviations.Add(Country.Philippines, "PH");
			abbreviations.Add(Country.Seychelles, "SC");
			abbreviations.Add(Country.Taiwan, "TW");
			abbreviations.Add(Country.Turkmenistan, "TM");
			abbreviations.Add(Country.Bhutan, "BT");
			abbreviations.Add(Country.China, "CN");
			abbreviations.Add(Country.Indonesia, "ID");
			abbreviations.Add(Country.Kyrgyzstan, "KG");
			abbreviations.Add(Country.Malaysia, "MY");
			abbreviations.Add(Country.Nepal, "NP");
			abbreviations.Add(Country.RepublicOfKorea, "KR");
			abbreviations.Add(Country.Singapore, "SG");
			abbreviations.Add(Country.Tajikistan, "TJ");
			abbreviations.Add(Country.Uzbekistan, "UZ");
			abbreviations.Add(Country.AmericanSamoa, "AS");
			abbreviations.Add(Country.Fiji, "FJ");
			abbreviations.Add(Country.Kiribati, "KI");
			abbreviations.Add(Country.NewCaledonia, "NC");
			abbreviations.Add(Country.Palau, "PW");
			abbreviations.Add(Country.SolomonIslands, "SB");
			abbreviations.Add(Country.Vanuatu, "VU");
			abbreviations.Add(Country.Australia, "AU");
			abbreviations.Add(Country.FrenchPolynesia, "PF");
			abbreviations.Add(Country.MarshallIslands, "MH");
			abbreviations.Add(Country.NewZealand, "NZ");
			abbreviations.Add(Country.PapuaNewGuinea, "PG");
			abbreviations.Add(Country.Tonga, "TO");
			abbreviations.Add(Country.FederatedStatesOfMicronesia, "FM");
			abbreviations.Add(Country.Guam, "GU");
			abbreviations.Add(Country.Nauru, "NR");
			abbreviations.Add(Country.NorthernMarianaIslands, "MP");
			abbreviations.Add(Country.Pitcairn, "PN");
			abbreviations.Add(Country.Tuvalu, "TV");
			abbreviations.Add(Country.Anguilla, "AI");
			abbreviations.Add(Country.Bahamas, "BS");
			abbreviations.Add(Country.BritishVirginIslands, "VI");
			abbreviations.Add(Country.DominicanRepublic, "DO");
			abbreviations.Add(Country.Haiti, "HT");
			abbreviations.Add(Country.NetherlandsAntilles, "AN");
			abbreviations.Add(Country.StLucia, "LC");
			abbreviations.Add(Country.TurksandCaicosIslands, "TC");
			abbreviations.Add(Country.AntiquaandBarbuda, "AG");
			abbreviations.Add(Country.Barbados, "BB");
			abbreviations.Add(Country.CaymanIslands, "KY");
			abbreviations.Add(Country.Grenada, "GD");
			abbreviations.Add(Country.Jamaica, "JM");
			abbreviations.Add(Country.PuertoRico, "PR");
			abbreviations.Add(Country.StVincentAndTheGrenadines, "VC");
			abbreviations.Add(Country.Aruba, "AW");
			abbreviations.Add(Country.Bermuda, "BM");
			abbreviations.Add(Country.Dominica, "DM");
			abbreviations.Add(Country.Guadeloupe, "GP");
			abbreviations.Add(Country.Martinique, "MQ");
			abbreviations.Add(Country.StKittsandNevis, "KN");
			abbreviations.Add(Country.TrinidadAndTobago, "TT");
			abbreviations.Add(Country.Belize, "BZ");
			abbreviations.Add(Country.Guatemala, "GT");
			abbreviations.Add(Country.Panama, "PA");
			abbreviations.Add(Country.CostaRica, "CR");
			abbreviations.Add(Country.Honduras, "HN");
			abbreviations.Add(Country.ElSalvador, "SV");
			abbreviations.Add(Country.Nicaragua, "NI");
			abbreviations.Add(Country.Albania, "AL");
			abbreviations.Add(Country.Austria, "AT");
			abbreviations.Add(Country.Belgium, "BE");
			abbreviations.Add(Country.Cyprus, "CY");
			abbreviations.Add(Country.Estonia, "EE");
			abbreviations.Add(Country.France, "FR");
			abbreviations.Add(Country.Gibraltar, "GI");
			abbreviations.Add(Country.Hungary, "HU");
			abbreviations.Add(Country.Italy, "IT");
			abbreviations.Add(Country.Lithuania, "LT");
			abbreviations.Add(Country.MetropolitanFrance, "FX");
			abbreviations.Add(Country.Norway, "NO");
			abbreviations.Add(Country.Romania, "RO");
			abbreviations.Add(Country.Spain, "ES");
			abbreviations.Add(Country.Switzerland, "CH");
			abbreviations.Add(Country.Ukraine, "UA");
			abbreviations.Add(Country.Yugoslavia, "YU");
			abbreviations.Add(Country.Andorra, "AD");
			abbreviations.Add(Country.Azerbaijan, "AZ");
			abbreviations.Add(Country.Bulgaria, "BG");
			abbreviations.Add(Country.CzechRepublic, "CZ");
			abbreviations.Add(Country.FaroeIslands, "FO");
			abbreviations.Add(Country.Georgia, "GE");
			abbreviations.Add(Country.Greece, "GR");
			abbreviations.Add(Country.Iceland, "IS");
			abbreviations.Add(Country.Latvia, "LV");
			abbreviations.Add(Country.Luxembourg, "LU");
			abbreviations.Add(Country.Moldova, "MD");
			abbreviations.Add(Country.Poland, "PL");
			abbreviations.Add(Country.Slovakia, "SK");
			abbreviations.Add(Country.SvalbardAndJanMayenIslands, "SJ");
			abbreviations.Add(Country.TheFormerYugoslavRepublicOfMacedonia, "MK");
			abbreviations.Add(Country.UnitedKingdom, "GB");
			abbreviations.Add(Country.Armenia, "AM");
			abbreviations.Add(Country.Belarus, "BY");
			abbreviations.Add(Country.Croatia, "HR");
			abbreviations.Add(Country.Denmark, "DK");
			abbreviations.Add(Country.Finland, "FI");
			abbreviations.Add(Country.Germany, "DE");
			abbreviations.Add(Country.Greenland, "GL");
			abbreviations.Add(Country.Ireland, "IE");
			abbreviations.Add(Country.Liechtenstein, "LI");
			abbreviations.Add(Country.Malta, "MT");
			abbreviations.Add(Country.Netherlands, "NL");
			abbreviations.Add(Country.Portugal, "PT");
			abbreviations.Add(Country.Slovenia, "SI");
			abbreviations.Add(Country.Sweden, "SE");
			abbreviations.Add(Country.Turkey, "TR");
			abbreviations.Add(Country.VaticanCity, "VA");
			abbreviations.Add(Country.Israel, "IL");
			abbreviations.Add(Country.Lebanon, "LB");
			abbreviations.Add(Country.SaudiArabia, "SA");
			abbreviations.Add(Country.Yemen, "YE");
			abbreviations.Add(Country.Jordan, "JO");
			abbreviations.Add(Country.Oman, "OM");
			abbreviations.Add(Country.Syria, "SY");
			abbreviations.Add(Country.Kuwait, "KW");
			abbreviations.Add(Country.Qatar, "QA");
			abbreviations.Add(Country.UnitedArabEmirates, "AE");
			abbreviations.Add(Country.Canada, "CA");
			abbreviations.Add(Country.Mexico, "MX");
			abbreviations.Add(Country.UnitedStates, "US");
			abbreviations.Add(Country.Argentina, "AR");
			abbreviations.Add(Country.Chile, "CL");
			abbreviations.Add(Country.FalklandIslands, "FK");
			abbreviations.Add(Country.Paraguay, "PY");
			abbreviations.Add(Country.Suriname, "SR");
			abbreviations.Add(Country.Bolivia, "BO");
			abbreviations.Add(Country.Colombia, "CO");
			abbreviations.Add(Country.FrenchGuiana, "GF");
			abbreviations.Add(Country.Uruguay, "UY");
			abbreviations.Add(Country.Brazil, "BR");
			abbreviations.Add(Country.Equador, "EC");
			abbreviations.Add(Country.Guyana, "GY");
			abbreviations.Add(Country.Peru, "PE");
			abbreviations.Add(Country.Venezuela, "VE");
			abbreviations.Add(Country.Bahrain, "BH");
			abbreviations.Add(Country.BouvetIslands, "BV");
			abbreviations.Add(Country.BritishIndianOceanTerritory, "IO");
			#endregion
		}

		protected CountryInformation(Country Country) {
			this.myCountry = Country;
		}
		#endregion

		#region Properties
		public Country Country { get { return this.myCountry; } }
		public string FullName { get { return this.GetFullName(); } }
		public string Abbreviation { get { return this.GetAbbreviation(); } }
		#endregion

		#region Helper Methods
		public virtual Country FromCountryList(string Value) {
			if (string.IsNullOrEmpty(Value))
				return Country.Unknown;

			byte enumVal = 0;
			if (byte.TryParse(Value, out enumVal)) {
				try {
					return (Country)Enum.ToObject(typeof(Country), enumVal);
				} catch {
					return Country.Unknown;
				}
			} else {
				return GetCountryFromAbbreviation(Value);
			}
		}

		public static Country StaticFromCountryList(string Value) {
			if (string.IsNullOrEmpty(Value))
				return Country.Unknown;

			byte enumVal = 0;
			if (byte.TryParse(Value, out enumVal)) {
				try {
					return (Country)Enum.ToObject(typeof(Country), enumVal);
				} catch {
					return Country.Unknown;
				}
			} else {
				return StaticGetCountryFromAbbreviation(Value);
			}
		}

		public virtual System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> GetCountryList() {
			return StaticGetCountryList();
		}

		public static System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> StaticGetCountryList() {
			System.Collections.Generic.List<System.Web.UI.WebControls.ListItem> lst = new System.Collections.Generic.List<System.Web.UI.WebControls.ListItem>(210);
			foreach (Country c in fullNames.Keys)
				lst.Add(new System.Web.UI.WebControls.ListItem(fullNames[c], ((ushort)c).ToString()));
			return lst;
		}

		public virtual string GetFullName() {
			return StaticGetFullName(this.myCountry);
		}

		public virtual string GetFullName(Country country) {
			return StaticGetFullName(country);
		}

		public static string StaticGetFullName(Country country) {
			if (fullNames == null || !fullNames.ContainsKey(country))
				return string.Empty;
			return fullNames[country];
		}

		public virtual string GetAbbreviation() {
			return StaticGetAbbreviation(this.myCountry);
		}

		public virtual string GetAbbreviation(Country country) {
			return StaticGetAbbreviation(country);
		}

		public static string StaticGetAbbreviation(Country country) {
			if (abbreviations == null || !abbreviations.ContainsKey(country))
				return string.Empty;
			return abbreviations[country];
		}

		public virtual Country GetCountryFromAbbreviation(string Abbreviation) {
			return StaticGetCountryFromAbbreviation(Abbreviation);
		}

		public static Country StaticGetCountryFromAbbreviation(string Abbreviation) {
			switch(Abbreviation.ToUpper()) {
				case "DZ":
					return Country.Algeria;
				case "BI":
					return Country.Burundi;
				case "CF":
					return Country.CentralAfricanRepublic;
				case "CG":
					return Country.Congo;
				case "GQ":
					return Country.EquatorialGuinea;
				case "EG":
					return Country.Egypt;
				case "GH":
					return Country.Ghana;
				case "KE":
					return Country.Kenya;
				case "MG":
					return Country.Madagascar;
				case "YT":
					return Country.Mayotte;
				case "MW":
					return Country.Malawi;
				case "NG":
					return Country.Nigeria;
				case "ST":
					return Country.SaoTomeAndPrincipe;
				case "SO":
					return Country.Somalia;
				case "SZ":
					return Country.Swaziland;
				case "UG":
					return Country.Uganda;
				case "ZM":
					return Country.Zambia;
				case "BJ":
					return Country.Benin;
				case "CM":
					return Country.Cameroon;
				case "TD":
					return Country.Chad;
				case "CI":
					return Country.CoteDIvoire;
				case "ER":
					return Country.Eritrea;
				case "GA":
					return Country.Gabon;
				case "GN":
					return Country.Guinea;
				case "LS":
					return Country.Lesotho;
				case "ML":
					return Country.Mali;
				case "MA":
					return Country.Morocco;
				case "NA":
					return Country.Namibia;
				case "RE":
					return Country.Reunion;
				case "SN":
					return Country.Senegal;
				case "ZA":
					return Country.SouthAfrica;
				case "TZ":
					return Country.Tanzania;
				case "EH":
					return Country.WesternSahara;
				case "ZW":
					return Country.Zimbabwe;
				case "BF":
					return Country.BurkinaFaso;
				case "CV":
					return Country.CapeVerde;
				case "KM":
					return Country.Comoros;
				case "DJ":
					return Country.Djibouti;
				case "ET":
					return Country.Ethiopia;
				case "GM":
					return Country.Gambia;
				case "GW":
					return Country.GuineaBissau;
				case "LR":
					return Country.Liberia;
				case "MR":
					return Country.Mauritania;
				case "MZ":
					return Country.Mozambique;
				case "NE":
					return Country.Niger;
				case "SH":
					return Country.StHelena;
				case "SL":
					return Country.SierraLeone;
				case "SD":
					return Country.Sudan;
				case "TG":
					return Country.Togo;
				case "ZR":
					return Country.Zaire;
				case "AQ":
					return Country.Antarctica;
				case "AF":
					return Country.Afghanistan;
				case "BN":
					return Country.Brunei;
				case "HK":
					return Country.HongKong;
				case "JP":
					return Country.Japan;
				case "LA":
					return Country.Laos;
				case "MV":
					return Country.Maldives;
				case "PK":
					return Country.Pakistan;
				case "RU":
					return Country.Russia;
				case "LK":
					return Country.SriLanka;
				case "TH":
					return Country.Thailand;
				case "VN":
					return Country.Vietnam;
				case "BD":
					return Country.Bangladesh;
				case "KH":
					return Country.Cambodia;
				case "IN":
					return Country.India;
				case "KZ":
					return Country.Kazakhstan;
				case "MO":
					return Country.Macau;
				case "MN":
					return Country.Mongolia;
				case "PH":
					return Country.Philippines;
				case "SC":
					return Country.Seychelles;
				case "TW":
					return Country.Taiwan;
				case "TM":
					return Country.Turkmenistan;
				case "BT":
					return Country.Bhutan;
				case "CN":
					return Country.China;
				case "ID":
					return Country.Indonesia;
				case "KG":
					return Country.Kyrgyzstan;
				case "MY":
					return Country.Malaysia;
				case "NP":
					return Country.Nepal;
				case "KR":
					return Country.RepublicOfKorea;
				case "SG":
					return Country.Singapore;
				case "TJ":
					return Country.Tajikistan;
				case "UZ":
					return Country.Uzbekistan;
				case "AS":
					return Country.AmericanSamoa;
				case "FJ":
					return Country.Fiji;
				case "KI":
					return Country.Kiribati;
				case "NC":
					return Country.NewCaledonia;
				case "PW":
					return Country.Palau;
				case "SB":
					return Country.SolomonIslands;
				case "VU":
					return Country.Vanuatu;
				case "AU":
					return Country.Australia;
				case "PF":
					return Country.FrenchPolynesia;
				case "MH":
					return Country.MarshallIslands;
				case "NZ":
					return Country.NewZealand;
				case "PG":
					return Country.PapuaNewGuinea;
				case "TO":
					return Country.Tonga;
				case "FM":
					return Country.FederatedStatesOfMicronesia;
				case "GU":
					return Country.Guam;
				case "NR":
					return Country.Nauru;
				case "MP":
					return Country.NorthernMarianaIslands;
				case "PN":
					return Country.Pitcairn;
				case "TV":
					return Country.Tuvalu;
				case "AI":
					return Country.Anguilla;
				case "BS":
					return Country.Bahamas;
				case "VI":
					return Country.BritishVirginIslands;
				case "DO":
					return Country.DominicanRepublic;
				case "HT":
					return Country.Haiti;
				case "AN":
					return Country.NetherlandsAntilles;
				case "LC":
					return Country.StLucia;
				case "TC":
					return Country.TurksandCaicosIslands;
				case "AG":
					return Country.AntiquaandBarbuda;
				case "BB":
					return Country.Barbados;
				case "KY":
					return Country.CaymanIslands;
				case "GD":
					return Country.Grenada;
				case "JM":
					return Country.Jamaica;
				case "PR":
					return Country.PuertoRico;
				case "VC":
					return Country.StVincentAndTheGrenadines;
				case "AW":
					return Country.Aruba;
				case "BM":
					return Country.Bermuda;
				case "DM":
					return Country.Dominica;
				case "GP":
					return Country.Guadeloupe;
				case "MQ":
					return Country.Martinique;
				case "KN":
					return Country.StKittsandNevis;
				case "TT":
					return Country.TrinidadAndTobago;
				case "BZ":
					return Country.Belize;
				case "GT":
					return Country.Guatemala;
				case "PA":
					return Country.Panama;
				case "CR":
					return Country.CostaRica;
				case "HN":
					return Country.Honduras;
				case "SV":
					return Country.ElSalvador;
				case "NI":
					return Country.Nicaragua;
				case "AL":
					return Country.Albania;
				case "AT":
					return Country.Austria;
				case "BE":
					return Country.Belgium;
				case "CY":
					return Country.Cyprus;
				case "EE":
					return Country.Estonia;
				case "FR":
					return Country.France;
				case "GI":
					return Country.Gibraltar;
				case "HU":
					return Country.Hungary;
				case "IT":
					return Country.Italy;
				case "LT":
					return Country.Lithuania;
				case "FX":
					return Country.MetropolitanFrance;
				case "NO":
					return Country.Norway;
				case "RO":
					return Country.Romania;
				case "ES":
					return Country.Spain;
				case "CH":
					return Country.Switzerland;
				case "UA":
					return Country.Ukraine;
				case "YU":
					return Country.Yugoslavia;
				case "AD":
					return Country.Andorra;
				case "AZ":
					return Country.Azerbaijan;
				case "BG":
					return Country.Bulgaria;
				case "CZ":
					return Country.CzechRepublic;
				case "FO":
					return Country.FaroeIslands;
				case "GE":
					return Country.Georgia;
				case "GR":
					return Country.Greece;
				case "IS":
					return Country.Iceland;
				case "LV":
					return Country.Latvia;
				case "LU":
					return Country.Luxembourg;
				case "MD":
					return Country.Moldova;
				case "PL":
					return Country.Poland;
				case "SK":
					return Country.Slovakia;
				case "SJ":
					return Country.SvalbardAndJanMayenIslands;
				case "MK":
					return Country.TheFormerYugoslavRepublicOfMacedonia;
				case "GB":
					return Country.UnitedKingdom;
				case "AM":
					return Country.Armenia;
				case "BY":
					return Country.Belarus;
				case "HR":
					return Country.Croatia;
				case "DK":
					return Country.Denmark;
				case "FI":
					return Country.Finland;
				case "DE":
					return Country.Germany;
				case "GL":
					return Country.Greenland;
				case "IE":
					return Country.Ireland;
				case "LI":
					return Country.Liechtenstein;
				case "MT":
					return Country.Malta;
				case "NL":
					return Country.Netherlands;
				case "PT":
					return Country.Portugal;
				case "SI":
					return Country.Slovenia;
				case "SE":
					return Country.Sweden;
				case "TR":
					return Country.Turkey;
				case "VA":
					return Country.VaticanCity;
				case "IL":
					return Country.Israel;
				case "LB":
					return Country.Lebanon;
				case "SA":
					return Country.SaudiArabia;
				case "YE":
					return Country.Yemen;
				case "JO":
					return Country.Jordan;
				case "OM":
					return Country.Oman;
				case "SY":
					return Country.Syria;
				case "KW":
					return Country.Kuwait;
				case "QA":
					return Country.Qatar;
				case "AE":
					return Country.UnitedArabEmirates;
				case "CA":
					return Country.Canada;
				case "MX":
					return Country.Mexico;
				case "US":
					return Country.UnitedStates;
				case "AR":
					return Country.Argentina;
				case "CL":
					return Country.Chile;
				case "FK":
					return Country.FalklandIslands;
				case "PY":
					return Country.Paraguay;
				case "SR":
					return Country.Suriname;
				case "BO":
					return Country.Bolivia;
				case "CO":
					return Country.Colombia;
				case "GF":
					return Country.FrenchGuiana;
				case "UY":
					return Country.Uruguay;
				case "BR":
					return Country.Brazil;
				case "EC":
					return Country.Equador;
				case "GY":
					return Country.Guyana;
				case "PE":
					return Country.Peru;
				case "VE":
					return Country.Venezuela;
				case "BH":
					return Country.Bahrain;
				case "BV":
					return Country.BouvetIslands;
				case "IO":
					return Country.BritishIndianOceanTerritory;
			}
			return Country.Unknown;
		}
		#endregion
	}
}
