/******************************************************************************
HelpSystem.cs

begin      : July 25, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;

namespace ApplicationUtility
{
    // Note: In the future this class may determine whether to retrieve help
    // file from local cache, network or Internet

    /// <summary>
    /// Class that accepts requests from the application for viewing 
    /// context-sensitive help.
    /// </summary>
    public class HelpSystem
    {
        // Note: The URL to the User Manual remains the same for all languages 
        // but the WIKI namespace will be different for different languages.
        /// <summary>
        /// Return the URL of the Help WIKI namespace (culture-specific)
        /// </summary>
        public static string GetHelpURL()
        {
#if true
            return Properties.Resources.WIKIUrl;
#else
            // TODO: Add multi-language support.
            string url = string.Empty;

            // Show the appropriate User Manual based on the current Language 
            // Culture Name.  For a complete list of Language Culture Names 
            // see http://msdn2.microsoft.com/en-us/library/ms970637.aspx
            switch (CultureInfo.CurrentCulture.Name)
            {
                case "fr-CA":
                    // french (canada)
                    System.Diagnostics.Debug.Assert(false);
                    break;
                default:
                    // en-US
                    url = Properties.Resources.WIKIUrl;
                    break;
            }

            return url;
#endif
        }

        public static void ShowHelp(string strHelpFile)
        {
            BrowserControl.Navigate(GetHelpURL() + strHelpFile);
        }

        ///// <summary>
        ///// Note: This method will in the future take the process id provided 
        ///// and find the URL to the help file associated with it
        ///// </summary>
        ////public static string ProcIDToHelpFile(string strProcID)
        ////{
        ////    return string.Empty;
        ////}
    }
}
