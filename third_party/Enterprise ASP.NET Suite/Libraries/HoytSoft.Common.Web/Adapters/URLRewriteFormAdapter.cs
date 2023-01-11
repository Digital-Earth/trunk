#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.Web.UI;
using System.Web.UI.Adapters;
using System.IO;

namespace HoytSoft.Common.Web.Adapters {
	public class URLRewriteFormAdapter : ControlAdapter {
		protected override void Render(System.Web.UI.HtmlTextWriter writer) {
			base.Render(new URLRewriteFormAdapterWriter(writer));
		}

		private class URLRewriteFormAdapterWriter : HtmlTextWriter {
			#region Constructors
			public URLRewriteFormAdapterWriter(TextWriter writer) : base(writer) {
			}

			public URLRewriteFormAdapterWriter(TextWriter writer, string tabString) : base(writer, tabString) {
			}
			#endregion

			public override void WriteAttribute(string name, string value, bool fEncode) {
				//Basically ignore the "action" attribute...
				if ("action".Equals(name, StringComparison.InvariantCultureIgnoreCase))
					return;
				base.WriteAttribute(name, value, fEncode);
			}
		}
	}
}
