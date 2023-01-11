#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2005, 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;
using System.Reflection;
using HoytSoft.Common.UAC;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

namespace HoytSoft.Common.Services {
	public abstract class ServiceBase : IDisposable {
		#region Enums
		private enum ArgumentAction : byte {
			Nothing		= 0, 
			Install		= 1,
			Uninstall	= 2
		}
		#endregion

		#region Variables
		private bool initialized = false;
		private bool disposed = false;
		private bool debug = false;
		
		private string[] args = null;
		private bool run;
		private string name;
		private string displayName;
		private string description;
		private ServiceType servType;
		private ServiceAccessType servAccessType;
		private ServiceStartType servStartType;
		private ServiceErrorControl servErrorControl;
		private ServiceControls servControls;

		private IntPtr servStatusHandle;
		private Native.SERVICE_STATUS servStatus;
		private ServiceState servState = ServiceState.Stopped;
		private Native.ServiceCtrlHandlerExProc servCtrlHandlerExProc;

		private string logName;
		private System.Diagnostics.EventLog log;

		private ManualResetEvent stopEvent = null;
		#endregion

		#region Constructors
		public ServiceBase() {
			this.debug = false;
			this.servCtrlHandlerExProc = new Native.ServiceCtrlHandlerExProc(this.serviceControlHandlerEx);
			this.servStatus = new Native.SERVICE_STATUS();
			this.servState = ServiceState.Stopped;
		}

		[MethodImpl(MethodImplOptions.Synchronized)]
		protected void init() {
			if (this.initialized)
				return;
			this.initialized = true;

			ServiceAttribute serviceDef = this.ServiceAttribute;
			this.name = serviceDef.Name;
			this.displayName = serviceDef.DisplayName;
			this.description = serviceDef.Description;
			this.run = serviceDef.Run;
			this.servType = serviceDef.ServiceType;
			this.servAccessType = serviceDef.ServiceAccessType;
			this.servStartType = serviceDef.ServiceStartType;
			this.servErrorControl = serviceDef.ServiceErrorControl;
			this.servControls = serviceDef.ServiceControls;
			this.logName = serviceDef.LogName;
		}
		#endregion

		#region Properties
		internal virtual ServiceAttribute ServiceAttribute {
			get {
				ServiceAttribute[] attribs = (ServiceAttribute[])GetType().GetCustomAttributes(Utilities.SERVICE_ATTRIBUTE_TYPE, true);
				if (attribs == null || attribs.Length <= 0)
					return null;
				return attribs[0];
			}
		}

		///<summary>The name of the service used in the service database.</summary>
		public string Name { get { return this.name; } }
		///<summary>The name of the service that will be displayed in the services snap-in.</summary>
		public string DisplayName { get { return this.displayName; } }
		///<summary>The description of the service that will be displayed in the service snap-in.</summary>
		public string Description { get { return this.description; } }
		///<summary>Indicates if you want the service to run or not on program startup.</summary>
		public bool Run { get { return this.run; } }
		///<summary>Indicates the type of service you want to run.</summary>
		public ServiceType ServiceType { get { return this.servType; } }
		///<summary>Access to the service. Before granting the requested access, the system checks the access token of the calling process.</summary>
		public ServiceAccessType ServiceAccessType { get { return this.servAccessType; } }
		///<summary>Service start options.</summary>
		public ServiceStartType ServiceStartType { get { return this.servStartType; } }
		///<summary>Severity of the error, and action taken, if this service fails to start.</summary>
		public ServiceErrorControl ServiceErrorControl { get { return this.servErrorControl; } }
		///<summary>The controls or actions the service responds to.</summary>
		public ServiceControls ServiceControls { get { return this.servControls; } }
		///<summary>The current state of the service.</summary>
		public ServiceState ServiceState { get { return this.servState; } }
		///<summary>Treats the service as a console application instead of a normal service.</summary>
		public bool Debug { get { return this.debug; } }
		#endregion

		#region Override Methods
		protected virtual bool Initialize(string[] Arguments) { return true; }
		protected virtual void Start() { }
		protected virtual void Pause() { }
		protected virtual void Stop() { }
		protected virtual void Continue() { }
		protected virtual void Shutdown() { }
		protected virtual void Interrogate() { }
		protected virtual void CustomMessage(uint Code) { }
		#endregion

