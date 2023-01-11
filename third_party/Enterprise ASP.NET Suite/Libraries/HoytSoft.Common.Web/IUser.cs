#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HoytSoft.Common.Web {
	public interface IUser {
		int ID { get; }
		string Login { get; set; }
		string FirstName { get; set; }
		string LastName { get; set; }
		string Company { get; set; }
		string Address1 { get; set; }
		string Address2 { get; set; }
		string City { get; set; }
		string State { get; set; }
		string Zip { get; set; }
		string Phone { get; set; }
		string Fax { get; set; }
		string Email { get; set; }
		string Web { get; set; }
		IUser OriginalUser { get; set; }
		bool IsImpersonating { get; set; }
		bool IsAdministrator { get; }
		bool IsAdministratorManager { get; }
		bool IsContentEditor { get; }
		bool OptionCanImpersonate { get; set; }
		bool OptionCanChangeLogin { get; set; }
		bool OptionCanChangePassword { get; set; }
		bool OptionCanEditPersonalInformation { get; set; }
	}
}
