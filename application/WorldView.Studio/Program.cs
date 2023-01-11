using System;
using System.Windows.Forms;
using Pyxis.Utilities;
using Pyxis.WorldView.Studio.Properties;

namespace Pyxis.WorldView.Studio
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            using (var crashReporter = new CrashReporter(
                Application.ProductName,
                Application.ProductVersion,
                Settings.Default.CrashDumpUploadUri)
            )
            {
                using (DDEManager ddeManager = new DDEManager())
                {
                    // try to find another instance of Studio and send the args to that studio if exists
                    if (ddeManager.Initialize(args) == false)
                    {
                        //we forward the args to the other instance, simply quit
                        return;
                    }

                    // Run the application verifier, that checks that the current version is compatible with the system state
                    if (!SystemRequirementsVerifier.CheckApplication())
                    {
                        // If it failed, the UI shoul not be run
                        return;
                    }

                    Application.EnableVisualStyles();
                    Application.SetCompatibleTextRenderingDefault(false);
                    try
                    {
                        Application.Run(new ApplicationForm(crashReporter, ddeManager, args));
                    }
                    catch (Exception e)
                    {
                        // A network connection failure is pretty much the only possible reason for this exception
                        MessageBox.Show("WorldView Studio was unable to start. Please check your network connection and try again.", "PYXIS WorldView Studio");
                        Trace.error(e.Message);
                    }

                    ddeManager.Unregister();
                }
            }
        }
    }
}