		#region Helper Methods
		private static ArgumentAction interpretAction(string[] args) {
			if (args == null || args.Length <= 0)
				return ArgumentAction.Nothing;
			string arg = args[0];
			if (string.IsNullOrEmpty(arg))
				return ArgumentAction.Nothing;
			switch (arg.ToLower()) {
				case "i":
				case "install":
					return ArgumentAction.Install;
				case "u":
				case "uninstall":
					return ArgumentAction.Uninstall;
				default:
					return ArgumentAction.Nothing;
			}
		}

		/*
		//
		// Purpose: 
		//   Sets the current service status and reports it to the SCM
		//
		// Parameters:
		//   dwCurrentState - The current state (see SERVICE_STATUS)
		//   dwWin32ExitCode - The system error code
		//   dwWaitHint - Estimated time for pending operation, 
		//     in milliseconds
		// 
		// Return value:
		//   None
		//
		VOID ReportSvcStatus( DWORD dwCurrentState,
					  DWORD dwWin32ExitCode,
					  DWORD dwWaitHint)
		{
		static DWORD dwCheckPoint = 1;

		// Fill in the SERVICE_STATUS structure.

		gSvcStatus.dwCurrentState = dwCurrentState;
		gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
		gSvcStatus.dwWaitHint = dwWaitHint;

		if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
		else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

		if ( (dwCurrentState == SERVICE_RUNNING) ||
		   (dwCurrentState == SERVICE_STOPPED) )
		gSvcStatus.dwCheckPoint = 0;
		else gSvcStatus.dwCheckPoint = dwCheckPoint++;

		// Report the status of the service to the SCM.
		SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
		}
		 */
		private uint checkPoint = 1;
		internal void ReportSvcStatus(uint CurrentState, uint Win32ExitCode, uint WaitHint) {
			servStatus.dwCurrentState = CurrentState;
			servStatus.dwWin32ExitCode = Win32ExitCode;
			servStatus.dwWaitHint = WaitHint;

			if (CurrentState == Native.SERVICE_START_PENDING)
				servStatus.dwControlsAccepted = 0;
			else
				servStatus.dwControlsAccepted = (uint)this.servControls;

			if (CurrentState == Native.SERVICE_RUNNING || CurrentState == Native.SERVICE_STOPPED)
				servStatus.dwCheckPoint = 0;
			else
				servStatus.dwCheckPoint = checkPoint++;
			Native.SetServiceStatus(this.servStatusHandle, ref servStatus);
		}
		#endregion

		#region Logging Methods
		private void checkLog() {
			if (this.log == null) {
				this.log = new System.Diagnostics.EventLog(this.logName);
				this.log.Source = this.displayName;
			}
		}

		public void Log(string Message) {
			try {
				checkLog();
                System.Diagnostics.Trace.WriteLine(Message);
				this.log.WriteEntry(Message);
				this.log.Close();
			} catch (System.ComponentModel.Win32Exception) {
				//In case the event log is full....
			}
            catch (System.Security.SecurityException)
            {                
            }
        }
		public void Log(string Message, System.Diagnostics.EventLogEntryType EntryType) {
            try
            {
                checkLog();
                System.Diagnostics.Trace.WriteLine(Message);
                this.log.WriteEntry(Message, EntryType);
            }
            catch (System.ComponentModel.Win32Exception)
            {
            }
            catch (System.Security.SecurityException )
            {
            }
		}
		public void Log(string Message, System.Diagnostics.EventLogEntryType EntryType, int EventID) {
			try {
				checkLog();
                System.Diagnostics.Trace.WriteLine(Message);
                this.log.WriteEntry(Message, EntryType, EventID);
			} catch (System.ComponentModel.Win32Exception) {
			}
            catch (System.Security.SecurityException )
            {
            }
        }
		public void Log(string Message, System.Diagnostics.EventLogEntryType EntryType, short Category, int EventID) {
			try {
				checkLog();
                System.Diagnostics.Trace.WriteLine(Message);
                this.log.WriteEntry(Message, EntryType, EventID, Category);
			} catch (System.ComponentModel.Win32Exception) {
			}
            catch (System.Security.SecurityException )
            {
            }
        }
		#endregion

		#region Testing Methods
		private bool inTestAction = false;

