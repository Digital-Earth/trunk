using System;
using System.IO;
using System.Net;
using System.Reflection;
using System.Runtime.ExceptionServices;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    /// <summary>
    /// This class is responsible for creating crash dumps on unhandled exceptions and upload them to given server. 
    /// 
    /// This class is intended to be used in the following manner:
    /// 
    /// static void Main(string[] args)
    /// {
    ///     var assmeblyName = Assembly.GetEntryAssembly().GetName();
    ///
    ///     using (var crashReporter = new CrashReporter(
    ///                assmeblyName.Name,
    ///                assmeblyName.Version.ToString(),
    ///                Settings.Default.CrashDumpUploadUri)
    ///           )
    ///     {
    ///         //run your program here.
    ///     }
    /// }
    /// </summary>
    public class CrashReporter : IDisposable
    {
        public string ServerUrl { get; private set; }
        public string ApplicationName { get; private set; }
        public string ApplicationVersion { get; private set; }

        /// <summary>
        /// if FailFast is set to true, CrashReporter will use Enviroment.FailFast(message) to terminate the application.
        /// 
        /// This is useful for Console applications to avoid "Application Failed" window message box
        /// </summary>
        public bool FailFast { get; set; }



        [DllImport("kernel32.dll")]
        static extern ErrorModes SetErrorMode(ErrorModes uMode);

        [Flags]
        internal enum ErrorModes : uint
        {
            SYSTEM_DEFAULT = 0x0,
            SEM_FAILCRITICALERRORS = 0x0001,
            SEM_NOGPFAULTERRORBOX = 0x0002,
            SEM_NOALIGNMENTFAULTEXCEPT = 0x0004,
            SEM_NOOPENFILEERRORBOX = 0x8000
        }

        [DllImport("wer.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern int WerAddExcludedApplication(String pwzExeName, bool bAllUsers);
        
        [DllImport("wer.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern int WerRemoveExcludedApplication(String pwzExeName, bool bAllUsers);

        /// <summary>
        /// DisableWindowsErrorReportingDialog will stop windows from using WER (Windows Error Reporting).
        /// This function will make the following changes:
        ///   1) Add the Entry Assembly name into the WER excluded application list
        ///   2) change SetErrorMode to not display Fault Errorbox
        ///   3) Enable FailFast option - kill the current process and will log the managed stack trace into Windows Event.
        /// </summary>
        public void DisableWindowsErrorReportingDialog()
        {
            SetErrorMode(ErrorModes.SEM_NOGPFAULTERRORBOX | ErrorModes.SEM_FAILCRITICALERRORS);
            WerAddExcludedApplication(System.IO.Path.GetFileName(Assembly.GetEntryAssembly().Location), false);
            FailFast = true;
        }

        /// <summary>
        /// EnableWindowsErrorReportingDialog will revent the actions done by DisableWindowsErrorReportingDialog
        /// </summary>
        public void EnableWindowsErrorReportingDialog()
        {
            SetErrorMode(ErrorModes.SYSTEM_DEFAULT);
            WerRemoveExcludedApplication(System.IO.Path.GetFileName(Assembly.GetEntryAssembly().Location), false);
            FailFast = false;
        }

        private string m_crashDumpDir;
        private string m_crashDumpExtension = ".mdmp";

        public CrashReporter(string application, string version, string server,bool autoEnable = true)
        {
            ApplicationName = application ?? AppDomain.CurrentDomain.FriendlyName;
            ApplicationVersion = version;
            ServerUrl = server;

            FailFast = false;

            if (PrepareCrashDumpDirectory())
            {
                if (autoEnable)
                {
                    Enabled = true;
                }

                //start upload old crash dump files....
                Task.Factory.StartNew(UploadOldCrashDumpFiles);
            }
        }

        private bool m_enabled = false;

        public bool Enabled
        {
            get
            {
                return m_enabled;
            }
            set
            {
                if (m_enabled != value)
                {
                    m_enabled = value;
                    if (m_enabled)
                    {
                        AppDomain.CurrentDomain.UnhandledException += HandleUnhandledException;
                    }
                    else
                    {
                        AppDomain.CurrentDomain.UnhandledException -= HandleUnhandledException;
                    }
                }
            }
        }
        
        private bool PrepareCrashDumpDirectory()
        {
            try
            {
                m_crashDumpDir = Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                    "PYXIS", 
                    "CrashDumps");

                Directory.CreateDirectory(m_crashDumpDir);
                return true;
            }
            catch (Exception)
            {
                System.Diagnostics.Trace.WriteLine("error: failed to create crash dump directory");
                return false;
            }
        }

        private void UploadOldCrashDumpFiles()
        {
            foreach (var file in Directory.EnumerateFiles(m_crashDumpDir, "*" + m_crashDumpExtension))
            {
                UploadCrashFile(file);
            }
        }

        private void UploadCrashFile(string file)
        {
            try
            {
                var webClient = new WebClient();
                webClient.UploadFile(ServerUrl, "POST", file);
                File.Delete(file);
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("error: failed to upload crash dump " + file + " due to error: " + e.Message);
            }
        }

        public void CreateDump(Exception exception,MiniDump.ExceptionInfo exceptionInfo)
        {
            // Name of the dump file
            var fileName = ".unknown-crash";

            // Try to extract information about the fault
            if (exception != null)
            {
                if (exception.TargetSite != null)
                {
                    // Add the fault place information to the file name
                    fileName = "_" + exception.TargetSite.Module.Name;
                    var method = exception.TargetSite as System.Reflection.MethodBase;
                    if (method != null)
                    {
                        fileName += "." + method.Name;
                    }
                }
            }

            // Include the product name and version in the file name
            // to help the developer quickly establish the dump's actuality
            fileName = ApplicationName + "." + ApplicationVersion + fileName;
            // Include the time information as well
            fileName += DateTime.UtcNow.ToString(".yyyy.MM.dd-HH.mm.ss") + m_crashDumpExtension;

            // Write the dump data
            string filePath = Path.Combine(m_crashDumpDir, fileName);
            using (var fs = new FileStream(filePath, FileMode.Create, FileAccess.ReadWrite, FileShare.Write))
            {
                MiniDump.Write(fs.SafeFileHandle,
                    MiniDump.Option.ScanMemory | MiniDump.Option.WithIndirectlyReferencedMemory,
                    exceptionInfo);
            }
        }

        [HandleProcessCorruptedStateExceptions]
        [SecurityCritical]
        private void HandleUnhandledException(object sender, UnhandledExceptionEventArgs earg)
        {
            if (earg.IsTerminating)
            {
                // Try to extract information about the fault
                if (earg.ExceptionObject != null)
                {
                    var eobj = earg.ExceptionObject as Exception;

                    CreateDump(eobj, MiniDump.ExceptionInfo.Present);
                }
                else
                {
                    CreateDump(null, MiniDump.ExceptionInfo.Present);
                }

                if (FailFast)
                {
                    //plan 1: try to FailFast on a new thread.
                    Task.Factory.StartNew(() =>
                    {
                        Environment.FailFast(ApplicationName + " (Version : " + ApplicationVersion + ") failed due to unhandled exception.");
                    });

                    //plan 2: try to FailFast on current thread
                    while (true)
                    {
                        Environment.FailFast(ApplicationName + " (Version : " + ApplicationVersion + ") failed due to unhandled exception.");                        
                    }                 
                }
            }
        }

        public void Dispose()
        {
            Enabled = false;   
        }
    }
}
