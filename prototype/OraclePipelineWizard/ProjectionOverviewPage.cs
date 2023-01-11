/******************************************************************************
ProjectionOverviewPage.cs

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
    /// Hosts projection overview page, displays projection WKT record as extracted by GDAL.
    /// </summary>
    public partial class ProjectionOverviewPage : UserControl
    {
        private OracleConnectData m_connectData;


        public ProjectionOverviewPage(OracleConnectData connectData)
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
            if (Enabled)
            {
                Cursor.Current = Cursors.WaitCursor;

                string projcs = ConnectData.GetProjectionData();

                richTextBox1.Clear();
                if( projcs != null )
                    richTextBox1.AppendText(projcs);
                richTextBox1.ReadOnly = true;

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

    }
}
