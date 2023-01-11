/******************************************************************************
SpatialTablePage.cs

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
    /// Hosts spatial table pages which displays all available spatial tables in context.
    /// </summary>
    public partial class SpatialTablePage : UserControl
    {
        private OracleConnectData m_connectData;

        
        public SpatialTablePage(OracleConnectData connectData)
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

                listBox1.Items.Clear();
                List<OracleConnectData.DatabaseNodeInfo> tables = ConnectData.FindTables();
                if (tables != null)
                {
                    foreach (OracleConnectData.DatabaseNodeInfo table in tables)
                    {
                        int nIdx = listBox1.Items.Add(table);
                        if (ConnectData.SpatialTable != null && table.Desc == ConnectData.SpatialTable.Desc)
                        {
                            listBox1.SelectedIndex = nIdx;
                        }
                    }

                    if (tables.Count == 1)
                    {
                        listBox1.SelectedIndex = 0;
                    }
                }

                Cursor.Current = Cursors.Default;
            }
            else
            {
                 ConnectData.SpatialTable = (OracleConnectData.DatabaseNodeInfo)listBox1.SelectedItem;
            }
        }

        /// <summary>
        /// ValidateChildren is called when tring to "go next" in wizard.
        /// </summary>
        /// <returns>Returns true if all is okay for wizard to proceed.</returns>
        public override bool ValidateChildren()
        {
            if (listBox1.SelectedItems.Count == 0)
            {
                MessageBox.Show("Make a selection first.", Application.ProductName);
                return false;
            }
            return true;
        }

    }
}
