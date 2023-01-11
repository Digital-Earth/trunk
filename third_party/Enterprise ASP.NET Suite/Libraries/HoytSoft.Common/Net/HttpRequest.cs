#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2007 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.IO;
using System.Net;
using System.Text;
using System.Security;
using System.Diagnostics;
using System.Net.Security;
using System.Security.Policy;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;

namespace HoytSoft.Common.Net {
	#region Public Enums
	public enum CertificateValidation {
		Default		= 0, 
		AllowAll	= 1, 
		AllowNone	= 2, 
		CheckZone	= 3
	}

	public enum RequestMethod {
		Get		= 0, 
		Post	= 1
	}
	#endregion

	public class HttpRequest {
		#region Constants
		private const string
			CONTENT_TYPE_FORM_URL_ENCODED	= "application/x-www-form-urlencoded", 
			DELIM_KEY_VALUE					= "=", 
			DELIM_KEY						= "&"
		;
		#endregion

		#region Variables
		private static object validationLock = new object();
		#endregion

		#region CertificateValidation Methods
		protected static bool ValidateCertificateAllowAll(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
			return true;
		}

		protected static bool ValidateCertificateAllowNone(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
			return true;
		}

		protected static bool ValidateCertificateDefault(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
			return (sslPolicyErrors == SslPolicyErrors.None);
		}

		protected static bool ValidateCertificateCheckZone(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
			if ((sslPolicyErrors & SslPolicyErrors.RemoteCertificateChainErrors) == SslPolicyErrors.RemoteCertificateChainErrors) {
				return false;
			} else if ((sslPolicyErrors & SslPolicyErrors.RemoteCertificateNameMismatch) == SslPolicyErrors.RemoteCertificateNameMismatch) {
				Zone z;
				z = Zone.CreateFromUrl(((HttpWebRequest)sender).RequestUri.ToString());
				switch (z.SecurityZone) {
					case SecurityZone.Intranet:
					case SecurityZone.MyComputer:
					case SecurityZone.Trusted:
						return true;
					default:
						return false;
				}
			}
			return true;
		}
		#endregion

		#region Misc Public Methods
		public static string Request(Uri RequestURI) {
			return Request(RequestURI, RequestMethod.Get, null, null, CertificateValidation.Default, null);
		}

		public static string Request(string RequestURI) {
			return Request(RequestURI, RequestMethod.Get, null, null, CertificateValidation.Default, null);
		}

		public static string Request(Uri RequestURI, RequestMethod Method) {
			return Request(RequestURI, Method, null, null, CertificateValidation.Default, null);
		}

		public static string Request(string RequestURI, RequestMethod Method) {
			return Request(RequestURI, Method, null, null, CertificateValidation.Default, null);
		}

		public static string Request(Uri RequestURI, RequestMethod Method, CertificateValidation ValidationType) {
			return Request(RequestURI, Method, null, null, ValidationType, null);
		}

		public static string Request(string RequestURI, RequestMethod Method, CertificateValidation ValidationType) {
			return Request(RequestURI, Method, null, null, ValidationType, null);
		}

