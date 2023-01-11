#region File Header
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion

using System.ComponentModel;
using System.Windows.Forms;

namespace Infralution.Controls
{
    /// <summary>
    /// Defines a base form class that supports saving/loading its position and size to
    /// the application settings user.config file using the <see cref="FormSettingsManager"/> class.
    /// </summary>
    public class BaseForm : Form
    {
        /// <summary>
        /// Settings for the form
        /// </summary>
        private FormSettingsManager _formSettingsManager;

        /// <summary>
        /// Create a new instance of the form
        /// </summary>
        public BaseForm()
        {
            _formSettingsManager = CreateFormSettingsManager();
        }

        /// <summary>
        /// Should the form save and load user settings
        /// </summary>
        [Description("Should the menu save and load user settings")]
        [Category("Behavior")]
        [DefaultValue(true)]
        public bool SaveSettings
        {
            get { return _formSettingsManager.SaveSettings; }
            set { _formSettingsManager.SaveSettings = value; }
        }

        /// <summary>
        /// The name used to identify this form in the user settings
        /// </summary>
        [Description("The name used to identify this mru menu in the user settings")]
        [Category("Behavior")]
        public string SettingsKey
        {
            get { return _formSettingsManager.SettingsKey; }
            set { _formSettingsManager.SettingsKey = value; }
        }

        /// <summary>
        /// Creates the <see cref="FormSettingsManager"/> object used to manage 
        /// the settings for this form
        /// </summary>
        /// <returns>A new FormSettingsManager object</returns>
        /// <remarks>
        /// Can be overridden by derived classes to return a subclass of FormSettingsManager
        /// </remarks>
        protected virtual FormSettingsManager CreateFormSettingsManager()
        {
            return new FormSettingsManager(this);
        }
    }
}