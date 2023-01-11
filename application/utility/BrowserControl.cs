/******************************************************************************
BrowserControl.cs

begin      : July 25, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;

namespace ApplicationUtility
{
    /// <summary>
    /// A Form that contains a WebBrowser control to display WIKI help files 
    /// to the user.
    /// </summary>
    // Note: For licensing and Distribution info, see 
    // http://msdn2.microsoft.com/en-us/library/aa770040.aspx
    // Security permissions applied to this code, where "Internet" specifies 
    // the default policy permission suitable for content from 
    // unknown origin
    [PermissionSet(SecurityAction.Demand, Name = "Internet")]

    // indicating that this class is visible to COM (only public 
    // variables/methods]
    [System.Runtime.InteropServices.ComVisibleAttribute(true)]
    public partial class BrowserControl : Form
    {
        /// <summary>
        /// Reference to this form.
        /// </summary>
        private static BrowserControl m_control;
        
        /// <summary>
        /// An object to lock the loading and unloading stages of this form against
        /// unsafe thread action.
        /// </summary>
        private static object m_lock = new object();

        /// <summary>
        /// Empty constructor
        /// </summary>
        public BrowserControl()
        {
            InitializeComponent();

            // TODO: find another way to prevent script errors, this
            // disables the script errors dialog box; side-effect is that it 
            // also disables the dialog for logging into secure websites 
            // with user certificates
            m_webBrowser.ScriptErrorsSuppressed = true;

            // Set position and maximize settings.
            ApplicationUtility.FormState.RememberFormState(this);
        }

        ~BrowserControl()
        {
            lock (m_lock)
            {
                m_control = null;
            }
        }

        /// <summary>
        /// Accepts and navigates to the URL provided.
        /// </summary>
        /// <param name="url">
        /// The URL to navigate to.
        /// </param>
        public static void Navigate(string url)
        {
            url = url.Replace(" ", "_");
            bool initializing = false;
            lock (m_lock)
            {
                if ((m_control == null) || m_control.IsDisposed)
                {
                    initializing = true;
                    m_control = new BrowserControl();
                    m_control.Text = Properties.Resources.HelpTitle;
                    m_control.Visible = false;
                    m_control.Show();
                }
            }

            new System.Threading.Thread(delegate()
            {
                System.Threading.Thread.CurrentThread.Name = "Help Focus Helper";
                System.Threading.Thread.CurrentThread.IsBackground = true;
                System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));

                m_control.Invoke(new MethodInvoker(
                    delegate()
                    {
                        lock (m_lock)
                        {
                            Uri currentUri = m_control.m_webBrowser.Url;
                            if (initializing)
                            {
                                m_control.Visible = true;
                                m_control.m_webBrowser.Navigate(url);
                            }
                            else if ((currentUri == null) || 
                                !currentUri.ToString().Equals(url))
                            {
                                m_control.m_webBrowser.Navigate(url);
                            }
                            m_control.BringToFront();
                            m_control.Focus();
                        }
                    }
                    ));
            }).Start();
        }        
    }
}