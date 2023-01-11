/******************************************************************************
OracleLoginPage.cs

project    : Oracle Pipeline Wizard

begin      : July 21, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

using Microsoft.Win32;
using System.Text.RegularExpressions;


namespace OraclePipelineWizard
{
    /// <summary>
    /// Class hosts the oracle login page.
    /// </summary>
    public partial class OracleLoginPage : UserControl
    {
        private OracleConnectData m_connectData;
        
        public OracleLoginPage(OracleConnectData connectData)
        {
            Enabled = false;
            m_connectData = connectData;
            InitializeComponent();
        }
  
        public OracleConnectData ConnectData
        {
            get { return m_connectData; }
        }

        public string UserName
        {
            set { userNameTextBox.Text = value; }
            get { return userNameTextBox.Text; }
        }

        public string Password
        {
            set { passwordTextBox.Text = value; }
            get { return passwordTextBox.Text; }
        }

        public string SID
        {
            get { return (string)tnsNamesListBox.SelectedItem; }
        }

        /// <summary>
        /// OnEnable handler get called going and leaving page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnEnable(object sender, EventArgs e)
        {
            if (Enabled)
            {
                UserName = ConnectData.UserName;
                Password = ConnectData.Password;

                TNSNamesReader reader = new TNSNamesReader();

                tnsNamesListBox.Items.Clear();
                List<string> oracleHomeNames = reader.GetOracleHomes();
                foreach (string oracleHome in oracleHomeNames)
                {
                    List<string> names = reader.LoadTNSNames(oracleHome);
                    foreach (string tnsName in names)
                    {
                        int nIdx = tnsNamesListBox.Items.Add(tnsName);
                        if (tnsName == ConnectData.SID)
                        {
                            tnsNamesListBox.SelectedIndex = nIdx;
                        }
                    }

                    if (names.Count == 1)
                    {
                        tnsNamesListBox.SelectedIndex = 0;
                    }

                }
            }
            else
            {
                ConnectData.Password = Password;
                ConnectData.UserName = UserName;
                ConnectData.SID = SID;
            }

        }

        /// <summary>
        /// ValidateChildren is called when tring to "go next" in wizard.
        /// </summary>
        /// <returns>Returns true if all is okay for wizard to proceed.</returns>
        public override bool ValidateChildren()
        {
            if (tnsNamesListBox.SelectedItems.Count == 0)
            {
                MessageBox.Show("Select an Oracle Named Service.", Application.ProductName);
                return false;
            }

            return true;
        }
    }

    //---------------------------------------------------------------------
    //--
    //-- TNSNamesReader
    //---
    //---------------------------------------------------------------------

    /// <summary>
    /// Reads Oracle TNS Names from system.
    /// </summary>
    public class TNSNamesReader
    {
        public List<string> GetOracleHomes()
        {
            List<string> oracleHomes = new List<string>();
            RegistryKey rgkLM = Registry.LocalMachine;
            RegistryKey rgkAllHome = rgkLM.OpenSubKey(@"SOFTWARE\ORACLE");
            if (rgkAllHome != null)
            {
                foreach (string subkey in rgkAllHome.GetSubKeyNames())
                {
                    if (subkey.StartsWith("KEY_"))
                        oracleHomes.Add(subkey);
                }
            }
            return oracleHomes;
        }

        private string GetOracleHomePath(String OracleHomeRegistryKey)
        {
            RegistryKey rgkLM = Registry.LocalMachine;
            RegistryKey rgkOracleHome = rgkLM.OpenSubKey(@"SOFTWARE\ORACLE\" +
                OracleHomeRegistryKey);

            if (!rgkOracleHome.Equals(""))
                return rgkOracleHome.GetValue("ORACLE_HOME").ToString();
            return "";
        }

        private string GetTNSNAMESORAFilePath(String OracleHomeRegistryKey)
        {
            string oracleHomePath = this.GetOracleHomePath(OracleHomeRegistryKey);
            string tnsNamesOraFilePath = "";
            if (!oracleHomePath.Equals(""))
            {
                tnsNamesOraFilePath = oracleHomePath + @"\NETWORK\ADMIN\TNSNAMES.ORA";
                if (!(System.IO.File.Exists(tnsNamesOraFilePath)))
                {
                    tnsNamesOraFilePath = oracleHomePath + @"\NET80\ADMIN\TNSNAMES.ORA";
                }
            }
            return tnsNamesOraFilePath;
        }

        public List<string> LoadTNSNames(string OracleHomeRegistryKey)
        {
            List<string> DBNamesCollection = new List<string>();
            string RegExPattern = @"[\n][\s]*[^\(][a-zA-Z0-9_.]+[\s]*=[\s]*\(";
            string strTNSNAMESORAFilePath = GetTNSNAMESORAFilePath(OracleHomeRegistryKey);

            if (!strTNSNAMESORAFilePath.Equals(""))
            {
                //check out that file does physically exists
                System.IO.FileInfo fiTNS = new System.IO.FileInfo(strTNSNAMESORAFilePath);
                if (fiTNS.Exists)
                {
                    if (fiTNS.Length > 0)
                    {
                        //read tnsnames.ora file
                        int iCount;
                        for (iCount = 0; iCount < Regex.Matches(
                            System.IO.File.ReadAllText(fiTNS.FullName),
                            RegExPattern).Count; iCount++)
                        {
                            DBNamesCollection.Add(Regex.Matches(
                                System.IO.File.ReadAllText(fiTNS.FullName),
                                RegExPattern)[iCount].Value.Trim().Substring(0,
                                Regex.Matches(System.IO.File.ReadAllText(fiTNS.FullName),
                                RegExPattern)[iCount].Value.Trim().IndexOf(" ")));
                        }
                    }
                }
            }
            return DBNamesCollection;
        }
    }

}
