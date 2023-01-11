using System;
using System.IO;
using System.Windows.Forms;
using Pyxis.WorldView.Studio.Properties;

namespace Pyxis.WorldView.Studio
{
    /// <summary>
    /// Performs verification for known cases of possible incompatibility of WorldView Studio with the operating system,
    /// and allows in the case to shut down the application and return the user a proper message.
    /// </summary>
    internal class SystemRequirementsVerifier
    {
        /// <summary>
        /// Consequently runs verification for all known problematic cases
        /// </summary>
        /// <returns>false on a first failure<br/>
        /// true if all verifications pass</returns>
        public static bool CheckApplication()
        {
            // Run verification here on all the known specific cases we are aware of
            return CheckMfcLibraryPresence();
        }

        /// <summary>
        /// Checks if the OS satisfies the dependencies of the application on MFC libraries
        /// </summary>
        /// <returns>true if the required MFC resources are present, otherwise false</returns>
        private static bool CheckMfcLibraryPresence()
        {
            // The Studio needs mfc110u.dll
            // If it's installed, it may be located in two places, depending on the system bitness
            var windir = Environment.GetEnvironmentVariable("windir");
            if (windir == null)
            {
                throw new Exception("Failed to obtain value for environment variable windir%");
            }
            if (
                !File.Exists(Path.Combine(windir, @"SysWOW64\mfc110u.dll"))
                && !File.Exists(Path.Combine(windir, @"System32\mfc110u.dll"))
                )
            {
                // A custom message for this particular case
                var message = "A new version of WorldView Studio was recently released. " +
                              "To support the important functionality introduced in this release we had to make some fundamental architectural changes. " +
                              "As a result, you will need to download and run a new installer so that you can continue using and enjoying WorldView Studio." +
                              Environment.NewLine + Environment.NewLine +
                              "Please do the following:" +
                              Environment.NewLine +
                              "1. Click OK below to open WorldView Studio download page." +
                              Environment.NewLine +
                              "2. Save the new installer on your computer." +
                              Environment.NewLine +
                              "3. Run the downloaded installer." +
                              Environment.NewLine + Environment.NewLine +
                              "The entire process should only take a few minutes. We understand that this is an inconvenience, and appreciate your patience." +
                              Environment.NewLine + Environment.NewLine +
                              "Thank you.";

                // Explain the user what happened and give an option to update the installer or exit
                if (
                    MessageBox.Show(
                    message,
                    @"WorldView Studio",
                    MessageBoxButtons.OKCancel
                    ) == DialogResult.OK)
                {
                    System.Diagnostics.Process.Start(Settings.Default.ApplicationDownloadUrl);
                }
                // Let the caller know that the application needs to be shut down
                return false;
            }
            // Declare success
            return true;
        }
    }
}