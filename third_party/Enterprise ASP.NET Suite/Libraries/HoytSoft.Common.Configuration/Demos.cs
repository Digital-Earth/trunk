using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Configuration {
	public class Demos {
		#region Constants
		public const string
			SETTINGS_LOCK_EXTENSION = @".lck",
			SETTINGS_LOCK_CONTENTS = "demo",
			SETTINGS_LOCK_EMPTY = "no demo"
		;
		#endregion

		#region Properties
		public string PresentersLockFolder { 
			get {
				DemosSettings src = Settings.From<DemosSettings>(Settings.Section.Demos);
				if (src == null)
					return string.Empty;
				return src.PresentersLockFolder; 
			} 
		}
		#endregion

		#region Public Static Methods
		public static User UserFromSessionID(string SessionID) {
			DemosSettings src = Settings.From<DemosSettings>(Settings.Section.Demos);
			if (src == null)
				return null;
			foreach (User u in src.Users.Values)
				if (u.SessionID.Equals(SessionID, StringComparison.InvariantCulture))
					return u;
			return null;
		}

		public static string FileLock(string Login) {
			DemosSettings src = Settings.From<DemosSettings>(Settings.Section.Demos);
			if (src == null)
				return string.Empty;

			if (!src.Users.ContainsKey(Login)) return Login + SETTINGS_LOCK_EXTENSION;
			if (System.Web.HttpContext.Current == null) return src.Users[Login].SessionID + SETTINGS_LOCK_EXTENSION;
			else {
				string dir = System.Web.HttpContext.Current.Server.MapPath(src.PresentersLockFolder);
				if (!System.IO.Directory.Exists(dir))
					System.IO.Directory.CreateDirectory(dir).Attributes = System.IO.FileAttributes.Hidden;
				return dir + src.Users[Login].SessionID + SETTINGS_LOCK_EXTENSION;
			}
		}

		public static bool WriteLock(string Login) {
			//Write file to disk...
			try {
				System.IO.File.WriteAllText(FileLock(Login), SETTINGS_LOCK_CONTENTS);
				return true;
			} catch {
				return false;
			}
		}

		public static bool RemoveLock(string Login) {
			//Write file to disk...
			try {
				System.IO.File.WriteAllText(FileLock(Login), SETTINGS_LOCK_EMPTY);
				return true;
			} catch {
				return false;
			}
		}

		public static bool SessionStarted(string Login) {
			//Should check the file system to see if the file exists, and if so, return true...
			try {
				string file = FileLock(Login);
				return System.IO.File.Exists(file) && System.IO.File.ReadAllText(file) == SETTINGS_LOCK_CONTENTS;
			} catch {
				return false;
			}
		}

		public static uint OpenSessionCount() {
			List<string> tmp = null;
			return OpenSessionCount(ref tmp);
		}

		public static uint OpenSessionCount(ref List<string> Names) {
			DemosSettings src = Settings.From<DemosSettings>(Settings.Section.Demos);
			
			if (System.Web.HttpContext.Current == null) return 0;
			else {
				if (src == null)
					return 0;

				uint ret = 0;
				string dir = System.Web.HttpContext.Current.Server.MapPath(src.PresentersLockFolder);
				if (!System.IO.Directory.Exists(dir))
					System.IO.Directory.CreateDirectory(dir).Attributes = System.IO.FileAttributes.Hidden;

				User u = null;
				System.IO.FileInfo fi = null;
				string[] files = System.IO.Directory.GetFiles(dir, "*" + SETTINGS_LOCK_EXTENSION, System.IO.SearchOption.TopDirectoryOnly);
				foreach (string file in files)
					if (System.IO.File.Exists(file) && System.IO.File.ReadAllText(file) == SETTINGS_LOCK_CONTENTS) {
						//Add in the list of names if we want it...
						if (Names != null) {
							fi = new System.IO.FileInfo(file);
							if ((u = UserFromSessionID(fi.Name.Remove(fi.Name.Length - fi.Extension.Length))) != null)
								Names.Add(u.Name);
						}
						++ret;
					}
				return ret;
			}
			//return 0;
		}

		public static bool Authenticate(string Login, string Password) {
			string tmp = "";
			return Authenticate(Login, Password, ref tmp, ref tmp);
		}
		public static bool Authenticate(string Login, string Password, ref string Name, ref string SessionID) {
			DemosSettings src = Settings.From<DemosSettings>(Settings.Section.Demos);

			if (src == null || src.Users == null || src.Users.Count <= 0) return false;
			if (!src.Users.ContainsKey(Login)) return false;
			Name = src.Users[Login].Name;
			SessionID = src.Users[Login].SessionID;
			return (
				   src.Users[Login].Login.Equals(Login, StringComparison.InvariantCultureIgnoreCase)
				&& src.Users[Login].Password.Equals(Password, StringComparison.InvariantCulture)
			);
		}
		#endregion

		#region Helper Classes
		public class User {
			private string name, login, password, sessionID;

			public User(string Name, string Login, string Password, string SessionID) {
				this.name = Name;
				this.login = Login;
				this.password = Password;
				this.sessionID = SessionID;
			}

			public string Name { get { return this.name; } }
			public string Login { get { return this.login; } }
			public string Password { get { return this.password; } }
			public string SessionID { get { return this.sessionID; } }
		}
		#endregion
	}
}
