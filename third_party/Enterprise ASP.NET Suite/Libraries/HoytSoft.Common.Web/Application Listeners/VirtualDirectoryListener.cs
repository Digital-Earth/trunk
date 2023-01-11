#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2006, 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.IO;
using System.Collections.Generic;
using System.Web;
using HoytSoft.Common.Web;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	internal class VirtualDirectoryListener : AbstractApplicationListener {
		#region Variables
		private VirtualDirectorySettings vdSettings = null;
		#endregion

		public override void AppStart() {
			this.vdSettings = Settings.From<VirtualDirectorySettings>(Settings.Section.VirtualDirectory);
		}

		public override void AppBeginRequest() {
			if (this.vdSettings == null || HttpContext.Current == null)
				return;

			//System.Diagnostics.Debug.WriteLine(HttpContext.Current.Request.RawUrl);
			string url = null, path = null;
			int index = 0;
			if ((url = this.vdSettings.Find(HttpContext.Current.Request.Path, ref index, ref path)) == null || path == null)
				return;

			string modRequestFile = path + HttpContext.Current.Request.Path.Substring(index + url.Length).Replace('/', System.IO.Path.DirectorySeparatorChar);

			try {
				Utils.SendFile(modRequestFile, HttpContext.Current.Response);
			} catch (FileNotFoundException) {
				HttpContext.Current.Response.End();
			}
		}

		#region Helper Classes
		private class RequestFile {
			public byte[] Data;
			public string ContentType;
			public string FileName;

			public RequestFile(string FileName, string ContentType, byte[] Data) {
				this.FileName = FileName;
				this.ContentType = ContentType;
				this.Data = Data;
			}
		}
		#endregion
	}
}
