using System;

namespace HoytSoft.Common.Web {
	public class User : System.Web.Security.MembershipUser, IUser {
		#region Variables
		private int userID;
		private string login, firstName, lastName, company, address1, address2, city, state, zip, phone, fax, email, web, brandingUniqueIDAsString;
		private DateTime created, lastLogin;
		private bool isContentEditor, isAdminManager, isImpersonating;
		private IUser origUser;
		private bool optionCanImpersonate, optionCanChangeLogin, optionCanChangePassword, optionCanEditPersonalInformation;
		#endregion

		#region Properties
		public int			ID									{ get { return this.userID;					} }

		public string		Login								{ get { return this.login;					} set { this.login = value;					} }
		public string		FirstName							{ get { return this.firstName;				} set { this.firstName = value;				} }
		public string		LastName							{ get { return this.lastName;				} set { this.lastName = value;				} }
		public string		Company								{ get { return this.company;				} set { this.company = value;				} }
		public string		Address1							{ get { return this.address1;				} set { this.address1 = value;				} }
		public string		Address2							{ get { return this.address2;				} set { this.address2 = value;				} }
		public string		City								{ get { return this.city;					} set { this.city = value;					} }
		public string		State								{ get { return this.state;					} set { this.state = value;					} }
		public string		Zip									{ get { return this.zip;					} set { this.zip = value;					} }
		public string		Phone								{ get { return this.phone;					} set { this.phone = value;					} }
		public string		Fax									{ get { return this.fax;					} set { this.fax = value;					} }
		public override		string Email						{ get { return this.email;					} set { this.email = value;					} }
		public string		Web									{ get { return this.web;					} set { this.web = value;					} }

		///<summary>Retrieves information about the original member if we're impersonating somebody.</summary>
		public IUser		OriginalUser						{ get { return this.origUser;				} set { this.origUser = value;				} }

		public DateTime		Created								{ get { return this.created;				} set { this.created = value;				} }
		public DateTime		LastLogin							{ get { return this.lastLogin;				} set { this.lastLogin = value;				} }

		public bool			IsAdministratorManager				{ get { return this.isAdminManager;			} set { this.isAdminManager = value;		} }
		public bool			IsImpersonating						{ get { return this.isImpersonating;		} set { this.isImpersonating = value;		} }
		public bool			IsContentEditor						{ get { return this.isContentEditor;		} set { this.isContentEditor = value;		} }

		public bool			OptionCanImpersonate				{ get { return this.optionCanImpersonate;	} set { this.optionCanImpersonate = value;	} }

		public bool			OptionCanChangeLogin				{ get { return this.optionCanChangeLogin; } set { this.optionCanChangeLogin = value; } }
		public bool			OptionCanChangePassword				{ get { return this.optionCanChangePassword; } set { this.optionCanChangePassword = value; } }
		public bool			OptionCanEditPersonalInformation	{ get { return this.optionCanEditPersonalInformation; } set { this.optionCanEditPersonalInformation = value; } }

		public bool IsAdministrator {
			get {
				return 
					   this.isAdminManager 
					|| this.isContentEditor
				;
			}
		}
		#endregion

		#region Constructor
		public User() : this(-1, "", "", "", "", "", "", "", "", "", "", "", "", "", DateTime.MinValue, DateTime.MinValue, false, false, false, false, false, false) {
		}

		public User(string UserID, string Login, string FirstName, string LastName) : this(int.Parse(UserID), Login, FirstName, LastName) {
		}

		public User(int UserID, string Login, string FirstName, string LastName) : this(UserID, Login, FirstName, LastName, "", "", "", "", "", "", "", "", "", "", DateTime.MinValue, DateTime.MinValue, false, false, false, false, false, false) {
		}

		public User(
			int UserID, 
			string Login, string FirstName, string LastName, string Company, string Address1, string Address2, 
			string City, string State, string Zip, string Phone, string Fax, string Email, string Web, 
			DateTime Created, DateTime LastLogin, 
			bool IsAdministratorManager, bool IsContentEditor, bool OptionCanImpersonate, 
			bool OptionCanChangeLogin, bool OptionCanChangePassword, bool OptionCanEditPersonalInformation
		) {
			this.userID								= UserID;

			this.login								= Login;
			this.firstName							= FirstName;
			this.lastName							= LastName;
			this.company							= Company;
			this.address1							= Address1;
			this.address2							= Address2;
			this.city								= City;
			this.state								= State;
			this.zip								= Zip;
			this.phone								= Phone;
			this.fax								= Fax;
			this.email								= Email;
			this.web								= Web;

			this.created							= Created;
			this.lastLogin							= LastLogin;

			this.isImpersonating					= false;

			this.origUser							= null;

			this.isAdminManager						= IsAdministratorManager;
			this.isContentEditor					= IsContentEditor;

			this.optionCanImpersonate				= OptionCanImpersonate;

			this.optionCanChangeLogin				= OptionCanChangeLogin;
			this.optionCanChangePassword			= OptionCanChangePassword;
			this.optionCanEditPersonalInformation	= OptionCanEditPersonalInformation;
		}
		#endregion

		#region Public Methods
		#endregion

		#region MembershipUser
		public override DateTime CreationDate { get { return this.Created; } }
		public override bool IsApproved { get { return true; /* Approved if this obj exists at all */ } set { base.IsApproved = true; } }
		public override string GetPassword() { return string.Empty; }
		public override string Comment { get { return string.Empty; } set { } }
		public override bool IsLockedOut { get { return false; } }
		public override DateTime LastActivityDate { get { return this.LastLogin; } set { this.LastLogin = value; } }
		public override DateTime LastLockoutDate { get { return DateTime.MinValue; } }
		public override DateTime LastLoginDate { get { return this.LastLogin; } set { this.LastLogin = value; } }
		public override DateTime LastPasswordChangedDate { get { return this.Created; } }
		public override string PasswordQuestion { get { return string.Empty; } }
		public override string ProviderName { get { return typeof(HoytSoft.Common.Web.Providers.MembershipProvider).FullName; } }
		public override object ProviderUserKey { get { return this.ID; } }
		public override string UserName { get { return this.Login; } }
		#endregion
	}
}
