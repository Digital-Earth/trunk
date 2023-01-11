#region File Header
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Drawing;
using System.Windows.Forms;
using System.Configuration;
using System.Collections.Generic;
using Infralution.Common;
namespace Infralution.Controls
{

    /// <summary>
    /// Defines a class for managing the settings for a form to user settings
    /// </summary>
    /// <remarks>
    /// This class can be used manage settings for existing form classes (which
    /// can't be changed to derived from <see cref="BaseForm"/>)
    /// </remarks>
    public class FormSettingsManager
    {

        /// <summary>
        /// Settings for the form
        /// </summary>
        private FormSettings _formSettings = new FormSettings();
        private bool _saveSettings = true;
        private string _settingsKey;
        private Form _form;

        /// <summary>
        /// Create an instance to manage the given form
        /// </summary>
        /// <param name="form">The form to manage</param>
        public FormSettingsManager(Form form)
        {
            if (form == null) throw new ArgumentNullException("form");
            _form = form;
            _form.FormClosed += new FormClosedEventHandler(OnFormClosed);
            _form.Load += new EventHandler(OnFormLoad);
        }

        /// <summary>
        /// Create an instance to manage the given form
        /// </summary>
        /// <param name="form">The form to manage</param>
        /// <param name="settingsKey">The name of the user settings key</param>
        public FormSettingsManager(Form form, string settingsKey)
            : this(form)
        {
            _settingsKey = settingsKey;
        }

        /// <summary>
        /// Should the form manager save and load user settings
        /// </summary>
        public bool SaveSettings
        {
            get { return _saveSettings; }
            set { _saveSettings = value; }
        }

        /// <summary>
        /// The name used to identify the form in the user settings
        /// </summary>
        public string SettingsKey
        {
            get
            {
                string result = _settingsKey;
                if (string.IsNullOrEmpty(result))
                {
                    result = Form.Name;
                    if (string.IsNullOrEmpty(result))
                    {
                        result = Form.GetType().Name;
                    }
                }
                return result;
            }
            set { _settingsKey = value; }
        }

        /// <summary>
        /// The form being managed
        /// </summary>
        public Form Form
        {
            get { return _form; }
        }

        /// <summary>
        /// Save the form position etc to settings
        /// </summary>
        protected virtual void SaveFormSettings()
        {
            Point location;
            Size size;
            if (Form.WindowState == FormWindowState.Normal)
            {
                size = Form.Size;
                location = Form.Location;
            }
            else
            {
                size = Form.RestoreBounds.Size;
                location = Form.RestoreBounds.Location;
            }

            // if the form is owned then save the position relative to the owner
            //
            if (Form.Owner != null)
            {
                location.X -= Form.Owner.Location.X;
                location.Y -= Form.Owner.Location.Y;
            }

            _formSettings.WindowState = Form.WindowState;
            _formSettings.Size = size;
            _formSettings.Location = location;
        }

        /// <summary>
        /// Load the form position etc from settings
        /// </summary>
        protected virtual void LoadFormSettings()
        {
            Point location = _formSettings.Location;
            Size size = _formSettings.Size;

            // if the form is owned then adjust the position relative to the owner
            //
            if (Form.Owner != null)
            {
                location.X += Form.Owner.Location.X;
                location.Y += Form.Owner.Location.Y;
            }

            if (Form.FormBorderStyle == FormBorderStyle.Sizable ||
                Form.FormBorderStyle == FormBorderStyle.SizableToolWindow)
            {
                size = _formSettings.Size;
            }
            else
            {
                size = Form.Size;
            }

            // find the screen containing the centre of the form
            //
            Point centre = location;
            centre.X += size.Width / 2;
            centre.Y += size.Height / 2;

            Screen screen = Screen.FromPoint(centre);
            Rectangle bounds = screen.WorkingArea;

            // limit the size of the window to the working area of the screen
            //
            size.Width = Math.Min(size.Width, bounds.Width);
            size.Height = Math.Min(size.Height, bounds.Height);
            if (size.Height > bounds.Height) size.Height = bounds.Height;

            // adjust the location so that the form fits fully in the screen bounds
            // this prevents forms disappearing when the screen dimensions change
            //
            if (location.X < bounds.X)
            {
                location.X = bounds.X;
            }
            else if (location.X + size.Width > bounds.Right)
            {
                location.X = bounds.Right - size.Width;
            }
            if (location.Y < bounds.Y)
            {
                location.Y = bounds.Y;
            }
            else if (location.Y + size.Height > bounds.Bottom)
            {
                location.Y = bounds.Bottom - size.Height;
            }

            Form.SetBounds(location.X, location.Y, size.Width, size.Height);
            if (_formSettings.WindowState == FormWindowState.Maximized)
            {
                Form.WindowState = FormWindowState.Maximized;
            }
        }

        /// <summary>
        /// Handle the form load event to configure the form size and position
        /// based on the saved settings
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnFormLoad(object sender, EventArgs e)
        {
            if (SaveSettings)
            {
                try
                {
                    _formSettings.SettingsKey = SettingsKey;
                    if (_formSettings.UpgradeSettings)
                    {
                        _formSettings.Upgrade();
                        _formSettings.UpgradeSettings = false;
                    }
                    if (_formSettings.Saved)
                    {
                        LoadFormSettings();
                    }
                }
                catch (ConfigurationException ex)
                {
                    ExceptionUtilities.HandleConfigurationException(ex);
                }
            }
        }

        /// <summary>
        /// Handle the form closed event to save the form size and position
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnFormClosed(object sender, FormClosedEventArgs e)
        {
            if (SaveSettings)
            {
                try
                {
                    SaveFormSettings();
                    _formSettings.Saved = true;
                    _formSettings.Save();

                    // detach the event handlers
                    //
                    _form.FormClosed -= new FormClosedEventHandler(OnFormClosed);
                    _form.Load -= new EventHandler(OnFormLoad);
                }
                catch (ConfigurationException ex)
                {
                    ExceptionUtilities.HandleConfigurationException(ex);
                }
            }
        }

    }
}
