#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2006, 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	internal class LicensesListener : AbstractApplicationListener {
		#region Variables
		private LicenseSettings settings = null;
		#endregion
		
		public override void AppStart() {
			this.settings = Settings.From<LicenseSettings>(Settings.Section.Licenses);
			IList<ILicense> lst = LicenseSettings.Licenses;
			if (lst != null && lst.Count > 0) {
				foreach(ILicense l in lst) {
					if (!l.LoadOnStartup) continue;
					if (string.IsNullOrEmpty(l.Code)) continue;
					if (l.TypeInAssembly == null) continue;

					Code.EvalVoidWithLoadedAssemblies(l.Language, l.Code, l.TypeInAssembly.Assembly.ManifestModule.FullyQualifiedName);
				}
			}
			
		}
	}
}
