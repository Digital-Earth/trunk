#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Web;
using System.Xml;
using System.Configuration;
using System.Web.Configuration;

namespace HoytSoft.Common.Web {
	public interface IApplicationListener {
		void AppStart();
		void AppEnd();
		void AppError();
		void AppBeginRequest();
		void AppEndRequest();
		void AppAuthenticateRequest();
		void AppAuthorizeRequest();

		void SessionStart();
		void SessionEnd();
	}

	public abstract class AbstractApplicationListener : IApplicationListener {
		public virtual void AppStart() { }
		public virtual void AppEnd() { }
		public virtual void AppError() { }
		public virtual void AppBeginRequest() { }
		public virtual void AppEndRequest() { }
		public virtual void AppAuthenticateRequest() { }
		public virtual void AppAuthorizeRequest() { }
		public virtual void SessionStart() { }
		public virtual void SessionEnd() { }
	}

	public class Application : System.Web.HttpApplication, IApplicationListener, Settings.ISectionHandler {
		#region Variables
		private static Application app = null;
		private int appObjectCount = 0;
		private System.Collections.Generic.IList<IApplicationListener> appObjects = null;
		#endregion

		#region Constructors
		static Application() {
			app = (Application)Settings.From(Settings.Section.Application);
		}
		#endregion

		#region Properties
		public Settings.Section SectionName { get { return Settings.Section.Application; } }
		public string ConfigSectionName { get { return "Application"; } }
		public static System.Collections.Generic.IList<IApplicationListener> ApplicationObjects { get { if (app != null) return app.appObjects; return null; } }
		public static int ApplicationObjectCount { get { return app.appObjectCount; } }
		#endregion

		#region Helper Methods
		private void loadProviders() {
			object section = null;
			
			//Open web parts section...
			if ((section = WebConfigurationManager.GetSection("system.web/webParts")) != null && section is WebPartsSection) {
				WebPartsSection webPartsSection = (WebPartsSection)section; //webPartsSection is read only!
				//webPartsSection.Personalization.Providers.Add(new ProviderSettings(Shared.Providers.CookiePersonalizationProvider.ProviderName, typeof(Shared.Providers.CookiePersonalizationProvider).FullName));
			}

		}
		#endregion

		#region Protected Methods
		protected void Application_Start(Object sender, EventArgs e) { this.AppStart(); }
		protected void Session_Start(Object sender, EventArgs e) { this.SessionStart(); }
		protected void Application_BeginRequest(Object sender, EventArgs e) { this.AppBeginRequest(); } 
		protected void Application_EndRequest(Object sender, EventArgs e) { this.AppEndRequest(); }
		protected void Application_AuthenticateRequest(Object sender, EventArgs e) { this.AppAuthenticateRequest(); }
		protected void Application_AuthorizeRequest(object sender, EventArgs e) { this.AppAuthorizeRequest(); }
		protected void Application_Error(Object sender, EventArgs e) { this.AppError(); }
		protected void Session_End(Object sender, EventArgs e) { this.SessionEnd(); }
		protected void Application_End(Object sender, EventArgs e) { this.AppEnd(); }
		#endregion

		#region IApplication Members
		public void AppStart() {
			this.AuthorizeRequest += new EventHandler(Application_AuthorizeRequest);

			this.loadProviders();

			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppStart();
		}

		public void AppEnd() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppEnd();
		}

		public void AppError() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppError();
		}

		public void AppBeginRequest() {
			//Throw in some localization support for those areas that have it...
			try {
				if (Request.UserLanguages != null && Request.UserLanguages.Length > 0) {
					System.Globalization.CultureInfo ci = System.Globalization.CultureInfo.CreateSpecificCulture(Request.UserLanguages[0]);
					//System.Globalization.CultureInfo ci = System.Globalization.CultureInfo.CreateSpecificCulture("es");
					System.Threading.Thread.CurrentThread.CurrentCulture = ci;
					System.Threading.Thread.CurrentThread.CurrentUICulture = ci;
				}
			} catch (ArgumentException) {
			}
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppBeginRequest();
		}

		public void AppEndRequest() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppEndRequest();
		}

		public void AppAuthenticateRequest() {
			Authentication.AuthenticateRequest();

			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppAuthenticateRequest();
		}

		public void AppAuthorizeRequest() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.AppAuthorizeRequest();
		}

		public void SessionStart() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.SessionStart();
		}

		public void SessionEnd() {
			if (ApplicationObjects == null || ApplicationObjectCount <= 0) return;
			foreach (IApplicationListener ia in ApplicationObjects)
				ia.SessionEnd();
		}
		#endregion

		#region IConfigurationSectionHandler
		public object Create(object Parent, object Context, XmlNode Section) {
			if (Section.HasChildNodes) {
				XmlNodeList xnl = null;
				if ((xnl = Section.SelectNodes("Add")) != null && xnl.Count > 0) {
					//Remove any existing ones...
					if (this.appObjects != null) {
						this.appObjects.Clear();
						this.appObjectCount = 0;
					}

					foreach (XmlNode xn in xnl) {
						if (xn.Attributes["type"] != null && !string.IsNullOrEmpty(xn.Attributes["type"].Value)) {
							Type t = null;
							if ((t = Type.GetType(xn.Attributes["type"].Value, false, true)) != null) {
								Type[] tt = t.GetInterfaces(); 
								foreach (Type ti in tt) {
									if (ti.IsInterface && ti.FullName == typeof(IApplicationListener).FullName) {
										if (this.appObjects == null)
											this.appObjects = new System.Collections.Generic.List<IApplicationListener>();
										IApplicationListener obj = (IApplicationListener)Activator.CreateInstance(t);
										this.appObjects.Add(obj);
										this.appObjectCount++;
									}
								}
							}
						}
					}
				}
			}
			return this;
		}
		#endregion
	}
}
