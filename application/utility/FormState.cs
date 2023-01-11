/******************************************************************************
FormState.cs

begin      : 19/07/2007 9:13:10 AM
web        : www.pyxisinnovation.com

This class was taken directly from the Code Poject page:
http://www.codeproject.com/useritems/FormState.asp
Refer to the SVN history for any local modifications.
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    using System;
    using System.Drawing;
    using Microsoft.Win32;
    using System.Windows.Forms;

    /// <summary>
    /// Create an instance of this class in the constructor of a form. 
    /// Will save and restore window state, size, and position.
    /// Uses DesktopBounds (instead of just Form.Location/Size) 
    /// to place window correctly on a multi screen desktop.
    /// </summary>
    public class FormState
    {
        /// <summary>
        /// Remembers the state of the form.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <returns></returns>
        public static FormState RememberFormState(System.Windows.Forms.Form form)
        {
            return RememberFormState(form,
                form.Name);
        }
        
        /// <summary>
        /// Remembers the state of the form.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <param name="name">
        /// The name to save the form settings under (defaults to form.Name).
        /// </param>
        /// <returns></returns>
        public static FormState RememberFormState(System.Windows.Forms.Form form, string name)
        {
            return new FormState(form, name);
        }

        /// <summary>
        /// Remembers the state of the form.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <param name="ignoreSize">
        /// Whether to not save/remember the form's size.
        /// </param>
        /// <returns></returns>
        public static FormState RememberFormState(
            System.Windows.Forms.Form form, 
            bool ignoreSize)
        {
            return new FormState(form, form.Name, ignoreSize);
        }

        /// <summary>
        /// Remembers the state of the form.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <param name="name">
        /// The name to save the form settings under (defaults to form.Name).
        /// </param>
        /// <returns></returns>
        public static FormState RememberFormState(
            System.Windows.Forms.Form form, 
            string name, 
            bool ignoreSize)
        {
            return new FormState(form, name, ignoreSize);
        }

        private Form m_parent;
        private string m_registryKey;
        private bool m_ignoreSize = false;

        /// <summary>
        /// Initializes an instance of the FormState class.
        /// </summary>
        /// <param name="parent">
        /// The form to store settings for.
        /// </param>
        /// <param name="subkey">
        /// Registry path from HKEY_CURRENT_USER to place for storing settings.
        /// Will create a subkey named "FormState".
        /// </param>
        public FormState(Form parent, string subkey)
        {
            if ((subkey == null) || (subkey.Length == 0))
            {
                throw new ArgumentException(
                    "FormState must be initialized with a proper subkey.  Have you forgotten to call InitializeComponent()?",
                    "subkey");
            }

            this.m_parent = parent;
            this.m_registryKey = subkey + "\\FormState";
            this.m_parent.Load += new EventHandler(On_Load);
            this.m_parent.FormClosed += new FormClosedEventHandler(On_FormClosed);
        }

        public FormState(Form parent, string subkey, bool ignoreSize) : 
            this(parent, subkey)
        {
            m_ignoreSize = ignoreSize;
        }

        public void SetValue(string name, object value)
        {
            this.RegKey.SetValue(name, value);
        }

        public object GetValue(string name, object default_value)
        {
            return this.RegKey.GetValue(name, default_value);
        }

        /// <summary>
        /// If for some reason the value stored in reg cannot be parsed to int 
        /// the default_value is returned.
        /// </summary>
        public int GetIntValue(string name, int default_value)
        {
            int val = default_value;
            if (!int.TryParse(this.RegKey.GetValue(name, default_value).ToString(), out val))
                val = default_value;
            return val;
        }

        /// <summary>
        /// All form's state are stored within one root key under 
        /// HKEY_CURRENT_USER.
        /// </summary>
        private RegistryKey RegKey
        {
            get
            {
                return Registry.CurrentUser.CreateSubKey(
                    "Software\\PYXIS\\FormStates\\" +
                    this.m_registryKey + "\\" + this.m_parent.Name);
            }
        }

        private void On_Load(object sender, EventArgs e)
        {
            int X, Y, width, height, windowState;

            // place to get settings from
            RegistryKey key = this.RegKey;

            if (!int.TryParse(key.GetValue("DesktopBounds.Width",
              this.m_parent.DesktopBounds.Width).ToString(),
              out width))
            {
                width = this.m_parent.DesktopBounds.Width;
            }

            if (!int.TryParse(key.GetValue("DesktopBounds.Height",
              this.m_parent.DesktopBounds.Height).ToString(),
              out height))
            {
                height = this.m_parent.DesktopBounds.Height;
            }

            if (!int.TryParse(key.GetValue("DesktopBounds.X",
              this.m_parent.DesktopBounds.X).ToString(),
              out X))
            {
                X = this.m_parent.DesktopBounds.X;
            }

            if (!int.TryParse(key.GetValue("DesktopBounds.Y",
              this.m_parent.DesktopBounds.Y).ToString(),
              out Y))
            {
                Y = this.m_parent.DesktopBounds.Y;
            }

            // In case of multi screen desktops, check if we got the
            // screen the form was when closed.
            // If not there we put it in upper left corner of nearest 
            // screen.
            // We don't bother checking size (as long as the user sees
            // the form ...).
            Rectangle screenBounds = Screen.GetBounds(new Point(X, Y));
            if ((X > screenBounds.X + screenBounds.Width) || (X < screenBounds.X) ||
                (Y > screenBounds.Y + screenBounds.Height) || (Y < screenBounds.Y))
            {
                X = screenBounds.X;
                Y = screenBounds.Y;
            }

            if (width == 0 || height == 0 || m_ignoreSize)
            {
                this.m_parent.DesktopLocation = new Point(X, Y);
            }
            else
            {
                this.m_parent.DesktopBounds = new Rectangle(X, Y, width, height);
            }

            if (!int.TryParse(key.GetValue("WindowState",
              (int)this.m_parent.WindowState).ToString(),
              out windowState))
            {
                windowState = (int)this.m_parent.WindowState;
            }

            this.m_parent.WindowState = (FormWindowState)windowState;
        }

        private void On_FormClosed(object sender, FormClosedEventArgs e)
        {
            // There may be cases where the event is raised twice.
            // To avoid handling it twice we remove the handler.
            // TODO: find out why it is raised twice ...
            this.m_parent.FormClosed -= new FormClosedEventHandler(On_FormClosed);            

            // place to store settings
            RegistryKey key = this.RegKey;

            if (this.m_parent.WindowState != FormWindowState.Minimized)
            {
                // save window state
                key.SetValue("WindowState", (int)this.m_parent.WindowState);
            }
            else
            {
                // if the window was minimized, remember its state as normal
                key.SetValue("WindowState", (int)FormWindowState.Normal);
            }

            // save pos & size in normal window state
            if (this.m_parent.WindowState != FormWindowState.Normal)
            {
                this.m_parent.WindowState = FormWindowState.Normal;
            }
            key.SetValue("DesktopBounds.Y", this.m_parent.DesktopBounds.Y);
            key.SetValue("DesktopBounds.X", this.m_parent.DesktopBounds.X);

            if (m_ignoreSize)
            {
                key.SetValue("DesktopBounds.Width", 0);
                key.SetValue("DesktopBounds.Height", 0);
            }
            else
            {
                key.SetValue("DesktopBounds.Width", this.m_parent.DesktopBounds.Width);
                key.SetValue("DesktopBounds.Height", this.m_parent.DesktopBounds.Height);
            }
        }
    }
}
