/******************************************************************************
MessageBoxEx.cs

begin      : Dec 18, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PyxNet.Utilities
{
    public partial class MessageBoxEx : Form
    {
        private MessageBoxEx()
        {
            InitializeComponent();
        }

        public enum ShowFlags{
            AskUser,
            DontAskNo,
            DontAskYes
        }

        /// <summary>
        /// Shows the specified message, unless the flag is set.
        /// </summary>
        /// <param name="message">The message.</param>
        /// <param name="caption">The caption.</param>
        /// <param name="showFlags">The show flags.</param>
        /// <returns></returns>
        public static bool Show(string message, string caption, ref ShowFlags showFlags)
        {
            switch (showFlags)
            {
                default:
                case ShowFlags.AskUser:
                    {
                        MessageBoxEx messageBox = new MessageBoxEx();
                        messageBox.Text = caption;
                        messageBox.MessageText.Text = message;
                        messageBox.ShowDialog();
                        if (messageBox.DontAsk.Checked)
                        {
                            switch (messageBox.DialogResult)
                            {
                                case DialogResult.OK:
                                    showFlags = ShowFlags.DontAskYes;
                                    break;
                                case DialogResult.Cancel:
                                    showFlags = ShowFlags.DontAskNo;
                                    break;
                            }
                        }
                        return messageBox.DialogResult == DialogResult.OK;
                    }
                case ShowFlags.DontAskNo:
                    return false;
                case ShowFlags.DontAskYes:
                    return true;
            }
        }

        private void Yes_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }
    }
}