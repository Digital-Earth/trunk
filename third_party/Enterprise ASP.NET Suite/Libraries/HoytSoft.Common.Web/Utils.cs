#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.IO;
using System.Web;
using System.Text;
using System.Web.UI;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Web.Configuration;
using System.Diagnostics;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Web {
	public class Utils {
		#region Constants 
		private const string
			VAR_BASE_URL			= "{BaseUrl}", 
			VAR_FIRST_NAME			= "{FirstName}",
			VAR_LAST_NAME			= "{LastName}",
			VAR_COMPANY_NAME		= "{CompanyName}",
			VAR_EMAIL				= "{Email}",
			VAR_PHONE				= "{Phone}",
			VAR_FAX					= "{Fax}",
			VAR_ADDRESS1			= "{Address1}",
			VAR_ADDRESS2			= "{Address2}",
			VAR_CITY				= "{City}",
			VAR_STATE				= "{State}",
			VAR_ZIP					= "{Zip}", 
			VAR_HEAR				= "{Hear}",
			VAR_SUBSCRIBE			= "{Subscribe}", 
			VAR_LOGIN				= "{Login}",
			VAR_PASSWORD			= "{Password}", 
			VAR_USER_FIRST_NAME		= "{UserFirstName}", 
			VAR_USER_LAST_NAME		= "{UserLastName}", 
			VAR_USER_EMAIL			= "{UserEmail}", 
			VAR_ID					= "{ID}"
		;
		#endregion

		#region Constructors
		static Utils() {
			Utils.loadMimeTypes();
		}
		#endregion

		#region Sending Files
		private const int FILESEND_BUFFER_SIZE = 1024;
		private static readonly Dictionary<string, string> mime = new Dictionary<string, string>();

		private static void loadMimeTypes() {
			//Add in the mime types...
			mime.Add(".ai", @"application/postscript");
			mime.Add(".aif", @"audio/x-aiff");
			mime.Add(".aifc", @"audio/x-aiff");
			mime.Add(".aiff", @"audio/x-aiff");
			mime.Add(".asc", @"text/plain");
			mime.Add(".au", @"audio/basic");
			mime.Add(".avi", @"video/x-msvideo");
			mime.Add(".bcpio", @"application/x-bcpio");
			mime.Add(".bin", @"application/octet-stream");
			mime.Add(".c", @"text/plain");
			mime.Add(".cc", @"text/plain");
			mime.Add(".ccad", @"application/clariscad");
			mime.Add(".cdf", @"application/x-netcdf");
			mime.Add(".class", @"application/octet-stream");
			mime.Add(".cpio", @"application/x-cpio");
			mime.Add(".cpt", @"application/mac-compactpro");
			mime.Add(".csh", @"application/x-csh");
			mime.Add(".css", @"text/css");
			mime.Add(".dcr", @"application/x-director");
			mime.Add(".dir", @"application/x-director");
			mime.Add(".dms", @"application/octet-stream");
			mime.Add(".doc", @"application/msword");
			mime.Add(".drw", @"application/drafting");
			mime.Add(".dvi", @"application/x-dvi");
			mime.Add(".dwg", @"application/acad");
			mime.Add(".dxf", @"application/dxf");
			mime.Add(".dxr", @"application/x-director");
			mime.Add(".eps", @"application/postscript");
			mime.Add(".etx", @"text/x-setext");
			mime.Add(".exe", @"application/octet-stream");
			mime.Add(".ez", @"application/andrew-inset");
			mime.Add(".f", @"text/plain");
			mime.Add(".f90", @"text/plain");
			mime.Add(".fli", @"video/x-fli");
			mime.Add(".gif", @"image/gif");
			mime.Add(".gtar", @"application/x-gtar");
			mime.Add(".gz", @"application/x-gzip");
			mime.Add(".h", @"text/plain");
			mime.Add(".hdf", @"application/x-hdf");
			mime.Add(".hh", @"text/plain");
			mime.Add(".hqx", @"application/mac-binhex40");
			mime.Add(".htm", @"text/html");
			mime.Add(".html", @"text/html");
			mime.Add(".ice", @"x-conference/x-cooltalk");
			mime.Add(".ief", @"image/ief");
			mime.Add(".iges", @"model/iges");
			mime.Add(".igs", @"model/iges");
			mime.Add(".ips", @"application/x-ipscript");
			mime.Add(".ipx", @"application/x-ipix");
			mime.Add(".jpe", @"image/jpeg");
			mime.Add(".jpeg", @"image/jpeg");
			mime.Add(".jpg", @"image/jpeg");
			mime.Add(".js", @"application/x-javascript");
			mime.Add(".kar", @"audio/midi");
			mime.Add(".latex", @"application/x-latex");
			mime.Add(".lha", @"application/octet-stream");
			mime.Add(".lsp", @"application/x-lisp");
			mime.Add(".lzh", @"application/octet-stream");
			mime.Add(".m", @"text/plain");
			mime.Add(".man", @"application/x-troff-man");
			mime.Add(".me", @"application/x-troff-me");
			mime.Add(".mesh", @"model/mesh");
			mime.Add(".mid", @"audio/midi");
			mime.Add(".midi", @"audio/midi");
			mime.Add(".mif", @"application/vnd.mif");
			mime.Add(".mime", @"www/mime");
			mime.Add(".mov", @"video/quicktime");
			mime.Add(".movie", @"video/x-sgi-movie");
			mime.Add(".mp2", @"audio/mpeg");
			mime.Add(".mp3", @"audio/mpeg");
			mime.Add(".mpe", @"video/mpeg");
			mime.Add(".mpeg", @"video/mpeg");
			mime.Add(".mpg", @"video/mpeg");
			mime.Add(".mpga", @"audio/mpeg");
			mime.Add(".ms", @"application/x-troff-ms");
			mime.Add(".msh", @"model/mesh");
			mime.Add(".nc", @"application/x-netcdf");
			mime.Add(".oda", @"application/oda");
			mime.Add(".pbm", @"image/x-portable-bitmap");
			mime.Add(".pdb", @"chemical/x-pdb");
			mime.Add(".pdf", @"application/pdf");
			mime.Add(".pgm", @"image/x-portable-graymap");
			mime.Add(".pgn", @"application/x-chess-pgn");
			mime.Add(".png", @"image/png");
			mime.Add(".pnm", @"image/x-portable-anymap");
			mime.Add(".pot", @"application/mspowerpoint");
			mime.Add(".ppm", @"image/x-portable-pixmap");
			mime.Add(".pps", @"application/mspowerpoint");
			mime.Add(".ppt", @"application/mspowerpoint");
			mime.Add(".ppz", @"application/mspowerpoint");
			mime.Add(".pre", @"application/x-freelance");
			mime.Add(".prt", @"application/pro_eng");
			mime.Add(".ps", @"application/postscript");
			mime.Add(".qt", @"video/quicktime");
			mime.Add(".ra", @"audio/x-realaudio");
			mime.Add(".ram", @"audio/x-pn-realaudio");
			mime.Add(".ras", @"image/cmu-raster");
			mime.Add(".rgb", @"image/x-rgb");
			mime.Add(".rm", @"audio/x-pn-realaudio");
			mime.Add(".roff", @"application/x-troff");
			mime.Add(".rpm", @"audio/x-pn-realaudio-plugin");
			mime.Add(".rtf", @"text/rtf");
			mime.Add(".rtx", @"text/richtext");
			mime.Add(".scm", @"application/x-lotusscreencam");
			mime.Add(".set", @"application/set");
			mime.Add(".sgm", @"text/sgml");
			mime.Add(".sgml", @"text/sgml");
			mime.Add(".sh", @"application/x-sh");
			mime.Add(".shar", @"application/x-shar");
			mime.Add(".silo", @"model/mesh");
			mime.Add(".sit", @"application/x-stuffit");
			mime.Add(".skd", @"application/x-koan");
			mime.Add(".skm", @"application/x-koan");
			mime.Add(".skp", @"application/x-koan");
			mime.Add(".skt", @"application/x-koan");
			mime.Add(".smi", @"application/smil");
			mime.Add(".smil", @"application/smil");
			mime.Add(".snd", @"audio/basic");
			mime.Add(".sol", @"application/solids");
			mime.Add(".spl", @"application/x-futuresplash");
			mime.Add(".src", @"application/x-wais-source");
			mime.Add(".step", @"application/STEP");
			mime.Add(".stl", @"application/SLA");
			mime.Add(".stp", @"application/STEP");
			mime.Add(".sv4cpio", @"application/x-sv4cpio");
			mime.Add(".sv4crc", @"application/x-sv4crc");
			mime.Add(".swf", @"application/x-shockwave-flash");
			mime.Add(".t", @"application/x-troff");
			mime.Add(".tar", @"application/x-tar");
			mime.Add(".tcl", @"application/x-tcl");
			mime.Add(".tex", @"application/x-tex");
			mime.Add(".texi", @"application/x-texinfo");
			mime.Add(".texinfo", @"application/x-texinfo");
			mime.Add(".tif", @"image/tiff");
			mime.Add(".tiff", @"image/tiff");
			mime.Add(".tr", @"application/x-troff");
			mime.Add(".tsi", @"audio/TSP-audio");
			mime.Add(".tsp", @"application/dsptype");
			mime.Add(".tsv", @"text/tab-separated-values");
			mime.Add(".txt", @"text/plain");
			mime.Add(".unv", @"application/i-deas");
			mime.Add(".ustar", @"application/x-ustar");
			mime.Add(".vcd", @"application/x-cdlink");
			mime.Add(".vda", @"application/vda");
			mime.Add(".viv", @"video/vnd.vivo");
			mime.Add(".vivo", @"video/vnd.vivo");
			mime.Add(".vrml", @"model/vrml");
			mime.Add(".wav", @"audio/x-wav");
			mime.Add(".wrl", @"model/vrml");
			mime.Add(".xbm", @"image/x-xbitmap");
			mime.Add(".xlc", @"application/vnd.ms-excel");
			mime.Add(".xll", @"application/vnd.ms-excel");
			mime.Add(".xlm", @"application/vnd.ms-excel");
			mime.Add(".xls", @"application/vnd.ms-excel");
			mime.Add(".xlw", @"application/vnd.ms-excel");
			mime.Add(".xml", @"text/xml");
			mime.Add(".xpm", @"image/x-xpixmap");
			mime.Add(".xwd", @"image/x-xwindowdump");
			mime.Add(".xyz", @"chemical/x-pdb");
			mime.Add(".zip", @"application/zip");
		}

		public static IDictionary<string, string> MimeTypes {
			get {
				return mime;
			}
		}

		public static void SendFile(string FileName, HttpResponse Response) {
			if (File.Exists(FileName)) {

				FileInfo fi = new FileInfo(FileName);

				string ContentType = null, ext = fi.Extension.ToLower();
				if (mime.ContainsKey(ext)) ContentType = mime[ext];
				else ContentType = "application/octet-stream";

				Response.ContentType = ContentType;
				System.IO.Stream iStream = null;

				byte[] buffer = new byte[FILESEND_BUFFER_SIZE];	//Buffer to read 1K bytes in chunk
				int length;										//Length of the file
				long dataToRead;								//Total bytes left to read

				try {
					//Open the file.
					iStream = new FileStream(FileName, FileMode.Open, FileAccess.Read, FileShare.Read);

					//Total bytes to read:
					dataToRead = iStream.Length;

					// Read the bytes.
					while (dataToRead > 0) {
						//Verify that the client is connected.
						if (Response.IsClientConnected) {
							//Read the data in buffer.
							length = iStream.Read(buffer, 0, FILESEND_BUFFER_SIZE);
							dataToRead -= length;

							//Write the data to the current output stream.
							Response.OutputStream.Write(buffer, 0, length);

							//Flush the data to the HTML output.
							Response.Flush();
						} else {
							//prevent infinite loop if user disconnects
							dataToRead = -1;
						}
					}
				} catch (Exception) {
				} finally {
					if (iStream != null)
						iStream.Close();
				}
				Response.End();
				return;
			} else throw new System.IO.FileNotFoundException("Unable to find the file '" + FileName + "'");
		}
		#endregion

		#region MapPath
		public static string MapPath(string URI) {
			try {
				//Check out our virtual directories first...
				VirtualDirectorySettings vs = Settings.From<VirtualDirectorySettings>(Settings.Section.VirtualDirectory);
				if (vs != null && vs.VirtualDirectories != null) {
					int index = 0;
					string fullPath = string.Empty;
					string vPath = vs.Find(URI, ref index, ref fullPath);
					if (!string.IsNullOrEmpty(fullPath) && !string.IsNullOrEmpty(vPath)) {
						return Path.GetFullPath(fullPath + URI.Substring(index + vPath.Length));
					}
				}
			} catch {
			}
			return HttpContext.Current.Server.MapPath(URI);
		}
		#endregion

		public static string GetBaseUrl() {
			HttpSecuritySettings hs = Settings.From<HttpSecuritySettings>(Settings.Section.HttpSecurity);
			if (hs != null)
				return hs.UnsecureSite;
			else
				return string.Empty;
		}

		public static string CheckApplicationPath(string Path) {
			if (HttpContext.Current != null)
				return Path.Replace("~/", HttpContext.Current.Request.ApplicationPath + (HttpContext.Current.Request.ApplicationPath.EndsWith(@"/") || HttpContext.Current.Request.ApplicationPath.EndsWith(@"\") ? "" : @"/"));
			return Path;
		}

		///<summary>Recursively looks for a control in the hierarchy...</summary>
		public static Control FindControl(Control Parent, string Name) {
			if (Parent == null) return null;

			foreach (Control c in Parent.Controls) {
				if (!string.IsNullOrEmpty(c.ID) && Name.Equals(c.ID, StringComparison.InvariantCultureIgnoreCase))
					return c;
				else
					if (c.Controls != null && c.Controls.Count > 0) {
						Control ret = FindControl(c, Name);
						if (ret != null)
							return ret;
					}
			}
			return null;
		}

		public static bool RemoveCookie(string CookieName) {
			if (System.Web.HttpContext.Current == null) return false;
			if (System.Web.HttpContext.Current.Request.Cookies[CookieName] == null) return false;
			if (System.Web.HttpContext.Current.Response.Cookies[CookieName] == null) return false;
			System.Web.HttpContext.Current.Response.Cookies[CookieName].Expires = DateTime.Now.AddMinutes(-1.0D);
			return true;
		}

		public static PeriodInformation ParsePeriod(string Period) {
			return new PeriodInformation(Period);
		}

		private static PasswordGenerator pwdGenerator = new PasswordGenerator();
		public static string GeneratePassword() {
			return pwdGenerator.Generate();
		}

		public static void SendSystemErrorMessage(string Subject, string Message) {
			SendSystemErrorMessage(Subject, Message, false);
		}
		public static void SendSystemErrorMessage(string To, string Subject, string Message) {
			SendSystemErrorMessage(To, Subject, Message, false);
		}
		public static void SendSystemErrorMessage(string Subject, string Message, bool Important) {
			CustomErrorsSettings ce = Settings.From<CustomErrorsSettings>(Settings.Section.CustomErrors);
			if (ce != null && ce.UseErrorEmail)
				SendSystemErrorMessage(ce.ErrorEmailAddress, Subject, Message, Important);
		}
		public static void SendSystemErrorMessage(string To, string Subject, string Message, bool Important) {
			if (string.IsNullOrEmpty(To) || string.IsNullOrEmpty(Message)) return;
			Email.Send("(" + DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToShortTimeString() + ")  ::  " + Subject, Message, To, Important ? System.Net.Mail.MailPriority.High : System.Net.Mail.MailPriority.Normal);
		}

		public static string CleanExtraChars(string Value) {
			if (string.IsNullOrEmpty(Value))
				return string.Empty;
			return Value.Replace(" ", string.Empty).Replace("-", string.Empty);
		}

		public static string CCStarRepresentation(string val) {
			if (string.IsNullOrEmpty(val))
				return string.Empty;

			string ccNumber = val.Replace(" ", string.Empty).Replace("-", string.Empty);
			int len = ccNumber.Length;
			if (len <= 2)
				return ccNumber;
			return ccNumber.Substring(len - 2, 2).PadLeft(len - 2, '*');
		}

		public static string CheckNA(string val) {
			if (!string.IsNullOrEmpty(val))
				return val;
			else
				return "N/A";
		}

		public static object CheckNAObject(object val) {
			if (val == null)
				return "N/A";
			else
				return val;
		}

		public static bool RegisterHttpHandler(string Path, string Type, string Verb) {
			return RegisterHttpHandler(Path, Type, Verb, false);
		}

		public static bool RegisterHttpHandler(string Path, string Type, string Verb, bool Validate) {
			//DOES NOT WORK! For some reason it's added but the framework doesn't pick it up...odd!

			System.Configuration.Configuration config = WebConfigurationManager.OpenWebConfiguration("~");
			if (config == null)
				return false;

			HttpHandlersSection section = (HttpHandlersSection)config.GetSection("system.web/httpHandlers");


			//SystemWebSectionGroup grp = (SystemWebSectionGroup)config.GetSectionGroup("system.web");
			//if (grp == null)
			//	return false;
			//
			//if (grp.HttpHandlers == null || grp.HttpHandlers.Handlers == null)
			//	return false;
			//
			//HttpHandlersSection section = grp.HttpHandlers;

			//Debug.WriteLine(section.SectionInformation.GetRawXml());
			HttpHandlerActionCollection coll = section.Handlers;
			coll.Add(new HttpHandlerAction(Path, Type, Verb, Validate));
			//Debug.WriteLine(section.SectionInformation.GetRawXml());
			return true;
		}

		public static long BytesToLong(byte[] address) {
			long ipnum = 0;
			for (int i = 0; i < 4; ++i) {
				long y = address[i];
				if (y < 0) {
					y += 256;
				}
				ipnum += y << ((3 - i) * 8);
			}
			return ipnum;
		}

		public static long GetIPAddressAsLong(string IPAddress) {
			System.Net.IPAddress addr = null;
			try {
				addr = System.Net.IPAddress.Parse(IPAddress);
			} catch {
				return 0L;
			}
			return BytesToLong(addr.GetAddressBytes());
		}
	}

	//{BaseUrl}, {FirstName}, {LastName}, {CompanyName}, {Email}, {Login}, {ID}, {Password}

	#region Helper Classes
	///<summary>
	///	Copyright © 2005 Kevin Stewart
	/// See http://www.codeproject.com/csharp/pwdgen.asp
	///</summary>
	internal class PasswordGenerator {
		public PasswordGenerator() {
			this.Minimum = DefaultMinimum;
			this.Maximum = DefaultMaximum;
			this.ConsecutiveCharacters = false;
			this.RepeatCharacters = true;
			this.ExcludeSymbols = false;
			this.Exclusions = null;

			rng = new RNGCryptoServiceProvider();
		}

		protected int GetCryptographicRandomNumber(int lBound, int uBound) {
			// Assumes lBound >= 0 && lBound < uBound
			// returns an int >= lBound and < uBound
			uint urndnum;
			byte[] rndnum = new Byte[4];
			if (lBound == uBound - 1) {
				// test for degenerate case where only lBound can be returned
				return lBound;
			}

			uint xcludeRndBase = (uint.MaxValue -
				(uint.MaxValue % (uint)(uBound - lBound)));

			do {
				rng.GetBytes(rndnum);
				urndnum = System.BitConverter.ToUInt32(rndnum, 0);
			} while (urndnum >= xcludeRndBase);

			return (int)(urndnum % (uBound - lBound)) + lBound;
		}

		protected char GetRandomCharacter() {
			int upperBound = pwdCharArray.GetUpperBound(0);

			if (true == this.ExcludeSymbols) {
				upperBound = PasswordGenerator.UBoundDigit;
			}

			int randomCharPosition = GetCryptographicRandomNumber(
				pwdCharArray.GetLowerBound(0), upperBound);

			char randomChar = pwdCharArray[randomCharPosition];

			return randomChar;
		}

		public string Generate() {
			// Pick random length between minimum and maximum   
			int pwdLength = GetCryptographicRandomNumber(this.Minimum, this.Maximum);

			StringBuilder pwdBuffer = new StringBuilder();
			pwdBuffer.Capacity = this.Maximum;

			// Generate random characters
			char lastCharacter, nextCharacter;

			// Initial dummy character flag
			lastCharacter = nextCharacter = '\n';

			for (int i = 0; i < pwdLength; i++) {
				nextCharacter = GetRandomCharacter();

				if (false == this.ConsecutiveCharacters) {
					while (lastCharacter == nextCharacter) {
						nextCharacter = GetRandomCharacter();
					}
				}

				if (false == this.RepeatCharacters) {
					string temp = pwdBuffer.ToString();
					int duplicateIndex = temp.IndexOf(nextCharacter);
					while (-1 != duplicateIndex) {
						nextCharacter = GetRandomCharacter();
						duplicateIndex = temp.IndexOf(nextCharacter);
					}
				}

				if ((null != this.Exclusions)) {
					while (-1 != this.Exclusions.IndexOf(nextCharacter)) {
						nextCharacter = GetRandomCharacter();
					}
				}

				pwdBuffer.Append(nextCharacter);
				lastCharacter = nextCharacter;
			}

			if (null != pwdBuffer) {
				return pwdBuffer.ToString();
			} else {
				return String.Empty;
			}
		}

		public string Exclusions {
			get { return this.exclusionSet; }
			set { this.exclusionSet = value; }
		}

		public int Minimum {
			get { return this.minSize; }
			set {
				this.minSize = value;
				if (PasswordGenerator.DefaultMinimum > this.minSize) {
					this.minSize = PasswordGenerator.DefaultMinimum;
				}
			}
		}

		public int Maximum {
			get { return this.maxSize; }
			set {
				this.maxSize = value;
				if (this.minSize >= this.maxSize) {
					this.maxSize = PasswordGenerator.DefaultMaximum;
				}
			}
		}

		public bool ExcludeSymbols {
			get { return this.hasSymbols; }
			set { this.hasSymbols = value; }
		}

		public bool RepeatCharacters {
			get { return this.hasRepeating; }
			set { this.hasRepeating = value; }
		}

		public bool ConsecutiveCharacters {
			get { return this.hasConsecutive; }
			set { this.hasConsecutive = value; }
		}

		private const int DefaultMinimum = 6;
		private const int DefaultMaximum = 10;
		private const int UBoundDigit = 61;

		private RNGCryptoServiceProvider rng;
		private int minSize;
		private int maxSize;
		private bool hasRepeating;
		private bool hasConsecutive;
		private bool hasSymbols;
		private string exclusionSet;
		private static char[] pwdCharArray = (
			"abcdefghijklmnopqrstuvwxyzABCDEFG" +
			"HIJKLMNOPQRSTUVWXYZ0123456789!@#$%*()-=+:" +
			".?"
		).ToCharArray();
	}
	#endregion
}
