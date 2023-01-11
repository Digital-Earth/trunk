#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Gateways {
	public abstract class USStateInformation {
		#region Variables
		private static System.Collections.Generic.Dictionary<USState, string> fullNames;
		private static System.Collections.Generic.Dictionary<USState, string> abbreviations;
		private USState myState;
		#endregion

		#region Constructors
		static USStateInformation() {
			#region Add Full Names
			fullNames = new System.Collections.Generic.Dictionary<USState, string>(51);
			fullNames.Add(USState.Alabama, "Alabama");
			fullNames.Add(USState.Arizona, "Arizona");
			fullNames.Add(USState.Alaska, "Alaska");
			fullNames.Add(USState.Arkansas, "Arkansas");
			fullNames.Add(USState.California, "California");
			fullNames.Add(USState.Connecticut, "Connecticut");
			fullNames.Add(USState.Colorado, "Colorado");
			fullNames.Add(USState.Delaware, "Delaware");
			fullNames.Add(USState.DistrictOfColumbia, "District of Columbia");
			fullNames.Add(USState.Florida, "Florida");
			fullNames.Add(USState.Georgia, "Georgia");
			fullNames.Add(USState.Hawaii, "Hawaii");
			fullNames.Add(USState.Idaho, "Idaho");
			fullNames.Add(USState.Indiana, "Indiana");
			fullNames.Add(USState.Illinois, "Illinois");
			fullNames.Add(USState.Iowa, "Iowa");
			fullNames.Add(USState.Kansas, "Kansas");
			fullNames.Add(USState.Kentucky, "Kentucky");
			fullNames.Add(USState.Louisiana, "Louisiana");
			fullNames.Add(USState.Maine, "Maine");
			fullNames.Add(USState.Massachusetts, "Massachusetts");
			fullNames.Add(USState.Minnesota, "Minnesota");
			fullNames.Add(USState.Missouri, "Missouri");
			fullNames.Add(USState.Maryland, "Maryland");
			fullNames.Add(USState.Michigan, "Michigan");
			fullNames.Add(USState.Mississippi, "Mississippi");
			fullNames.Add(USState.Montana, "Montana");
			fullNames.Add(USState.Nebraska, "Nebraska");
			fullNames.Add(USState.NewHampshire, "New Hampshire");
			fullNames.Add(USState.NewMexico, "New Mexico");
			fullNames.Add(USState.NorthCarolina, "North Carolina");
			fullNames.Add(USState.Nevada, "Nevada");
			fullNames.Add(USState.NewJersey, "New Jersey");
			fullNames.Add(USState.NewYork, "New York");
			fullNames.Add(USState.NorthDakota, "North Dakota");
			fullNames.Add(USState.Ohio, "Ohio");
			fullNames.Add(USState.Oregon, "Oregon");
			fullNames.Add(USState.Oklahoma, "Oklahoma");
			fullNames.Add(USState.Pennsylvania, "Pennsylvania");
			fullNames.Add(USState.RhodeIsland, "Rhode Island");
			fullNames.Add(USState.SouthCarolina, "South Carolina");
			fullNames.Add(USState.SouthDakota, "South Dakota");
			fullNames.Add(USState.Tenessee, "Tenessee");
			fullNames.Add(USState.Texas, "Texas");
			fullNames.Add(USState.Utah, "Utah");
			fullNames.Add(USState.Vermont, "Vermont");
			fullNames.Add(USState.Virginia, "Virginia");
			fullNames.Add(USState.Washington, "Washington");
			fullNames.Add(USState.WestVirginia, "West Virginia");
			fullNames.Add(USState.Wisconsin, "Wisconsin");
			fullNames.Add(USState.Wyoming, "Wyoming");
			#endregion

			#region Add Abbreviations
			abbreviations = new System.Collections.Generic.Dictionary<USState, string>(51);
			abbreviations.Add(USState.Alabama, "AL");
			abbreviations.Add(USState.Arizona, "AZ");
			abbreviations.Add(USState.Alaska, "AK");
			abbreviations.Add(USState.Arkansas, "AR");
			abbreviations.Add(USState.California, "CA");
			abbreviations.Add(USState.Connecticut, "CT");
			abbreviations.Add(USState.Colorado, "CO");
			abbreviations.Add(USState.Delaware, "DE");
			abbreviations.Add(USState.DistrictOfColumbia, "DC");
			abbreviations.Add(USState.Florida, "FL");
			abbreviations.Add(USState.Georgia, "GA");
			abbreviations.Add(USState.Hawaii, "HI");
			abbreviations.Add(USState.Idaho, "ID");
			abbreviations.Add(USState.Indiana, "IN");
			abbreviations.Add(USState.Illinois, "IL");
			abbreviations.Add(USState.Iowa, "IA");
			abbreviations.Add(USState.Kansas, "KS");
			abbreviations.Add(USState.Kentucky, "KY");
			abbreviations.Add(USState.Louisiana, "LA");
			abbreviations.Add(USState.Maine, "ME");
			abbreviations.Add(USState.Massachusetts, "MA");
			abbreviations.Add(USState.Minnesota, "MN");
			abbreviations.Add(USState.Missouri, "MO");
			abbreviations.Add(USState.Maryland, "MD");
			abbreviations.Add(USState.Michigan, "MI");
			abbreviations.Add(USState.Mississippi, "MS");
			abbreviations.Add(USState.Montana, "MT");
			abbreviations.Add(USState.Nebraska, "NE");
			abbreviations.Add(USState.NewHampshire, "NH");
			abbreviations.Add(USState.NewMexico, "NM");
			abbreviations.Add(USState.NorthCarolina, "NC");
			abbreviations.Add(USState.Nevada, "NV");
			abbreviations.Add(USState.NewJersey, "NJ");
			abbreviations.Add(USState.NewYork, "NY");
			abbreviations.Add(USState.NorthDakota, "ND");
			abbreviations.Add(USState.Ohio, "OH");
			abbreviations.Add(USState.Oregon, "OR");
			abbreviations.Add(USState.Oklahoma, "OK");
			abbreviations.Add(USState.Pennsylvania, "PA");
			abbreviations.Add(USState.RhodeIsland, "RI");
			abbreviations.Add(USState.SouthCarolina, "SC");
			abbreviations.Add(USState.SouthDakota, "SD");
			abbreviations.Add(USState.Tenessee, "TN");
			abbreviations.Add(USState.Texas, "TX");
			abbreviations.Add(USState.Utah, "UT");
			abbreviations.Add(USState.Vermont, "VT");
			abbreviations.Add(USState.Virginia, "VA");
			abbreviations.Add(USState.Washington, "WA");
			abbreviations.Add(USState.WestVirginia, "WV");
			abbreviations.Add(USState.Wisconsin, "WI");
			abbreviations.Add(USState.Wyoming, "WY");
			#endregion
		}

		protected USStateInformation(USState USState) {
			this.myState = USState;
		}
		#endregion

		#region Properties
		public USState USState { get { return this.myState; } }
		public string FullName { get { return this.GetFullName(); } }
		public string Abbreviation { get { return this.GetAbbreviation(); } }
		#endregion

		#region Helper Methods
		public virtual USState FromUSStateList(string Value) {
			if (string.IsNullOrEmpty(Value))
				return USState.Unknown;

			byte enumVal = 0;
			if (byte.TryParse(Value, out enumVal)) {
				try {
					return (USState)Enum.ToObject(typeof(USState), enumVal);
				} catch {
					return USState.Unknown;
				}
			} else {
				return GetUSStateFromAbbreviation(Value);
			}
		}

		public static USState StaticFromUSStateList(string Value) {
			if (string.IsNullOrEmpty(Value))
				return USState.Unknown;

			byte enumVal = 0;
			if (byte.TryParse(Value, out enumVal)) {
				try {
					return (USState)Enum.ToObject(typeof(USState), enumVal);
				} catch {
					return USState.Unknown;
				}
			} else {
				return StaticGetUSStateFromAbbreviation(Value);
			}
		}

		public virtual System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> GetUSStateList() {
			return StaticGetUSStateList();
		}

		public static System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> StaticGetUSStateList() {
			System.Collections.Generic.List<System.Web.UI.WebControls.ListItem> lst = new System.Collections.Generic.List<System.Web.UI.WebControls.ListItem>(210);
			foreach (USState s in fullNames.Keys)
				lst.Add(new System.Web.UI.WebControls.ListItem(fullNames[s], ((ushort)s).ToString()));
			return lst;
		}

		public virtual System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> GetUSStateListWithAbbreviations() {
			return StaticGetUSStateListWithAbbreviations();
		}

		public static System.Collections.Generic.IList<System.Web.UI.WebControls.ListItem> StaticGetUSStateListWithAbbreviations() {
			System.Collections.Generic.List<System.Web.UI.WebControls.ListItem> lst = new System.Collections.Generic.List<System.Web.UI.WebControls.ListItem>(210);
			foreach (USState s in fullNames.Keys)
				lst.Add(new System.Web.UI.WebControls.ListItem(fullNames[s], abbreviations[s]));
			return lst;
		}

		public virtual string GetFullName() {
			return StaticGetFullName(this.myState);
		}

		public virtual string GetFullName(USState state) {
			return StaticGetFullName(state);
		}

		public static string StaticGetFullName(USState state) {
			if (fullNames == null || !fullNames.ContainsKey(state))
				return string.Empty;
			return fullNames[state];
		}

		public virtual string GetAbbreviation() {
			return StaticGetAbbreviation(this.myState);
		}

		public virtual string GetAbbreviation(USState state) {
			return StaticGetAbbreviation(state);
		}

		public static string StaticGetAbbreviation(USState state) {
			if (abbreviations == null || !abbreviations.ContainsKey(state))
				return string.Empty;
			return abbreviations[state];
		}

		public virtual USState GetUSStateFromAbbreviation(string Abbreviation) {
			return StaticGetUSStateFromAbbreviation(Abbreviation);
		}

		public static USState StaticGetUSStateFromAbbreviation(string Abbreviation) {
			switch (Abbreviation.ToUpper()) {
				case "AL":
					return USState.Alabama;
				case "AZ":
					return USState.Arizona;
				case "AK":
					return USState.Alaska;
				case "AR":
					return USState.Arkansas;
				case "CA":
					return USState.California;
				case "CT":
					return USState.Connecticut;
				case "CO":
					return USState.Colorado;
				case "DE":
					return USState.Delaware;
				case "DC":
					return USState.DistrictOfColumbia;
				case "FL":
					return USState.Florida;
				case "GA":
					return USState.Georgia;
				case "HI":
					return USState.Hawaii;
				case "ID":
					return USState.Idaho;
				case "IN":
					return USState.Indiana;
				case "IL":
					return USState.Illinois;
				case "IA":
					return USState.Iowa;
				case "KS":
					return USState.Kansas;
				case "KY":
					return USState.Kentucky;
				case "LA":
					return USState.Louisiana;
				case "ME":
					return USState.Maine;
				case "MA":
					return USState.Massachusetts;
				case "MN":
					return USState.Minnesota;
				case "MO":
					return USState.Missouri;
				case "MD":
					return USState.Maryland;
				case "MI":
					return USState.Michigan;
				case "MS":
					return USState.Mississippi;
				case "MT":
					return USState.Montana;
				case "NE":
					return USState.Nebraska;
				case "NH":
					return USState.NewHampshire;
				case "NM":
					return USState.NewMexico;
				case "NC":
					return USState.NorthCarolina;
				case "NV":
					return USState.Nevada;
				case "NJ":
					return USState.NewJersey;
				case "NY":
					return USState.NewYork;
				case "ND":
					return USState.NorthDakota;
				case "OH":
					return USState.Ohio;
				case "OR":
					return USState.Oregon;
				case "OK":
					return USState.Oklahoma;
				case "PA":
					return USState.Pennsylvania;
				case "RI":
					return USState.RhodeIsland;
				case "SC":
					return USState.SouthCarolina;
				case "SD":
					return USState.SouthDakota;
				case "TN":
					return USState.Tenessee;
				case "TX":
					return USState.Texas;
				case "UT":
					return USState.Utah;
				case "VT":
					return USState.Vermont;
				case "VA":
					return USState.Virginia;
				case "WA":
					return USState.Washington;
				case "WV":
					return USState.WestVirginia;
				case "WI":
					return USState.Wisconsin;
				case "WY":
					return USState.Wyoming;
			}
			return USState.Unknown;
		}
		#endregion
	}
}
