#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Globalization;
using HoytSoft.Common.Configuration;

namespace HoytSoft.Common.Web {

	public class LocalizedMasterPage : HoytSoft.Common.Web.MasterPage {

		public SupportedCulture FindCurrentSupportedCulture() {
			if (!(this.Page is LocalizedPage))
				return null;
			return (this.Page as LocalizedPage).FindCurrentSupportedCulture();
		}

		public bool SaveCulture(string CultureName) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(CultureName);
		}

		public bool SaveCulture(int LCID) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(LCID);
		}

		public bool SaveCulture(CultureInfo Culture) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(Culture);
		}

		public bool SaveCulture(string CultureName, bool SetCurrentCulture) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(CultureName, SetCurrentCulture);
		}

		public bool SaveCulture(int LCID, bool SetCurrentCulture) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(LCID, SetCurrentCulture);
		}

		public bool SaveCulture(CultureInfo Culture, bool SetCurrentCulture) {
			if (!(this.Page is LocalizedPage))
				return false;
			return (this.Page as LocalizedPage).SaveCulture(Culture, SetCurrentCulture);
		}

	} //class

} //namespace
