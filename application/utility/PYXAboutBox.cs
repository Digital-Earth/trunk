using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ApplicationUtility
{
    /// <summary>
    /// PYXAbout box is a general about dialog box to be used in Pyxis applications.
    /// The about box displays information such as the company name, product, 
    /// version number, build number and copyright information for all 
    /// third party code used within the application. 
    /// </summary>
    public partial class PYXAboutBox : Form
    { 
        /// <summary>
        /// Constructor for the about box.
        /// </summary>
        /// <param name="appName">
        /// The application name. The title bar of the dialog and the 
        /// application name are set to this string. 
        /// </param>
        public PYXAboutBox(string appName)
        {
            InitializeComponent();
            System.Diagnostics.Debug.Assert(appName.Length != 0);
            Text = Text + appName;
            m_appNameText.Text = appName;
        }

        /// <summary>
        /// Sets the label containing the version number to this string.
        /// </summary>
        /// <param name="versionNumber">
        /// The string representation of the version number.
        /// </param>
        public void setVersionNumber(string versionNumber)
        {
            System.Diagnostics.Debug.Assert(versionNumber.Length != 0);
            m_versionText.Text = versionNumber;
        }

        /// <summary>
        /// Sets the copyright notice label displaying that the information
        /// is copyright protected and the year in which it occurs.
        /// </summary>
        /// <param name="copyRightLabel">
        /// The string indicating that the information is copyrighted.
        /// </param>
        public void setCopyrightLabel(string copyRightLabel)
        {
            System.Diagnostics.Debug.Assert(copyRightLabel.Length != 0);
            m_copyrightText.Text = copyRightLabel;
        }

        /// <summary>
        /// Sets the text box to display PYXIS copyright information as well
        /// as copyright information for the third party components used 
        /// within the application this dialog box is for.
        /// </summary>
        /// <param name="copyRightText">
        /// The string which represents PYXIS copyright information as 
        /// well as all third party component copyright information.
        /// </param>
        public void setCopyrightText(string copyRightText)
        {
            System.Diagnostics.Debug.Assert(copyRightText.Length != 0);
            m_copyrightTextBox.MaxLength = copyRightText.Length;
            m_copyrightTextBox.Text = copyRightText;
            m_copyrightTextBox.Select(0, 0);
        }

        /// <summary>
        /// Hides the dialog from view. 
        /// </summary>
        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Open the default browser to the pyxis webpage
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void websiteLink_Clicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start("www.pyxisinnovation.com");
        }

        /// <summary>
        /// Open the default mail program initialized to send an email
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mailto_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            string mailString = string.Format(
                Properties.Resources.MailtoString,
                m_appNameText.Text, 
                m_versionText.Text);
            System.Diagnostics.Process.Start(mailString);
        }
    }
}