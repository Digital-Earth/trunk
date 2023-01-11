using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using Pyxis.Contract.Publishing;
using StudioLauncher.Properties;

namespace StudioLauncher
{

    public partial class ControlModeForm : Form
    {
        /// <summary>
        /// The selected version. Set when the user OK's the dialog.
        /// </summary>
        public Product SelectedVersion { get; set; }

        /// <summary>
        /// The startup URL. Set when the user OK's the dialog.
        /// </summary>
        public String StartupURL { get; set; }

        /// <summary>
        /// The geosource test directory. Set when the user OK's the dialog.
        /// </summary>
        public String TestDirectory { get; set; }

        /// <summary>
        /// Set to true to clear the data cache
        /// </summary>
        public bool ClearCache { get; set; }

        public ControlModeForm(List<Product> products, String url)
        {
            SelectedVersion = null;
            StartupURL = null;
            TestDirectory = null;
            ClearCache = false;

            InitializeComponent();

            // do an anonymous LINQ query to create the proper columns for the grid
            var gridDataSource = products.Select(p => new ProductProxy
            {
                Product = p,
                Version = p.ProductVersion,
                Description = p.Metadata.Description,
                SystemTags = string.Join(", ", p.Metadata.SystemTags)
            });

            versionDataGridView.AutoGenerateColumns = false;
            versionDataGridView.DataSource = gridDataSource.ToList();

            DataGridViewColumn col1 = new DataGridViewTextBoxColumn();
            col1.HeaderText = "Version";
            col1.DataPropertyName = "Version";
            versionDataGridView.Columns.Add(col1);

            DataGridViewColumn col2 = new DataGridViewTextBoxColumn();
            col2.HeaderText = "Description";
            col2.DataPropertyName = "Description";
            versionDataGridView.Columns.Add(col2);

            DataGridViewColumn col3 = new DataGridViewTextBoxColumn();
            col3.HeaderText = "System Tags";
            col3.DataPropertyName = "SystemTags";
            versionDataGridView.Columns.Add(col3);

            versionDataGridView.MultiSelect = false;
            versionDataGridView.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            versionDataGridView.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill;
            versionDataGridView.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);

            // set the startup url
            startupURLComboBox.Items.Add(Settings.Default.StagingStartupURL);
            startupURLComboBox.Items.Add(Settings.Default.ProductionStartupURL);
            startupURLComboBox.Items.Add(Settings.Default.TestStartupURL);
            startupURLComboBox.Items.Add(Settings.Default.DevelopmentStartupURL);
            startupURLComboBox.Text = url ?? "";
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            ProductProxy proxy = versionDataGridView.SelectedRows[0].DataBoundItem as ProductProxy;
            SelectedVersion = (proxy != null) ? proxy.Product : new Product();
            StartupURL = startupURLComboBox.Text;
            TestDirectory = testDirectoryTextBox.Text;
            ClearCache = clearCacheCheckBox.Checked;
        }

        private void browseButton_Click(object sender, EventArgs e)
        {
            var browser = new FolderBrowserDialog();
            if (browser.ShowDialog() == DialogResult.OK)
            {
                testDirectoryTextBox.Text = browser.SelectedPath;
            }
        }

        /// <summary>
        /// Converts product fields into those suitable for the ListGridView
        /// </summary>
        internal class ProductProxy
        {
            public Product Product { get; set; }
            public Version Version { get; set; }
            public string Description { get; set; }
            public string SystemTags { get; set; }
        }
    }
}
