#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace HoytSoft.Common.Web {
	public class URLRewriteRegularExpressionRule {
		#region Variables
		private string from;
		private string to;
		private Regex regExp;
		#endregion

		#region Constructors
		public URLRewriteRegularExpressionRule(string From, string To, Regex RegExp) {
			this.from = From;
			this.to = To;
			this.regExp = RegExp;
		}
		#endregion

		#region Properties
		public string From {
			get { return this.from; }
		}

		public string To {
			get { return this.to; }
		}

		public Regex RegularExpression {
			get { return this.regExp; }
		}
		#endregion
	}
}
