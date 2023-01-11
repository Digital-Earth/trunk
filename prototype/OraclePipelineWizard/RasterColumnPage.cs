/******************************************************************************
RasterColumnPage.cs

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
    /// Hosts Raster Column Page, displays all raster columns in context of previously 
    /// selected spatial table.
    /// </summary>
    public partial class RasterColumnPage : UserControl
    {
        private OracleConnectData m_connectData;

        
        public RasterColumnPage(OracleConnectData connectData)
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

                List<OracleConnectData.DatabaseNodeInfo> columns = ConnectData.FindColumns();
                if (columns != null)
                {
                    foreach (OracleConnectData.DatabaseNodeInfo column in columns)
                    {
                        int nIdx = listBox1.Items.Add(column);
                        if (ConnectData.RasterColumnName != null && column.Desc == ConnectData.RasterColumnName.Desc)
                        {
                            listBox1.SelectedIndex = nIdx;
                        }
                    }

                    if (columns.Count == 1)
                    {
                        listBox1.SelectedIndex = 0;
                    }
                }

                Cursor.Current = Cursors.Default;
            }
            else
            {
                ConnectData.RasterColumnName = (OracleConnectData.DatabaseNodeInfo)listBox1.SelectedItem;
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
