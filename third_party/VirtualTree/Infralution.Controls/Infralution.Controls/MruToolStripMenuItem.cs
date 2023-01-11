#region File Header
//
//      FILE:   MruToolStripMenuItem.cs.
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Text;
using System.Windows.Forms;
using System.ComponentModel;
using System.Configuration;
using Infralution.Common;
using System.Runtime.InteropServices;
namespace Infralution.Controls
{
    /// <summary>
    /// Defines a <see cref="ToolStripMenuItem"/> that has a sub menu displaying the
    /// Most Recently Used (MRU) entries (typically file paths)
    /// </summary>
    ///
    [DesignTimeVisible(true)]
    [ToolboxItem(true)]
    [DefaultEvent("MruMenuItemClicked")]
    public class MruToolStripMenuItem : ToolStripMenuItem
    {
        #region Member Variables

        private const int _defaultMaxEntries = 6;
        private const int _defaultMaxDisplayLength = 30;

        private int _maxEntries = _defaultMaxEntries;
        private int _maxDisplayLength = _defaultMaxDisplayLength;
        private string _settingsKey;
        private bool _saveSettings = true;
        private MruSettings _mruSettings;
        private StringCollection _entries = new StringCollection();

        #endregion

        #region Local Types


        #endregion

        #region Public Interface

        /// <summary>
        /// Fired when the user clicks on the an item in the MRU Menu
        /// </summary>
        public event MruMenuItemClickedHandler MruMenuItemClicked;

        /// <summary>
        /// Create a new instance of the class
        /// </summary>
        public MruToolStripMenuItem()
        {
        }

        /// <summary>
        /// Should the menu save and load user settings
        /// </summary>
        [Description("Should the menu save and load user settings")]
        [Category("Behavior")]
        [DefaultValue(true)]
        public bool SaveSettings
        {
            get { return _saveSettings; }
            set { _saveSettings = value; }
        }

        /// <summary>
        /// The name used to identify this mru menu in the user settings
        /// </summary>
        [Description("The name used to identify this mru menu in the user settings")]
        [Category("Behavior")]
        public string SettingsKey
        {
            get
            {
                if (_settingsKey == null)
                {
                    return this.Name;
                }
                return _settingsKey;
            }
            set { _settingsKey = value; }
        }

        /// <summary>
        /// The maximum number of entries the submenu should contain
        /// </summary>
        [Description("The maximum number of entries the submenu should contain")]
        [Category("Behavior")]
        [DefaultValue(_defaultMaxEntries)]
        public int MaxEntries
        {
            get { return _maxEntries; }
            set
            {
                if (value < 1) 
                    throw new ArgumentOutOfRangeException("MaxEntries", "MaxEntries must be one or more");
                if (value != _maxEntries)
                {
                    _maxEntries = value;
                    LoadComponentSettings();
                    RemoveExcessEntries();
                }
            }
        }

        /// <summary>
        /// The maximum number of characters to display for an entry
        /// </summary>
        [Description("The maximum number of characters to display for an entry")]
        [Category("Behavior")]
        [DefaultValue(_defaultMaxDisplayLength)]
        public int MaxDisplayLength
        {
            get { return _maxDisplayLength; }
            set
            {
                if (value < 15)
                    throw new ArgumentOutOfRangeException("MaxDisplayLength", "MaxDisplayLength must be greater than 15");
                if (value != _maxDisplayLength)
                {
                    _maxDisplayLength = value;
                    BuildChildMenuItems();
                }
            }
        }
        
        /// <summary>
        /// Get/Set the entries for the MRU menus
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public string[] Entries
        {
            get 
            {
                string[] result = new string[_entries.Count];
                _entries.CopyTo(result, 0);
                return result;
            }
            set
            {
                LoadComponentSettings();
                _entries.Clear();
                _entries.AddRange(value);
                BuildChildMenuItems();
            }
        }

        /// <summary>
        /// Add an entry to the MRU list
        /// </summary>
        /// <param name="entry">The entry to add</param>
        public void AddEntry(string entry)
        {
            LoadComponentSettings();
            _entries.Remove(entry);
            _entries.Insert(0, entry);
             RemoveExcessEntries();
             BuildChildMenuItems();
         }

        /// <summary>
        /// Remove an entry from the MRU list
        /// </summary>
        /// <param name="entry">The entry to remove</param>
        public void RemoveEntry(string entry)
        {
            LoadComponentSettings();
            _entries.Remove(entry);
            BuildChildMenuItems();
        }