		protected void TestStop() {
			Thread t = new Thread(new ThreadStart(delegate() {
				while (this.inTestAction || this.servState == ServiceState.Interrogating)
					;
				this.inTestAction = true;
				this.servState = ServiceState.Stopped;
				this.Stop();
				this.inTestAction = false;
			}));
			t.Name = "Test Stop: " + this.displayName;
			t.IsBackground = true;
			t.Start();
		}

		protected void TestPause() {
			Thread t = new Thread(new ThreadStart(delegate() {
				if (this.servState != ServiceState.ShuttingDown) {
					while (this.inTestAction || this.servState == ServiceState.Interrogating)
						;
					this.inTestAction = true;
					this.servState = ServiceState.Paused;
					this.Pause();
					this.inTestAction = false;
				}
			}));
			t.Name = "Test Pause: " + this.displayName;
			t.IsBackground = true;
			t.Start();
		}

		protected void TestContinue() {
			Thread t = new Thread(new ThreadStart(delegate() {
				while (this.inTestAction || this.servState == ServiceState.Interrogating)
					;
				this.inTestAction = true;
				this.servState = ServiceState.Running;
				this.Continue();
				this.inTestAction = false;
			}));
			t.Name = "Test Continue: " + this.displayName;
			t.IsBackground = true;
			t.Start();
		}

		protected void TestShutdown() {
			Thread t = new Thread(new ThreadStart(delegate() {
				while (this.inTestAction || this.servState == ServiceState.Interrogating)
					;
				this.inTestAction = true;
				this.servState = ServiceState.ShuttingDown;
				this.Shutdown();
				this.inTestAction = false;
			}));
			t.Name = "Test Shutdown: " + this.displayName;
			t.IsBackground = true;
			t.Start();
		}

		protected void TestInterrogate() {
			Thread t = new Thread(new ThreadStart(delegate() {
				while (this.inTestAction)
					;
				this.inTestAction = true;
				this.servState = ServiceState.Interrogating;
				this.Interrogate();
				this.servState = ServiceState.Running;
				this.inTestAction = false;
			}));
			t.Name = "Test Interrogate: " + this.displayName;
			t.IsBackground = true;
			t.Start();
		}

		protected void TestCustomMessage(uint Code) {
			Thread t = new Thread(new ParameterizedThreadStart(delegate(object param) {
				while (this.inTestAction)
					;
				this.inTestAction = true;
				this.servState = ServiceState.Running;
				this.CustomMessage((uint)param);
				this.servState = ServiceState.Running;
				this.inTestAction = false;
			}));
			t.Name = "Test Custom Message (" + Code + "): " + this.displayName;
			t.IsBackground = true;
			t.Start(Code);
		}
		#endregion

		#region IDisposable Members
		protected virtual void DisposeService() {
		}

		public bool IsDisposed {
			get { return this.disposed; }
		}

		[MethodImpl(MethodImplOptions.Synchronized)]
		public void Dispose() {
			if (this.disposed)
				return;
			this.disposed = true;

			if (!string.IsNullOrEmpty(name) && Utilities.IsServiceInstalled(this.Name) && Utilities.IsServiceRunning(this.Name))
				Utilities.StopService(this.Name);

			if (stopEvent != null)
				stopEvent.Close();

			DisposeService();

			if (this.log != null) {
				this.log.Close();
				this.log.Dispose();
				this.log = null;
			}
		}

		~ServiceBase() {
			Dispose();
		}
		#endregion

		#region Overloaded Methods
		///<summary>Run all services in the executing assembly.</summary>
		public static void RunServices(string[] Args) {
			RunServices(Args, Assembly.GetEntryAssembly());
		}

		///<summary>Run the service defined by ServiceType.</summary>
		public static void RunService(string[] Args, Type ServiceType) {
			RunServices(Args, new Type[] { ServiceType });
		}

		///<summary>Run the service.</summary>
		public static void RunService(string[] Args, ServiceBase Service) {
			RunServices(Args, new ServiceBase[] { Service });
		}

		///<summary>Run all services in the assembly.</summary>
		public static void RunServices(string[] Args, Assembly Assembly) {
			if (Assembly == null) throw new ServiceException("No currently executing assembly.");
			RunServices(Args, Utilities.FindAllValidServices(Assembly));
		}
		#endregion