		public static string Request(Uri RequestURI, RequestMethod Method, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {
			return Request(RequestURI, Method, null, null, ValidationType, ClientCertificates);
		}

		public static string Request(string RequestURI, RequestMethod Method, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {
			return Request(RequestURI, Method, null, null, ValidationType, ClientCertificates);
		}

		public static string Request(Uri RequestURI, RequestMethod Method, string[] ArgumentNames, string[] ArgumentValues, CertificateValidation ValidationType) {
			return Request(RequestURI, Method, ArgumentNames, ArgumentValues, ValidationType, null);
		}

		public static string Request(string RequestURI, RequestMethod Method, string[] ArgumentNames, string[] ArgumentValues, CertificateValidation ValidationType) {
			return Request(RequestURI, Method, ArgumentNames, ArgumentValues, ValidationType, null);
		}

		public static string Request(string RequestURI, RequestMethod Method, string[] ArgumentNames, string[] ArgumentValues, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {
			return Request(new Uri(RequestURI), Method, ArgumentNames, ArgumentValues, ValidationType, ClientCertificates);
		}

		public static string Request(string RequestURI, RequestMethod Method, bool UsePostData, Stream PostData, string ContentType, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {
			return Request(new Uri(RequestURI), Method, true, PostData, ContentType, ValidationType, ClientCertificates);
		}
		#endregion

		public static string Request(Uri RequestURI, RequestMethod Method, bool UsePostData, Stream PostData, string ContentType, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {

			lock (validationLock) {
				RemoteCertificateValidationCallback callback = null, previous = null;
				Stream s = null;
				StreamReader sr = null;
				HttpWebResponse res = null;
				
				previous = ServicePointManager.ServerCertificateValidationCallback;

				switch (ValidationType) {
					case CertificateValidation.AllowAll:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowAll);
						break;
					case CertificateValidation.AllowNone:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowNone);
						break;
					case CertificateValidation.CheckZone:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateCheckZone);
						break;
					default:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateDefault);
						break;
				}

				try {
					//Hook a callback to verify the remote certificate
					ServicePointManager.ServerCertificateValidationCallback = callback;

					HttpWebRequest req = (HttpWebRequest)WebRequest.Create(RequestURI);

					req.Proxy = null;
					req.Credentials = CredentialCache.DefaultCredentials;

					if (ClientCertificates != null)
						req.ClientCertificates.AddRange(ClientCertificates);

					switch (Method) {
						case RequestMethod.Get:
							req.Method = "GET";
							break;

						case RequestMethod.Post:
							req.Method = "POST";
							#region load data
							if (UsePostData && PostData != null) {
								long total = 0;
								int count = 0;
								byte[] buffer = new byte[1024];
								using (Stream writeStream = req.GetRequestStream()) {
									while ((count = PostData.Read(buffer, 0, buffer.Length)) > 0) {
										writeStream.Write(buffer, 0, count);
										total += count;
									}
								}
								//ContentLength is set automatically...
								req.ContentType = ContentType;
							}
							#endregion
							break;
					}
					req.AllowAutoRedirect = true;
					req.AuthenticationLevel = AuthenticationLevel.MutualAuthRequested;
					req.ProtocolVersion = new Version(1, 0);

					res = req.GetResponse() as HttpWebResponse;
					s = res.GetResponseStream();
					sr = new StreamReader(s, Encoding.UTF8);
					return sr.ReadToEnd();

				} catch (Exception ex) {
					Debug.WriteLine(ex);
					return string.Empty;
				} finally {
					if (res != null) res.Close();
					if (s != null) s.Close();
					if (sr != null) sr.Close();

					ServicePointManager.ServerCertificateValidationCallback = previous;
				}
				
			}
		}

		public static string Request(Uri RequestURI, RequestMethod Method, string[] ArgumentNames, string[] ArgumentValues, CertificateValidation ValidationType, X509Certificate[] ClientCertificates) {
			if (ArgumentNames != null && ArgumentValues != null && ArgumentNames.Length != ArgumentValues.Length)
				throw new ArgumentException("ArgumentNames and ArgumentValues must be the same length");

			lock (validationLock) {
				RemoteCertificateValidationCallback callback = null, previous = null;
				Stream s = null;
				StreamReader sr = null;
				HttpWebResponse res = null;
				
				previous = ServicePointManager.ServerCertificateValidationCallback;

				switch (ValidationType) {
					case CertificateValidation.AllowAll:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowAll);
						break;
					case CertificateValidation.AllowNone:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowNone);
						break;
					case CertificateValidation.CheckZone:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateCheckZone);
						break;
					default:
						callback = new RemoteCertificateValidationCallback(ValidateCertificateDefault);
						break;
				}

