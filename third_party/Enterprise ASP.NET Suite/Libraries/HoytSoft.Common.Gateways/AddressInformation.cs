#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Text;

namespace HoytSoft.Common.Gateways {
	public abstract class AddressInformation {
		#region Variables
		private string name, company, address1, address2, city, state, zip, phone, fax, email, addressNumber;
		private USState myState;
		private Country country;
		#endregion

		#region Constructors
		protected AddressInformation(AddressInformation from) : this(from.name, from.company, from.address1, from.address2, from.city, from.state, from.zip, from.country, from.phone, from.fax, from.email) {
		}

		protected AddressInformation(string Name, string Company, string Address1, string Address2, string City, USState State, string Zip, string Phone, string Fax, string Email)
			: this(Name, Company, Address1, Address2, City, USStateInformation.StaticGetAbbreviation(State), Zip, Country.UnitedStates, Phone, Fax, Email) {
		}

		protected AddressInformation(string Name, string Company, string Address1, string Address2, string City, string State, string Zip, Country Country, string Phone, string Fax, string Email) {
			this.name			= Name;
			this.company		= Company;
			this.address1		= Address1;
			this.address2		= Address2;
			this.city			= City;
			this.state			= State;
			this.zip			= Zip;
			this.country		= Country;
			this.phone			= Phone;
			this.fax			= Fax;
			this.email			= Email;
			this.addressNumber	= addressNumberFromStreet(ref Address1);
			this.myState		= USState.Unknown;

			//Figure out the state enum based off of the abbreviation...
			if (Country == Country.UnitedStates && !string.IsNullOrEmpty(State) && State.Trim().Length == 2)
				this.myState = USStateInformation.StaticGetUSStateFromAbbreviation(State.Trim());
		}
		#endregion

		#region Properties
		public string Name { get { return name; } }
		public string Company { get { return company; } }
		public string Address1 { get { return address1; } }
		public string Address2 { get { return address2; } }
		public string City { get { return city; } }
		public string State { get { return state; } }
		public string Zip { get { return zip; } }
		public USState USState { get { return myState; } }
		public Country Country { get { return country; } }
		public string StateFullName { get { return this.GetStateFullName(); } }
		public string StateAbbreviation { get { return this.GetStateAbbreviation(); } }
		public string CountryFullName { get { return this.GetCountryFullName(); } }
		public string CountryAbbreviation { get { return this.GetCountryAbbreviation(); } }
		public string Phone { get { return phone; } }
		public string Fax { get { return fax; } }
		public string Email { get { return email; } }
		public string AddressNumber { get { return addressNumber; } }
		#endregion

		#region Helper Methods
		public string GetStateFullName() {
			if (this.country == Country.UnitedStates && this.myState != USState.Unknown)
				return USStateInformation.StaticGetFullName(this.myState);
			return this.state;
		}

		public string GetStateAbbreviation() {
			if (this.country == Country.UnitedStates && this.myState != USState.Unknown)
				return USStateInformation.StaticGetAbbreviation(this.myState);
			return this.state;
		}

		public string GetCountryFullName() {
			return CountryInformation.StaticGetFullName(this.country);
		}

		public string GetCountryAbbreviation() {
			return CountryInformation.StaticGetAbbreviation(this.country);
		}

		protected static string addressNumberFromStreet(ref string Address) {
			string addressNum = string.Empty;
			string trimmed = Address.Trim().ToLower();
			string tmp = string.Empty;
			int result;
			for (int i = 1; i < trimmed.Length; i++) {
				if (int.TryParse((tmp = trimmed.Substring(0, i)), out result))
					addressNum = tmp;
				else
					break;
			}
			return addressNum.Trim();
		}

		///<summary>Strips the text of all non-numeric characters</summary>
		protected static string numbersOnly(string Text) {
			if (string.IsNullOrEmpty(Text)) return Text;
			StringBuilder sb = new StringBuilder(Text.Length);
			for (int i = 0; i < Text.Length; i++)
				if (char.IsDigit(Text[i]))
					sb.Append(Text[i]);
			return sb.ToString();
		}
		#endregion

		public abstract bool IsRequired(AddressField Field);
		public abstract bool IsRequiredAVS(AddressField Field);
	}
}
