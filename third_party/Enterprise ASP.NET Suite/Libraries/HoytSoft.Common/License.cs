#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2006, 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace HoytSoft.Common {
	#region Interfaces
	public interface ILicense {
		string Name { get; }
		string Key { get; }
		bool LoadOnStartup { get; }
		Type TypeInAssembly { get; }
		string Language { get; }
		string Code { get; }
	}
	#endregion

	public class License : ILicense {
		#region Variables
		private string key;
		private string name;
		private string language;
		private string code;
		private Type typeInAssembly;
		private bool loadOnStartup;
		#endregion

		#region Constructors
		public License(string Name, string Key, bool LoadOnStartup, Type TypeInAssembly, string Language, string Code) {
			this.key = Key;
			this.name = Name;
			this.language = Language;
			this.code = Code;
			this.loadOnStartup = LoadOnStartup;

			//if (!string.IsNullOrEmpty(Assembly)) {
			//	FileInfo fi = new FileInfo(Assembly);
			//	this.assembly = fi.FullName;
			//} else this.assembly = null;
			this.typeInAssembly = TypeInAssembly;
		}
		#endregion

		#region ILicense Methods
		public string Name {
			get { return this.name; }
		}

		public string Key {
			get { return this.key; }
		}

		public bool LoadOnStartup {
			get { return this.loadOnStartup; }
		}

		public Type TypeInAssembly {
			get { return this.typeInAssembly; }
		}

		public string Language {
			get { return this.language; }
		}

		public string Code {
			get { return this.code; }
		}
		#endregion
	}
}