				try {
					//Hook a callback to verify the remote certificate
					ServicePointManager.ServerCertificateValidationCallback = callback;

					HttpWebRequest req = (HttpWebRequest)WebRequest.Create(RequestURI);

					req.Proxy = null;
					req.Credentials = CredentialCache.DefaultCredentials;
					
					if (ClientCertificates != null)
						req.ClientCertificates.AddRange(ClientCertificates);

					switch (Method) {
						case RequestMethod.Get:
							req.Method = "GET";
							break;

						case RequestMethod.Post:
							req.Method = "POST";
							#region load args
							if (ArgumentNames != null && ArgumentValues != null && ArgumentNames.Length > 0 && ArgumentValues.Length > 0) {
								StringBuilder sb = new StringBuilder();
								int len = ArgumentNames.Length;
								for (int i = 0; i < len; i++) {
									sb.Append(ArgumentNames[i]);
									sb.Append(DELIM_KEY_VALUE);
									sb.Append(ArgumentValues[i]);
									if (i < len - 1)
										sb.Append(DELIM_KEY);
								}

								using (Stream writeStream = req.GetRequestStream()) {
									byte[] bytes = Encoding.UTF8.GetBytes(sb.ToString());
									writeStream.Write(bytes, 0, bytes.Length);
									req.ContentLength = bytes.Length;
									req.ContentType = CONTENT_TYPE_FORM_URL_ENCODED;
								}
							}
							#endregion
							break;
					}

					res = req.GetResponse() as HttpWebResponse;
					s = res.GetResponseStream();
					sr = new StreamReader(s, Encoding.UTF8);
					return sr.ReadToEnd();

				} catch (Exception ex) {
					Debug.WriteLine(ex);
					return string.Empty;
				} finally {
					if (res != null) res.Close();
					if (s != null) s.Close();
					if (sr != null) sr.Close();

					ServicePointManager.ServerCertificateValidationCallback = previous;
				}
				
			}
		}

		public static string RawSSLRequest(string Host, int Port, string PostData, CertificateValidation ValidationType, X509Certificate[] Certificates) {
			return RawSSLRequest(Host, Port, Encoding.UTF8.GetBytes(PostData), ValidationType, Certificates);
		}

		public static string RawSSLRequest(string Host, int Port, byte[] PostData, CertificateValidation ValidationType, X509Certificate[] Certificates) {
			UriBuilder ub = new UriBuilder(Uri.UriSchemeHttps, Host, Port);
			return RawSSLRequest(ub.Uri, PostData, ValidationType, Certificates);
		}

		public static string RawSSLRequest(Uri RequestURI, byte[] PostData, CertificateValidation ValidationType, X509Certificate[] Certificates) {
			RemoteCertificateValidationCallback callback = null;
			switch (ValidationType) {
				case CertificateValidation.AllowAll:
					callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowAll);
					break;
				case CertificateValidation.AllowNone:
					callback = new RemoteCertificateValidationCallback(ValidateCertificateAllowNone);
					break;
				case CertificateValidation.CheckZone:
					callback = new RemoteCertificateValidationCallback(ValidateCertificateCheckZone);
					break;
				default:
					callback = new RemoteCertificateValidationCallback(ValidateCertificateDefault);
					break;
			}

			TcpClient client = null;
			SslStream sslStream = null;
			try {
				client = new TcpClient(RequestURI.Host, RequestURI.Port);
				
				//Create an SSL stream that will close the client's stream.
				sslStream = new SslStream(
					client.GetStream(),
					false,
					callback,
					null
				);

				//Do the magic...
				sslStream.AuthenticateAsClient(RequestURI.Host, new X509CertificateCollection(Certificates), System.Security.Authentication.SslProtocols.Tls, false);
				if (PostData != null && PostData.Length > 0) {
					sslStream.Write(PostData);
				}
				sslStream.Flush();

				//Get response...
				byte[] buffer = new byte[2048];
				StringBuilder response = new StringBuilder();
				int bytes = -1;
				while ((bytes = sslStream.Read(buffer, 0, buffer.Length)) > 0)
					response.Append(Encoding.UTF8.GetString(buffer, 0, bytes));
				return response.ToString();
			} catch(SocketException se) {
				throw se;
			} catch (Exception ex) {
				return null;
			} finally {
				if (client != null) client.Close();
				if (sslStream != null) sslStream.Close();
				if (sslStream != null) sslStream.Dispose();
			}


		}
	} //class
}
