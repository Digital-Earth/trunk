#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.IO;
using System.Xml;
using System.Security.Cryptography.X509Certificates;

namespace HoytSoft.Common.Configuration {
	public class LinkPoint : HoytSoft.Common.Settings.ISectionHandler {
		#region Public Enums
		public enum LogLevel : int {
			///<summary>No logging whatsoever</summary>
			None = 0,
			///<summary>Shows only errors</summary>
			Errors = 1,
			///<summary>Errors + additional information.</summary>
			Debug = 2,
			///<summary>Full debug mode with call tracing - produces a lot of output!</summary>
			Trace = 3
		}
		#endregion

		#region Variables
		public const string
			SETTINGS_SECTION = "LinkPoint";
		private int  port = -1;
		private string storeNumber, keyFile, host, logFile, configFile, password;
		private LogLevel logLvl = LogLevel.None;
		private X509Certificate certificate = null;
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.LinkPoint; } }
		public string ConfigSectionName { get { return SETTINGS_SECTION; } }
		public string StoreNumber { get { return this.storeNumber; } }
		public int Port { get { return this.port; } }
		public LogLevel LoggingLevel { get { return this.logLvl; } }
		public string KeyFile { get { return this.keyFile; } }
		public string Host { get { return this.host; } }
		public string Password { get { return this.password; } }
		public X509Certificate Certificate { get { return this.certificate; } }
		public string LoggingFile { get { return this.logFile; } }
		public string ConfigFile { get { return this.configFile; } }
		public bool Logging { get { return this.logLvl != LogLevel.None; } }
		#endregion

		public object Create(object Parent, object Context, System.Xml.XmlNode Section) {
			if (Section != null && Section.Attributes != null && Section.Attributes.Count > 0) {
				string tmpStoreNumber = null, tmpPort = null, tmpKeyFile = null, tmpHost = null, tmpLogFile = null, tmpPassword = null;
				object tmpLogLevel = null;

				//Check to see if we have all the required attributes...
				if (
					   Section.Attributes["storeNumber"] == null
					|| string.IsNullOrEmpty(tmpStoreNumber = Section.Attributes["storeNumber"].Value)
					|| Section.Attributes["keyFile"] == null
					|| string.IsNullOrEmpty(tmpKeyFile = Section.Attributes["keyFile"].Value)
					|| Section.Attributes["host"] == null
					|| string.IsNullOrEmpty(tmpHost = Section.Attributes["host"].Value)
					|| Section.Attributes["port"] == null
					|| string.IsNullOrEmpty(tmpPort = Section.Attributes["port"].Value)
					|| Section.Attributes["logLevel"] == null
					|| string.IsNullOrEmpty((string)(tmpLogLevel = Section.Attributes["logLevel"].Value))
					|| Section.Attributes["logFile"] == null
					|| string.IsNullOrEmpty(tmpLogFile = Section.Attributes["logFile"].Value)
				) throw new XmlException("Missing required attribute for LinkPoint tag. Must have storeNumber, keyFile, host, port, logLevel, and logFile");

				if (!Enum.IsDefined(typeof(LogLevel), tmpLogLevel)) {
					try {
						tmpLogLevel = int.Parse((string)tmpLogLevel);
					} catch (FormatException) {
					}
				}

				if (!Enum.IsDefined(typeof(LogLevel), tmpLogLevel))
					throw new XmlException("The specified logLevel is not a valid value. It must be between 0 and 3");

				if (Section.Attributes["password"] != null && !string.IsNullOrEmpty(tmpPassword = Section.Attributes["password"].Value))
					this.password = tmpPassword;
				else
					this.password = null;

				this.port = int.Parse(tmpPort);
				this.storeNumber = tmpStoreNumber;
				this.configFile = tmpStoreNumber; //ConfigFile is the same as the store number!
				this.logLvl = (LogLevel)Enum.ToObject(typeof(LogLevel), tmpLogLevel);
				this.host = tmpHost;
				if (System.Web.HttpContext.Current != null) {
					this.keyFile = System.Web.HttpContext.Current.Server.MapPath(tmpKeyFile);
					this.logFile = System.Web.HttpContext.Current.Server.MapPath(tmpLogFile);
				} else {
					this.keyFile = tmpKeyFile;
					this.logFile = tmpLogFile;
				}

				//Attempt to load certificate...
				if (File.Exists(this.keyFile)) {
					try {
						if (X509Certificate2.GetCertContentType(this.keyFile) != X509ContentType.Unknown) {
							if (this.password != null)
								this.certificate = new X509Certificate2(this.keyFile, this.password, X509KeyStorageFlags.PersistKeySet);
							else
								this.certificate = new X509Certificate2(this.keyFile);
						}
					} catch {
						try {
							Common.Security.CertificateFormatParser.ParseResult results = Common.Security.CertificateFormatParser.Parse(Common.Security.CertificateFormat.PEM, this.keyFile);
							if (results != null && results.Certificate != null)
								this.certificate = results.Certificate;
						} catch {
						}
					}
				}

				return this;
			}
			return null;
		}
	}
}
