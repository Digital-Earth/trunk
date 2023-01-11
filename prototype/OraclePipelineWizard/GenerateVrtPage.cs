/******************************************************************************
GenerateVrtPage.cs

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

namespace OraclePipelineWizard
{
    /// <summary>
    /// Class hosts the "Generate VRT Page".
    /// </summary>
    public partial class GenerateVrtPage : UserControl
    {
        private OracleConnectData m_connectData;


        public GenerateVrtPage(OracleConnectData connectData)
        {
            Enabled = false;
            m_connectData = connectData;
            InitializeComponent();
        }

  
        public OracleConnectData ConnectData
        {
            get { return m_connectData; }
        }

        /// <summary>
        /// OnEnable handler get called going and leaving page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnEnabled(object sender, EventArgs e)
        {
            //--
            //-- does nothing
            //--
            if (Enabled)
            {
                Cursor.Current = Cursors.WaitCursor;
    
                Cursor.Current = Cursors.Default;
            }
            else
            {
            }
        }

        /// <summary>
        /// ValidateChildren is called when tring to "go next" in wizard.
        /// </summary>
        /// <returns>Returns true if all is okay for wizard to proceed.</returns>
        public override bool ValidateChildren()
        {

            return true;
        }

        /// <summary>
        /// Button handler calls file dialog for easy selection of file.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnFileButtonClick(object sender, EventArgs e)
        {
            if (fileName.Text != null && fileName.Text.Length != 0)
            {
                saveFileDialog1.FileName = fileName.Text;
            }

            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                fileName.Text = saveFileDialog1.FileName;
            }
        }

        /// <summary>
        /// Button handler starts the GenerateVRT process.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnGenerate(object sender, EventArgs e)
        {
            if (fileName.Text != null && fileName.Text.Length != 0)
            {
                ConnectData.GenerateVRT(fileName.Text);
            }
            else
            {
                MessageBox.Show("Need an output file name.", Application.ProductName);
            }
        }
    }
}
