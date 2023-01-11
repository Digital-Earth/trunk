using Pyxis.Contract.Publishing;
using Pyxis.Utilities;
using StudioLauncher.Properties;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Forms;

namespace StudioLauncher
{
    internal class Program
    {
        // STAThread required for FileBrowserDialog in ControlMode dialog
        [STAThread]
        private static void Main(string[] args)
        {
            Trace.TraceInformation("Starting the StudioLauncher.");

            AppDomain.CurrentDomain.UnhandledException += UnhandledExceptionsHandler;

            // Exit if WorldView Studio is already running
            if (DDEManager.SendMessageToServer("WorldView.Studio", args))
            {
                Trace.TraceInformation("Exiting: WorldView Studio is already running.");
                return;
            }

            // Exit if multiple copies of the launcher are running
            if (Process.GetProcessesByName("StudioLauncher").Length > 1)
            {
                Trace.TraceInformation("Exiting: A copy of the launcher is already running.");
                return;
            }

            var versionManager = new ProductVersionManager(ProductType.WorldViewStudio, "WorldView.Studio.exe",
                new RestVersionServer(Settings.Default.RestAPIVersionServerAddress),
                new LocalVersionServer());

            // Check if the launcher was started in control mode (ControlMode == True in config file)
            if (Settings.Default.ControlMode)
            {
                HandleControlMode(versionManager, args);
            }
            else
            {
                // Run the local version of WorldView Studio even if an update is available. The
                // updated version will be downloaded and run next time the launcher is run.
                bool localVersionIsRunning = versionManager.TryRunTheLocalVersion(args);

                if (Process.GetProcessesByName("StudioLauncher").Length <= 1)
                {
                    try
                    {
                        if (!versionManager.UpdateAvailable())
                        {
                            if (!localVersionIsRunning)
                            {
                                throw new Exception("Unable to start WorldView Studio.");
                            }
                            Trace.TraceInformation("Exiting: Latest version exists locally.");
                            return;
                        }

                        Trace.TraceInformation("A new version is available.");
                        versionManager.DownloadUpdate(localVersionIsRunning, args);
                    }
                    catch (Exception e)
                    {
                        TraceException(e);
                        ShowErrorMessage("An exception happened while checking for updates");
                    }
                }
            }

            Trace.TraceInformation("Exiting the StudioLauncher.");
        }

        /// <summary>
        /// Display the control mode dialog allowing the user to select the version of WorldView Studio
        /// and the startup URL they would like to use.
        /// </summary>
        /// <param name="versionManager">The version manager.</param>
        /// <param name="args">The command line arguments passed to this application.</param>
        private static void HandleControlMode(ProductVersionManager versionManager, string[] args)
        {
            Trace.TraceInformation("Control mode selected.");

            // Display a dialog allowing the user to run a specific version of the Studio
            // against a specific startup URL.
            List<Product> products = versionManager.GetAllVersions();

            string url = null;
            args = ArgsParser.Parse(args, new ArgsParser.Option(Settings.Default.StartupArg, x => url = x));

            ControlModeForm controlDialog = new ControlModeForm(products, url ?? Settings.Default.StagingStartupURL);

            if (controlDialog.ShowDialog() == DialogResult.OK)
            {
                Product selectedVersion = controlDialog.SelectedVersion;

                // pass the startup URL and test directory to the Studio as command line arguments
                var newArgs = new List<string>(args);
                newArgs.Add("--" + Settings.Default.StartupArg + ":" + controlDialog.StartupURL);

                if (!String.IsNullOrWhiteSpace(controlDialog.TestDirectory))
                {
                    // enclose directory in double quotes in case there are spaces
                    newArgs.Add("--" + Settings.Default.TestDirectoryArg + ":\"" + controlDialog.TestDirectory + "\"");    
                }

                if (controlDialog.ClearCache)
                {
                    newArgs.Add("--" + Settings.Default.ClearCacheArg);
                }

                args = newArgs.ToArray();

                // Run the version from the user's disk if it exists
                if (!versionManager.TryRunTheLocalVersion(selectedVersion, args))
                {
                    try
                    {
                        // Version is not on user's computer, download and run it
                        versionManager.DownloadAndRunVersion(selectedVersion, args);
                    }
                    catch (Exception e)
                    {
                        TraceException(e);
                        ShowErrorMessage("An exception happened while updating WorldView Studio");
                    }
                }
            }
            controlDialog.Dispose();
        }

        private static void UnhandledExceptionsHandler(object sender, UnhandledExceptionEventArgs e)
        {
            TraceException((Exception)e.ExceptionObject);
            ShowErrorMessage("An unexpected error happened trying to launch WorldView Studio. See the log file for more details.");
        }

        private static void ShowErrorMessage(string message)
        {
            MessageBox.Show(message, @"WorldView Studio");
        }

        private static void TraceException(Exception exception)
        {
            var innerException = exception.InnerException;
            if (innerException != null)
            {
                TraceException(innerException);
            }

            var aggEx = exception as AggregateException;
            if (aggEx != null)
            {
                foreach (var childException in aggEx.InnerExceptions)
                {
                    TraceException(childException);
                }
            }
            Trace.TraceError("Exception Type:\n" + exception.GetType().Name);
            Trace.TraceError("Exception Message:\n" + exception.Message);
            Trace.TraceError("Stack Trace:\n" + exception.StackTrace);
        }
    }
}