        /// <summary>
        /// Clear all entries from the MRU list
        /// </summary>
        public void ClearEntries()
        {
            LoadComponentSettings();
            _entries.Clear();
            BuildChildMenuItems();
        }

        #endregion

        #region Local Methods

        [DllImport("shlwapi.dll", CharSet = CharSet.Auto)]
        static extern bool PathCompactPathEx([Out] StringBuilder pszOut, string szPath, int cchMax, int dwFlags);


        /// <summary>
        /// Attach to the owner VisibleChanged event to load settings when the owner tool strip
        /// first becomes visible
        /// </summary>
        /// <param name="e"></param>
        protected override void OnOwnerChanged(EventArgs e)
        {
            base.OnOwnerChanged(e);
            if (this.Owner != null)
            {
                this.Owner.VisibleChanged += new EventHandler(OnOwnerVisibleChanged);
            }
        }

        /// <summary>
        /// Load the component settings when the owner tool strip is first displayed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnOwnerVisibleChanged(object sender, EventArgs e)
        {
            if (Owner.Visible && SaveSettings)
            {
                LoadComponentSettings();
            }
        }

        /// <summary>
        /// Handle the user clicking on a given menu item
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDropDownItemClicked(ToolStripItemClickedEventArgs e)
        {
            string entry = e.ClickedItem.Tag as string;

            // add the entry again so it moves to the top of the list
            //
            AddEntry(entry);

            base.OnDropDownItemClicked(e);
            if (this.MruMenuItemClicked != null)
            {
                MruMenuItemClicked(this, new MruMenuItemClickedEventArgs(entry, e.ClickedItem));
            }
        }

        /// <summary>
        /// Return the text to display for the given entry
        /// </summary>
        /// <param name="index">The index of the entry in the menu</param>
        /// <param name="entry">The entry to get the display string for</param>
        /// <returns></returns>
        protected virtual string GetDisplayText(int index, string entry)
        {
            StringBuilder textBuilder = new StringBuilder(MaxDisplayLength);
            PathCompactPathEx(textBuilder, entry, MaxDisplayLength, 0);

            string format;
            if (index < 9)
                format = "&{0} {1}";
            else if (index == 10)
                format = "1&0 {1}";
            else
                format = "{0} {1}";
            return string.Format(format, index, textBuilder.ToString());
        }

        /// <summary>
        /// Build the sub menu based on the mru entry list
        /// </summary>
        private void BuildChildMenuItems()
        {
            DropDownItems.Clear();
            int index = 1;
            foreach (string entry in _entries)
            {
                ToolStripMenuItem item = new ToolStripMenuItem(GetDisplayText(index++, entry));
                item.Tag = entry;
                DropDownItems.Add(item);
            }
            this.Enabled = _entries.Count > 0;
        }

        /// <summary>
        /// Load settings for this menu item
        /// </summary>
        private void LoadComponentSettings()
        {
            // check we haven't already loaded the settings
            //
            if (_mruSettings == null)
            {
                try
                {
                    _mruSettings = new MruSettings();
                    _mruSettings.SettingsKey = SettingsKey;
                    if (_mruSettings.UpgradeSettings)
                    {
                        _mruSettings.Upgrade();
                        _mruSettings.UpgradeSettings = false;
                    }
                    _entries = _mruSettings.Entries;
                    if (_entries == null)
                    {
                        _entries = new StringCollection();
                    }
                    RemoveExcessEntries();
                    BuildChildMenuItems();
                }
                catch (ConfigurationException ex)
                {
                    ExceptionUtilities.HandleConfigurationException(ex);
                }
            }
        }

        /// <summary>
        /// Save settings for this menu item
        /// </summary>
        private void SaveComponentSettings()
        {
            try
            {
                _mruSettings.Entries = _entries;
                _mruSettings.Save();
            }
            catch (ConfigurationException ex)
            {
                ExceptionUtilities.HandleConfigurationException(ex);
            }
       }

        /// <summary>
        /// Trim the number of displayed entries to the MaxEntries value
        /// </summary>
        private void RemoveExcessEntries()
        {
            while (_entries.Count > MaxEntries)
            {
                _entries.RemoveAt(_entries.Count - 1);
            }
        }

        /// <summary>
        /// Save the settings for the component
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (SaveSettings && _mruSettings != null)
            {
                SaveComponentSettings();
            }
        }

        #endregion
    }

}