		#region Entry Point
		///<summary>If you do not want to specify your own main entry point, you can use this one.</summary>
		public static void Main(string[] Args) {
			RunServices(Args);
		}
		#endregion

		#region Public Static Methods
		///<summary>Executes your service. If multiple services are defined in the assembly, it will run them all in separate threads.</summary>
		/// <param name="Args">The arguments passed in from the command line.</param>
		/// <param name="Types">An array of types we want to inspect for services.</param>
		public static void RunServices(string[] Args, Type[] Types) {
			List<ServiceBase> services = new List<ServiceBase>(1);

			foreach (Type t in Types) {
				if (!Utilities.IsValidServiceType(t))
					continue;
				services.Add((ServiceBase)Activator.CreateInstance(t));
			}

			if (services.Count > 0)
				RunServices(Args, services.ToArray());
		}

		public static void RunServices(string[] Args, ServiceBase[] Services) {
			#region Setup variables
			if (Args == null)
				Args = new string[0];
			string path = Assembly.GetEntryAssembly().Location;
			ArgumentAction action = interpretAction(Args);
			TaskResult taskResult = null;
			List<Native.SERVICE_TABLE_ENTRY> dispatchTables = new List<Native.SERVICE_TABLE_ENTRY>(1);
			#endregion

			foreach(ServiceBase service in Services) {
				#region Validate type
				if (service == null)
					continue;
				ServiceAttribute serviceDef = service.ServiceAttribute;
				if (serviceDef == null)
					continue;
				#endregion

				#region Attempt to install, if necessary
				//Did the developer specify to automatically install this? If not, do nothing!
				if (action == ArgumentAction.Install || (action != ArgumentAction.Uninstall && serviceDef.AutoInstall && !Utilities.IsServiceInstalled(serviceDef)))
					if ((taskResult = Utilities.InstallService(path, serviceDef)) == null || taskResult.Result != Result.Success)
						throw new ServiceInstallException("Unable to install service: " + serviceDef.DisplayName);
				#endregion

				#region Check for uninstall action, if necessary
				if (action == ArgumentAction.Uninstall && Utilities.IsServiceInstalled(serviceDef))
					if ((taskResult = Utilities.UninstallService(serviceDef)) == null || taskResult.Result != Result.Success)
						throw new ServiceUninstallException("Unable to uninstall service: " + serviceDef.DisplayName);
				#endregion

				//Skip to next if we just want to install/uninstall
				if (action != ArgumentAction.Nothing)
					continue;

				//Make sure we want to run this service
				if (!serviceDef.Run)
					continue;

				Native.SERVICE_TABLE_ENTRY entry = new Native.SERVICE_TABLE_ENTRY();
				entry.lpServiceName = serviceDef.Name;
				entry.lpServiceProc = new Native.ServiceMainProc(service.serviceMain);
				dispatchTables.Add(entry);
				service.debug = false;
			}

			if (dispatchTables.Count > 0) {
				//Add a null entry to tell the API it's the last entry in the table...
				Native.SERVICE_TABLE_ENTRY entry = new Native.SERVICE_TABLE_ENTRY();
				entry.lpServiceName = null;
				entry.lpServiceProc = null;
				dispatchTables.Add(entry);

				//Send dispatch table to service control manager so it knows where to call the main func.
				Native.SERVICE_TABLE_ENTRY[] table = (Native.SERVICE_TABLE_ENTRY[])dispatchTables.ToArray();
				if (!Native.StartServiceCtrlDispatcher(table)) {
					//There was an error. What was it?
					switch (Marshal.GetLastWin32Error()) {
						case Native.ERROR_INVALID_DATA:
							throw new ServiceStartupException("The specified dispatch table contains entries that are not in the proper format.");
						case Native.ERROR_SERVICE_ALREADY_RUNNING:
							throw new ServiceStartupException("A service is already running.");
						case Native.ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
							//"A service is being run as a console application. Try setting the Service attribute's \"Debug\" property to true if you're testing an application."
							//If we've started up as a console/windows app, then we'll get this error in which case we treat the program
							//like a normal app instead of a service and start it up in "debug" mode...
							foreach (ServiceBase service in Services) {
								service.debug = true;
								service.args = Args;
								Thread t = new Thread(new ThreadStart(service.serviceDebugMain));
								t.Name = "Service: " + service.ServiceAttribute.DisplayName;
								t.IsBackground = false;
								t.Start();
							}/**/
							break;
						default:
							throw new ServiceStartupException("An unknown error occurred while starting up the service(s).");
					}
				}
			}
		}
		#endregion

