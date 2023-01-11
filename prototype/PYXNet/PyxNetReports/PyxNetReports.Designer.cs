namespace PyxNetReportsApplication
{
    partial class PyxNetReports
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PyxNetReports));
            this.label1 = new System.Windows.Forms.Label();
            this.OnlineStatus = new System.Windows.Forms.TextBox();
            this.NodeInfoList = new System.Windows.Forms.DataGridView();
            this.nodeInfoBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Notes = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.DNSResolution = new System.Windows.Forms.TextBox();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.friendlyNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Connected = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.Known = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.Persistant = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.Temporary = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.CanConnect = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.modeDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.isHubDataGridViewCheckBoxColumn = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.isLeafDataGridViewCheckBoxColumn = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.nodeGUIDDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.addressDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.nodeIdDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.NodeInfoList)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nodeInfoBindingSource)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(137, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "This application is currently:";
            // 
            // OnlineStatus
            // 
            this.OnlineStatus.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.OnlineStatus.Enabled = false;
            this.OnlineStatus.Location = new System.Drawing.Point(152, 10);
            this.OnlineStatus.Name = "OnlineStatus";
            this.OnlineStatus.ReadOnly = true;
            this.OnlineStatus.Size = new System.Drawing.Size(797, 20);
            this.OnlineStatus.TabIndex = 1;
            this.OnlineStatus.Text = "Offline";
            // 
            // NodeInfoList
            // 
            this.NodeInfoList.AllowUserToAddRows = false;
            this.NodeInfoList.AllowUserToDeleteRows = false;
            this.NodeInfoList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.NodeInfoList.AutoGenerateColumns = false;
            this.NodeInfoList.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle1.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle1.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle1.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle1.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle1.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.NodeInfoList.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
            this.NodeInfoList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.NodeInfoList.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.friendlyNameDataGridViewTextBoxColumn,
            this.Connected,
            this.Known,
            this.Persistant,
            this.Temporary,
            this.CanConnect,
            this.modeDataGridViewTextBoxColumn,
            this.isHubDataGridViewCheckBoxColumn,
            this.isLeafDataGridViewCheckBoxColumn,
            this.nodeGUIDDataGridViewTextBoxColumn,
            this.addressDataGridViewTextBoxColumn,
            this.nodeIdDataGridViewTextBoxColumn});
            this.NodeInfoList.DataSource = this.nodeInfoBindingSource;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.NodeInfoList.DefaultCellStyle = dataGridViewCellStyle2;
            this.NodeInfoList.Location = new System.Drawing.Point(12, 62);
            this.NodeInfoList.Name = "NodeInfoList";
            this.NodeInfoList.ReadOnly = true;
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.NodeInfoList.RowHeadersDefaultCellStyle = dataGridViewCellStyle3;
            this.NodeInfoList.Size = new System.Drawing.Size(945, 279);
            this.NodeInfoList.TabIndex = 2;
            // 
            // nodeInfoBindingSource
            // 
            this.nodeInfoBindingSource.DataSource = typeof(PyxNetReportsApplication.PyxNetReports.DisplayableNodeInfo);
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.DataPropertyName = "NodeId";
            this.dataGridViewTextBoxColumn1.HeaderText = "NodeId";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            // 
            // dataGridViewTextBoxColumn2
            // 
            this.dataGridViewTextBoxColumn2.DataPropertyName = "Address";
            this.dataGridViewTextBoxColumn2.HeaderText = "Address";
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            this.dataGridViewTextBoxColumn2.Width = 180;
            // 
            // Notes
            // 
            this.Notes.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.Notes.Enabled = false;
            this.Notes.Location = new System.Drawing.Point(12, 347);
            this.Notes.Multiline = true;
            this.Notes.Name = "Notes";
            this.Notes.ReadOnly = true;
            this.Notes.Size = new System.Drawing.Size(945, 112);
            this.Notes.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 36);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(219, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "DNS resolves pyxnet.pyxisinnovation.com to:";
            // 
            // DNSResolution
            // 
            this.DNSResolution.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.DNSResolution.Enabled = false;
            this.DNSResolution.Location = new System.Drawing.Point(237, 36);
            this.DNSResolution.Name = "DNSResolution";
            this.DNSResolution.ReadOnly = true;
            this.DNSResolution.Size = new System.Drawing.Size(712, 20);
            this.DNSResolution.TabIndex = 5;
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.DataPropertyName = "Address";
            this.dataGridViewTextBoxColumn3.HeaderText = "Address";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.ReadOnly = true;
            this.dataGridViewTextBoxColumn3.Width = 115;
            // 
            // dataGridViewTextBoxColumn4
            // 
            this.dataGridViewTextBoxColumn4.DataPropertyName = "NodeId";
            this.dataGridViewTextBoxColumn4.FillWeight = 1F;
            this.dataGridViewTextBoxColumn4.HeaderText = "NodeId";
            this.dataGridViewTextBoxColumn4.MinimumWidth = 100;
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            this.dataGridViewTextBoxColumn4.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn5
            // 
            this.dataGridViewTextBoxColumn5.DataPropertyName = "Address";
            this.dataGridViewTextBoxColumn5.HeaderText = "Address";
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            this.dataGridViewTextBoxColumn5.ReadOnly = true;
            this.dataGridViewTextBoxColumn5.Width = 107;
            // 
            // dataGridViewTextBoxColumn6
            // 
            this.dataGridViewTextBoxColumn6.DataPropertyName = "NodeId";
            this.dataGridViewTextBoxColumn6.FillWeight = 1F;
            this.dataGridViewTextBoxColumn6.HeaderText = "NodeId";
            this.dataGridViewTextBoxColumn6.MinimumWidth = 100;
            this.dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
            this.dataGridViewTextBoxColumn6.ReadOnly = true;
            // 
            // friendlyNameDataGridViewTextBoxColumn
            // 
            this.friendlyNameDataGridViewTextBoxColumn.DataPropertyName = "FriendlyName";
            this.friendlyNameDataGridViewTextBoxColumn.FillWeight = 1F;
            this.friendlyNameDataGridViewTextBoxColumn.HeaderText = "FriendlyName";
            this.friendlyNameDataGridViewTextBoxColumn.MinimumWidth = 100;
            this.friendlyNameDataGridViewTextBoxColumn.Name = "friendlyNameDataGridViewTextBoxColumn";
            this.friendlyNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // Connected
            // 
            this.Connected.DataPropertyName = "Connected";
            this.Connected.FillWeight = 1F;
            this.Connected.HeaderText = "Connected";
            this.Connected.MinimumWidth = 30;
            this.Connected.Name = "Connected";
            this.Connected.ReadOnly = true;
            // 
            // Known
            // 
            this.Known.DataPropertyName = "Known";
            this.Known.FillWeight = 1F;
            this.Known.HeaderText = "Known";
            this.Known.MinimumWidth = 30;
            this.Known.Name = "Known";
            this.Known.ReadOnly = true;
            // 
            // Persistant
            // 
            this.Persistant.DataPropertyName = "Persistant";
            this.Persistant.FillWeight = 1F;
            this.Persistant.HeaderText = "Persistant";
            this.Persistant.MinimumWidth = 30;
            this.Persistant.Name = "Persistant";
            this.Persistant.ReadOnly = true;
            // 
            // Temporary
            // 
            this.Temporary.DataPropertyName = "Temporary";
            this.Temporary.FillWeight = 1F;
            this.Temporary.HeaderText = "Temporary";
            this.Temporary.MinimumWidth = 30;
            this.Temporary.Name = "Temporary";
            this.Temporary.ReadOnly = true;
            // 
            // CanConnect
            // 
            this.CanConnect.DataPropertyName = "CanConnect";
            this.CanConnect.FillWeight = 1F;
            this.CanConnect.HeaderText = "CanConnect";
            this.CanConnect.MinimumWidth = 30;
            this.CanConnect.Name = "CanConnect";
            this.CanConnect.ReadOnly = true;
            // 
            // modeDataGridViewTextBoxColumn
            // 
            this.modeDataGridViewTextBoxColumn.DataPropertyName = "Mode";
            this.modeDataGridViewTextBoxColumn.FillWeight = 1F;
            this.modeDataGridViewTextBoxColumn.HeaderText = "Mode";
            this.modeDataGridViewTextBoxColumn.MinimumWidth = 50;
            this.modeDataGridViewTextBoxColumn.Name = "modeDataGridViewTextBoxColumn";
            this.modeDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // isHubDataGridViewCheckBoxColumn
            // 
            this.isHubDataGridViewCheckBoxColumn.DataPropertyName = "IsHub";
            this.isHubDataGridViewCheckBoxColumn.FillWeight = 1F;
            this.isHubDataGridViewCheckBoxColumn.HeaderText = "IsHub";
            this.isHubDataGridViewCheckBoxColumn.MinimumWidth = 40;
            this.isHubDataGridViewCheckBoxColumn.Name = "isHubDataGridViewCheckBoxColumn";
            this.isHubDataGridViewCheckBoxColumn.ReadOnly = true;
            // 
            // isLeafDataGridViewCheckBoxColumn
            // 
            this.isLeafDataGridViewCheckBoxColumn.DataPropertyName = "IsLeaf";
            this.isLeafDataGridViewCheckBoxColumn.FillWeight = 1F;
            this.isLeafDataGridViewCheckBoxColumn.HeaderText = "IsLeaf";
            this.isLeafDataGridViewCheckBoxColumn.MinimumWidth = 40;
            this.isLeafDataGridViewCheckBoxColumn.Name = "isLeafDataGridViewCheckBoxColumn";
            this.isLeafDataGridViewCheckBoxColumn.ReadOnly = true;
            // 
            // nodeGUIDDataGridViewTextBoxColumn
            // 
            this.nodeGUIDDataGridViewTextBoxColumn.DataPropertyName = "NodeGUID";
            this.nodeGUIDDataGridViewTextBoxColumn.FillWeight = 1F;
            this.nodeGUIDDataGridViewTextBoxColumn.HeaderText = "NodeGUID";
            this.nodeGUIDDataGridViewTextBoxColumn.MinimumWidth = 100;
            this.nodeGUIDDataGridViewTextBoxColumn.Name = "nodeGUIDDataGridViewTextBoxColumn";
            this.nodeGUIDDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // addressDataGridViewTextBoxColumn
            // 
            this.addressDataGridViewTextBoxColumn.DataPropertyName = "Address";
            this.addressDataGridViewTextBoxColumn.HeaderText = "Address";
            this.addressDataGridViewTextBoxColumn.Name = "addressDataGridViewTextBoxColumn";
            this.addressDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // nodeIdDataGridViewTextBoxColumn
            // 
            this.nodeIdDataGridViewTextBoxColumn.DataPropertyName = "NodeId";
            this.nodeIdDataGridViewTextBoxColumn.FillWeight = 1F;
            this.nodeIdDataGridViewTextBoxColumn.HeaderText = "NodeId";
            this.nodeIdDataGridViewTextBoxColumn.MinimumWidth = 100;
            this.nodeIdDataGridViewTextBoxColumn.Name = "nodeIdDataGridViewTextBoxColumn";
            this.nodeIdDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // PyxNetReports
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(969, 471);
            this.Controls.Add(this.DNSResolution);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.Notes);
            this.Controls.Add(this.NodeInfoList);
            this.Controls.Add(this.OnlineStatus);
            this.Controls.Add(this.label1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "PyxNetReports";
            this.Text = "PyxNet Status/Reports";
            this.Load += new System.EventHandler(this.PyxNetReports_Load);
            this.Shown += new System.EventHandler(this.PyxNetReports_Shown);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PyxNetReports_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.NodeInfoList)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nodeInfoBindingSource)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox OnlineStatus;
        private System.Windows.Forms.DataGridView NodeInfoList;
        private System.Windows.Forms.BindingSource nodeInfoBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private System.Windows.Forms.TextBox Notes;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox DNSResolution;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
        private System.Windows.Forms.DataGridViewTextBoxColumn friendlyNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewCheckBoxColumn Connected;
        private System.Windows.Forms.DataGridViewCheckBoxColumn Known;
        private System.Windows.Forms.DataGridViewCheckBoxColumn Persistant;
        private System.Windows.Forms.DataGridViewCheckBoxColumn Temporary;
        private System.Windows.Forms.DataGridViewCheckBoxColumn CanConnect;
        private System.Windows.Forms.DataGridViewTextBoxColumn modeDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewCheckBoxColumn isHubDataGridViewCheckBoxColumn;
        private System.Windows.Forms.DataGridViewCheckBoxColumn isLeafDataGridViewCheckBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn nodeGUIDDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn addressDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn nodeIdDataGridViewTextBoxColumn;
    }
}

