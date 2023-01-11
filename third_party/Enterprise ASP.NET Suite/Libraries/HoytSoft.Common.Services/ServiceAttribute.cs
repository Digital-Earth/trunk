#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2005 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;

namespace HoytSoft.Common.Services {
	///<summary>Describes a new Win32 service.</summary>
	[AttributeUsage(AttributeTargets.Class, AllowMultiple=false, Inherited=true)]
	public class ServiceAttribute : System.Attribute {
		#region Private Variables
		private string name;
		private string displayName;
		private string description;
		private string logName;
		private bool run;
		private bool autoInstall = true;
		private ServiceType servType;
		private ServiceAccessType servAccessType;
		private ServiceStartType servStartType;
		private ServiceErrorControl servErrorControl;
		private ServiceControls servControls;
		#endregion

		#region Constructors
		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="Run">Indicates if you want the service to run or not on program startup.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		/// <param name="ServiceAccessType">Access to the service. Before granting the requested access, the system checks the access token of the calling process.</param>
		/// <param name="ServiceStartType">Service start options. By default this is "AutoStart."</param>
		/// <param name="ServiceErrorControl">Severity of the error, and action taken, if this service fails to start.</param>
		/// <param name="ServiceControls">The controls or actions the service responds to.</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, bool Run, ServiceType ServiceType, ServiceAccessType ServiceAccessType, ServiceStartType ServiceStartType, ServiceErrorControl ServiceErrorControl, ServiceControls ServiceControls) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = Run;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="Run">Indicates if you want the service to run or not on program startup.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		/// <param name="ServiceAccessType">Access to the service. Before granting the requested access, the system checks the access token of the calling process.</param>
		/// <param name="ServiceStartType">Service start options. By default this is "AutoStart."</param>
		/// <param name="ServiceErrorControl">Severity of the error, and action taken, if this service fails to start.</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, bool Run, ServiceType ServiceType, ServiceAccessType ServiceAccessType, ServiceStartType ServiceStartType, ServiceErrorControl ServiceErrorControl) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = Run;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		/// <param name="ServiceAccessType">Access to the service. Before granting the requested access, the system checks the access token of the calling process.</param>
		/// <param name="ServiceStartType">Service start options. By default this is "AutoStart."</param>
		/// <param name="ServiceErrorControl">Severity of the error, and action taken, if this service fails to start.</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, ServiceType ServiceType, ServiceAccessType ServiceAccessType, ServiceStartType ServiceStartType, ServiceErrorControl ServiceErrorControl) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = true;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		/// <param name="ServiceStartType">Service start options. By default this is "AutoStart."</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, ServiceType ServiceType, ServiceStartType ServiceStartType) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = true;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="ServiceStartType">Service start options. By default this is "AutoStart."</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, ServiceStartType ServiceStartType) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = true;
			this.servType = ServiceType.Default;;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, ServiceType ServiceType) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = true;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="Run">Indicates if you want the service to run or not on program startup.</param>
		/// <param name="ServiceType">Indicates the type of service you will be running. By default this is "Default."</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, bool Run, ServiceType ServiceType) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = Run;
			this.servType = ServiceType;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		/// <param name="Run">Indicates if you want the service to run or not on program startup.</param>
		public ServiceAttribute(string Name, string DisplayName, string Description, bool Run) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = Run;
			this.servType = ServiceType.Default;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		/// <param name="Description">The description of the service that will be displayed in the service snap-in.</param>
		public ServiceAttribute(string Name, string DisplayName, string Description) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = Description;
			this.run = true;
			this.servType = ServiceType.Default;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		/// <param name="DisplayName">The name of the service that will be displayed in the services snap-in.</param>
		public ServiceAttribute(string Name, string DisplayName) {
			this.name = Name;
			this.displayName = DisplayName;
			this.description = "";
			this.run = true;
			this.servType = ServiceType.Default;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}

		///<summary>Describes a new Win32 service.</summary>
		/// <param name="Name">The name of the service used in the service database.</param>
		public ServiceAttribute(string Name) {
			this.name = Name;
			this.displayName = Name; //If no display name is specified, then make it the same as the name...
			this.description = "";
			this.run = true;
			this.servType = ServiceType.Default;
			this.servAccessType = ServiceAccessType.AllAccess;
			this.servStartType = ServiceStartType.AutoStart;
			this.servErrorControl = ServiceErrorControl.Normal;
			this.servControls = ServiceControls.Default;
			this.logName = "Services";
		}
		#endregion

		#region Properties
		///<summary>The name of the service used in the service database.</summary>
		public string Name { get { return this.name; } set { this.Name = value; } }
		///<summary>The name of the service that will be displayed in the services snap-in.</summary>
		public string DisplayName { get { return this.displayName; } set { this.displayName = value; } }
		///<summary>The description of the service that will be displayed in the service snap-in.</summary>
		public string Description { get { return this.description; } set { this.description = value; } }
		///<summary>Indicates if you want the service to run or not on program startup.</summary>
		public bool Run { get { return this.run; } set { this.run = value; } }
		///<summary>Indicates if you want to attempt to automatically install the service if it doesn't already exist.</summary>
		public bool AutoInstall { get { return this.autoInstall; } set { this.autoInstall = value; } }
		///<summary>Indicates the type of service you want to run.</summary>
		public ServiceType ServiceType { get { return this.servType; } set { this.servType = value; } }
		///<summary>Access to the service. Before granting the requested access, the system checks the access token of the calling process.</summary>
		public ServiceAccessType ServiceAccessType { get { return this.servAccessType; } set { this.servAccessType = value; } }
		///<summary>Service start options.</summary>
		public ServiceStartType ServiceStartType { get { return this.servStartType; } set { this.servStartType = value; } }
		///<summary>Severity of the error, and action taken, if this service fails to start.</summary>
		public ServiceErrorControl ServiceErrorControl { get { return this.servErrorControl; } set { this.servErrorControl = value; } }
		///<summary>The controls or actions the service responds to.</summary>
		public ServiceControls ServiceControls { get { return this.servControls; } set { this.servControls = value; } }
		///<summary>The name of the log you want to write to.</summary>
		public string LogName { get { return this.logName; } set { this.logName = value; } }
		#endregion
	}
}
