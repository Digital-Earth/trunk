/******************************************************************************
MD5Form.cs

begin      : 08/21/2007 3:00:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace MD5Generator
{
    /// <summary>
    /// This is a small utility program to generate MD5's for a file that
    /// you can select. Don't put any work into this (like bulletproofing etc...)
    /// as it's just for some testing purposes. It doensn't need to be
    /// production quality.
    /// </summary>
    public partial class MD5Form : Form
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public MD5Form()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Handler for when the 'select file...' button is clicked.
        /// Controller method to generate the selected file's MD5
        /// and display it.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ButtonSelectFile_Click(object sender, EventArgs e)
        {
            textBoxSelectedFile.Text = "";
            labelGeneratedMD5.Text =
                GenerateMD5(GetSelectedFile());
        }

        /// <summary>
        /// Gets the full name and path of the file the user wants
        /// to generated an MD5 for, which they selected from the
        /// dialog.
        /// </summary>
        /// <returns>The full name and path of the file.</returns>
        private String GetSelectedFile()
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Title = "Select a file to Generate MD5 for";
            return (dialog.ShowDialog() == DialogResult.OK)
                ? dialog.FileName : null;
        }

        /// <summary>
        /// Calculate the MD5 value for the file the user selected.
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        private String GenerateMD5(String file)
        {
            if (file == null)
            {
                return "";
            }

            textBoxSelectedFile.Text = file;
            byte[] md5Result = null;
            System.Security.Cryptography.MD5CryptoServiceProvider md5 =
                new System.Security.Cryptography.MD5CryptoServiceProvider();
            FileStream fileStream = null;

            try
            {
                fileStream = new FileStream(file, FileMode.Open, FileAccess.Read);
                md5Result = md5.ComputeHash(fileStream);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error creating MD5: " + e.Message);
                // exit app
            }

            // Convert md5 to Hex format
            StringBuilder strBuilder = new StringBuilder();
            for (int index = 0; index < md5Result.Length; index++)
            {
                strBuilder.Append(md5Result[index].ToString("X2"));
            }
            return strBuilder.ToString();
        }       
    }
}