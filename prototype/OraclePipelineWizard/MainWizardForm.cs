/******************************************************************************
MainWizardForm.cs

project    : Oracle Pipeline Wizard

begin      : July 21, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace OraclePipelineWizard
{

    /// <summary>
    /// Main encompasing form for wizard like pages.
    /// </summary>
    public partial class WizardForm : Form
    {
        private List<UserControl> page;
        private int currentPageNo;

        private OracleConnectData connectionData;

        /// <summary>
        /// Wizard form constructor.
        /// </summary>
        public WizardForm()
        {
            InitializeComponent();

            connectionData = new OracleConnectData();
            connectionData.UserName = "system";
            connectionData.Password = "password";

            page = new List<UserControl>();
            page.Add( new OracleLoginPage(connectionData) );
            page.Add( new SpatialTablePage(connectionData) );
            page.Add( new RasterColumnPage(connectionData) );
            page.Add( new RasterDataTablesPage(connectionData) );
            page.Add( new ProjectionOverviewPage(connectionData) );
            page.Add( new GenerateVrtPage(connectionData) );

            currentPageNo = -1;
            SetPage(0);
        }

        /// <summary>
        /// Set active page.
        /// </summary>
        /// <param name="pageNo">Page number for new active page.</param>
        private void SetPage(int pageNo)
        {
            if (pageNo >= 0 && pageNo < page.Count )
            {
                if (currentPageNo >= 0 && currentPageNo < page.Count)
                {
                    pageHostPanel.Controls.Remove(page[currentPageNo]);
                    page[currentPageNo].Enabled = false;
                }

                page[pageNo].Enabled = true;
                pageHostPanel.Controls.Add(page[pageNo]);
                currentPageNo = pageNo;

                page[pageNo].Dock = DockStyle.Fill;

                prevButton.Enabled = currentPageNo > 0;
                nextButton.Enabled = currentPageNo < page.Count - 1;
            }
        }

        /// <summary>
        /// Next button handler, advances wizard to next page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnNext(object sender, EventArgs e)
        {
            if( page[currentPageNo].ValidateChildren() )
                SetPage(currentPageNo + 1);
        }

        /// <summary>
        /// Previous button handler, advances wizard to next page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnPrev(object sender, EventArgs e)
        {
            SetPage(currentPageNo - 1);
        }
    }
}