		#region Service Methods
		static void msg(string msg) {
			File.AppendAllText(Path.Combine(@"C:\Users\David\Desktop", "log.txt"), msg + "\r\n");
		}

		private void serviceMain(uint argc, string[] argv) {
			init();

			this.servStatusHandle = Native.RegisterServiceCtrlHandlerEx(this.name, this.servCtrlHandlerExProc);

			//These SERVICE_STATUS members remain as set here.
			servStatus.dwServiceType = (uint)this.servType;
			servStatus.dwServiceSpecificExitCode = 0;

			//Report initial status to the SCM.
			ReportSvcStatus(Native.SERVICE_START_PENDING, Native.NO_ERROR, 3000);

			stopEvent = new ManualResetEvent(false);

			if (!this.Initialize(argv)) {
				ReportSvcStatus(Native.SERVICE_STOPPED, Native.NO_ERROR, 0);
				return;
			}

			ReportSvcStatus(Native.SERVICE_RUNNING, Native.NO_ERROR, 0);
			this.Start();

			while (true) {
				stopEvent.WaitOne();
				ReportSvcStatus(Native.SERVICE_STOPPED, Native.NO_ERROR, 0);
			}
		}

		private void serviceDebugMain() {
            init(); //--nle--
			if (this.Initialize(this.args)) {
				this.servState = ServiceState.Running;
				this.Start();
				//Wait for all tests to finish...
				//while(this.inTestAction)
				//	;
				//this.TestStop();
			}
		}

		private uint serviceControlHandlerEx(uint OpCode, uint EventType, IntPtr EventData, IntPtr Context) {
			switch (OpCode) {
				case Native.SERVICE_CONTROL_STOP:
					servState = ServiceState.Stopped;
					ReportSvcStatus(Native.SERVICE_STOP_PENDING, Native.NO_ERROR, 3000);
					try {
						this.Stop();
					} catch (Exception e) {
						this.Log("An exception occurred while trying to stop the service:" + e);
					}
					stopEvent.Set();
					break;
				case Native.SERVICE_CONTROL_PAUSE:
					servState = ServiceState.Paused;
					ReportSvcStatus(Native.SERVICE_PAUSE_PENDING, Native.NO_ERROR, 3000);
					try {
						this.Pause();
					} catch (Exception e) {
						this.Log("An exception occurred while trying to pause the service:" + e);
					}
					servStatus.dwCurrentState = Native.SERVICE_PAUSED;
					break;
				case Native.SERVICE_CONTROL_CONTINUE:
					servState = ServiceState.Running;
					ReportSvcStatus(Native.SERVICE_CONTINUE_PENDING, Native.NO_ERROR, 3000);
					try {
						this.Continue();
					} catch (Exception e) {
						this.Log("An exception occurred while trying to continue the service:" + e);
					}
					servStatus.dwCurrentState = Native.SERVICE_RUNNING;
					break;
				case Native.SERVICE_CONTROL_SHUTDOWN:
					servState = ServiceState.Running;
					ReportSvcStatus(Native.SERVICE_STOP_PENDING, Native.NO_ERROR, 3000);
					try {
						this.Shutdown();
					} catch (Exception e) {
						this.Log("An exception occurred while trying to shutdown the service:" + e);
					}
					stopEvent.Set();
					break;
				case Native.SERVICE_INTERROGATE:
					servState = ServiceState.Interrogating;
					try {
						this.Interrogate();
					} catch (Exception e) {
						this.Log("An exception occurred while trying to interrogate the service:" + e);
					}
					break;
				default:
					if (OpCode >= 128 && OpCode <= 255)
						try {
							this.CustomMessage(OpCode);
						} catch (Exception e) {
							this.Log("An exception occurred while trying to interrogate the service:" + e);
						}
					break;
			}

			ReportSvcStatus(servStatus.dwCurrentState, Native.NO_ERROR, 0);
			return Native.NO_ERROR;
		}
		#endregion
	}
}
