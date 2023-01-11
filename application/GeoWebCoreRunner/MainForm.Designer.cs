namespace GeoWebCoreRunner
{
    partial class MainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.splitContainerMain = new System.Windows.Forms.SplitContainer();
            this.listViewInstances = new System.Windows.Forms.ListView();
            this.columnHeaderInstance = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderMemory = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderCPU = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderDetails = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.panelTools = new System.Windows.Forms.Panel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.buttonNewInstance = new System.Windows.Forms.Button();
            this.buttonKillInstance = new System.Windows.Forms.Button();
            this.buttonImport = new System.Windows.Forms.Button();
            this.buttonValidateChecksum = new System.Windows.Forms.Button();
            this.buttonSettings = new System.Windows.Forms.Button();
            this.labelStatus = new System.Windows.Forms.Label();
            this.tabControlInstance = new System.Windows.Forms.TabControl();
            this.tabPageConsole = new System.Windows.Forms.TabPage();
            this.richTextBoxConsole = new System.Windows.Forms.RichTextBox();
            this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
            this.button4 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.timerRefresh = new System.Windows.Forms.Timer(this.components);
            this.columnHeaderTags = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerMain)).BeginInit();
            this.splitContainerMain.Panel1.SuspendLayout();
            this.splitContainerMain.Panel2.SuspendLayout();
            this.splitContainerMain.SuspendLayout();
            this.panelTools.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            this.tabControlInstance.SuspendLayout();
            this.tabPageConsole.SuspendLayout();
            this.flowLayoutPanel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainerMain
            // 
            this.splitContainerMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerMain.Location = new System.Drawing.Point(0, 0);
            this.splitContainerMain.Name = "splitContainerMain";
            // 
            // splitContainerMain.Panel1
            // 
            this.splitContainerMain.Panel1.Controls.Add(this.listViewInstances);
            this.splitContainerMain.Panel1.Controls.Add(this.panelTools);
            this.splitContainerMain.Panel1.Controls.Add(this.labelStatus);
            // 
            // splitContainerMain.Panel2
            // 
            this.splitContainerMain.Panel2.Controls.Add(this.tabControlInstance);
            this.splitContainerMain.Size = new System.Drawing.Size(1252, 598);
            this.splitContainerMain.SplitterDistance = 504;
            this.splitContainerMain.TabIndex = 0;
            // 
            // listViewInstances
            // 
            this.listViewInstances.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderInstance,
            this.columnHeaderState,
            this.columnHeaderMemory,
            this.columnHeaderCPU,
            this.columnHeaderTags,
            this.columnHeaderDetails});
            this.listViewInstances.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listViewInstances.FullRowSelect = true;
            this.listViewInstances.HideSelection = false;
            this.listViewInstances.Location = new System.Drawing.Point(0, 35);
            this.listViewInstances.MultiSelect = false;
            this.listViewInstances.Name = "listViewInstances";
            this.listViewInstances.Size = new System.Drawing.Size(504, 540);
            this.listViewInstances.TabIndex = 1;
            this.listViewInstances.UseCompatibleStateImageBehavior = false;
            this.listViewInstances.View = System.Windows.Forms.View.Details;
            this.listViewInstances.SelectedIndexChanged += new System.EventHandler(this.listViewInstances_SelectedIndexChanged);
            // 
            // columnHeaderInstance
            // 
            this.columnHeaderInstance.Text = "Instance";
            // 
            // columnHeaderState
            // 
            this.columnHeaderState.Text = "State";
            // 
            // columnHeaderMemory
            // 
            this.columnHeaderMemory.Text = "Memory";
            // 
            // columnHeaderCPU
            // 
            this.columnHeaderCPU.Text = "CPU";
            // 
            // columnHeaderDetails
            // 
            this.columnHeaderDetails.Text = "Details";
            this.columnHeaderDetails.Width = 180;
            // 
            // panelTools
            // 
            this.panelTools.Controls.Add(this.flowLayoutPanel1);
            this.panelTools.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelTools.Location = new System.Drawing.Point(0, 0);
            this.panelTools.Name = "panelTools";
            this.panelTools.Size = new System.Drawing.Size(504, 35);
            this.panelTools.TabIndex = 0;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.Controls.Add(this.buttonNewInstance);
            this.flowLayoutPanel1.Controls.Add(this.buttonKillInstance);
            this.flowLayoutPanel1.Controls.Add(this.buttonImport);
            this.flowLayoutPanel1.Controls.Add(this.buttonValidateChecksum);
            this.flowLayoutPanel1.Controls.Add(this.buttonSettings);
            this.flowLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Padding = new System.Windows.Forms.Padding(3);
            this.flowLayoutPanel1.Size = new System.Drawing.Size(504, 35);
            this.flowLayoutPanel1.TabIndex = 0;
            // 
            // buttonNewInstance
            // 
            this.buttonNewInstance.AutoSize = true;
            this.buttonNewInstance.Location = new System.Drawing.Point(6, 6);
            this.buttonNewInstance.Name = "buttonNewInstance";
            this.buttonNewInstance.Size = new System.Drawing.Size(40, 23);
            this.buttonNewInstance.TabIndex = 0;
            this.buttonNewInstance.Text = "New";
            this.buttonNewInstance.UseVisualStyleBackColor = true;
            this.buttonNewInstance.Click += new System.EventHandler(this.buttonNewInstance_Click);
            // 
            // buttonKillInstance
            // 
            this.buttonKillInstance.AutoSize = true;
            this.buttonKillInstance.Location = new System.Drawing.Point(52, 6);
            this.buttonKillInstance.Name = "buttonKillInstance";
            this.buttonKillInstance.Size = new System.Drawing.Size(40, 23);
            this.buttonKillInstance.TabIndex = 1;
            this.buttonKillInstance.Text = "Kill";
            this.buttonKillInstance.UseVisualStyleBackColor = true;
            this.buttonKillInstance.Click += new System.EventHandler(this.buttonKillInstance_Click);
            // 
            // buttonImport
            // 
            this.buttonImport.AutoSize = true;
            this.buttonImport.Location = new System.Drawing.Point(98, 6);
            this.buttonImport.Name = "buttonImport";
            this.buttonImport.Size = new System.Drawing.Size(55, 23);
            this.buttonImport.TabIndex = 3;
            this.buttonImport.Text = "Import...";
            this.buttonImport.UseVisualStyleBackColor = true;
            this.buttonImport.Click += new System.EventHandler(this.buttonImport_Click);
            // 
            // buttonValidateChecksum
            // 
            this.buttonValidateChecksum.AutoSize = true;
            this.buttonValidateChecksum.Location = new System.Drawing.Point(159, 6);
            this.buttonValidateChecksum.Name = "buttonValidateChecksum";
            this.buttonValidateChecksum.Size = new System.Drawing.Size(67, 23);
            this.buttonValidateChecksum.TabIndex = 4;
            this.buttonValidateChecksum.Text = "Checksum";
            this.buttonValidateChecksum.UseVisualStyleBackColor = true;
            this.buttonValidateChecksum.Click += new System.EventHandler(this.buttonValidateChecksum_Click);
            // 
            // buttonSettings
            // 
            this.buttonSettings.AutoSize = true;
            this.buttonSettings.Location = new System.Drawing.Point(232, 6);
            this.buttonSettings.Name = "buttonSettings";
            this.buttonSettings.Size = new System.Drawing.Size(55, 23);
            this.buttonSettings.TabIndex = 5;
            this.buttonSettings.Text = "Settings";
            this.buttonSettings.UseVisualStyleBackColor = true;
            this.buttonSettings.Click += new System.EventHandler(this.buttonSettings_Click);
            // 
            // labelStatus
            // 
            this.labelStatus.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.labelStatus.Location = new System.Drawing.Point(0, 575);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(504, 23);
            this.labelStatus.TabIndex = 6;
            this.labelStatus.Text = "Status";
            // 
            // tabControlInstance
            // 
            this.tabControlInstance.Controls.Add(this.tabPageConsole);
            this.tabControlInstance.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControlInstance.Location = new System.Drawing.Point(0, 0);
            this.tabControlInstance.Name = "tabControlInstance";
            this.tabControlInstance.SelectedIndex = 0;
            this.tabControlInstance.Size = new System.Drawing.Size(744, 598);
            this.tabControlInstance.TabIndex = 0;
            // 
            // tabPageConsole
            // 
            this.tabPageConsole.Controls.Add(this.richTextBoxConsole);
            this.tabPageConsole.Controls.Add(this.flowLayoutPanel2);
            this.tabPageConsole.Location = new System.Drawing.Point(4, 22);
            this.tabPageConsole.Name = "tabPageConsole";
            this.tabPageConsole.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageConsole.Size = new System.Drawing.Size(736, 572);
            this.tabPageConsole.TabIndex = 0;
            this.tabPageConsole.Text = "Console";
            this.tabPageConsole.UseVisualStyleBackColor = true;
            // 
            // richTextBoxConsole
            // 
            this.richTextBoxConsole.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.richTextBoxConsole.Dock = System.Windows.Forms.DockStyle.Fill;
            this.richTextBoxConsole.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.richTextBoxConsole.ForeColor = System.Drawing.SystemColors.Window;
            this.richTextBoxConsole.Location = new System.Drawing.Point(3, 38);
            this.richTextBoxConsole.Name = "richTextBoxConsole";
            this.richTextBoxConsole.ReadOnly = true;
            this.richTextBoxConsole.Size = new System.Drawing.Size(730, 531);
            this.richTextBoxConsole.TabIndex = 2;
            this.richTextBoxConsole.Text = resources.GetString("richTextBoxConsole.Text");
            // 
            // flowLayoutPanel2
            // 
            this.flowLayoutPanel2.AutoSize = true;
            this.flowLayoutPanel2.Controls.Add(this.button4);
            this.flowLayoutPanel2.Controls.Add(this.button5);
            this.flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Top;
            this.flowLayoutPanel2.Location = new System.Drawing.Point(3, 3);
            this.flowLayoutPanel2.Name = "flowLayoutPanel2";
            this.flowLayoutPanel2.Padding = new System.Windows.Forms.Padding(3);
            this.flowLayoutPanel2.Size = new System.Drawing.Size(730, 35);
            this.flowLayoutPanel2.TabIndex = 1;
            // 
            // button4
            // 
            this.button4.AutoSize = true;
            this.button4.Location = new System.Drawing.Point(6, 6);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(41, 23);
            this.button4.TabIndex = 0;
            this.button4.Text = "Clear";
            this.button4.UseVisualStyleBackColor = true;
            // 
            // button5
            // 
            this.button5.AutoSize = true;
            this.button5.Location = new System.Drawing.Point(53, 6);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(41, 23);
            this.button5.TabIndex = 1;
            this.button5.Text = "Copy";
            this.button5.UseVisualStyleBackColor = true;
            // 
            // timerRefresh
            // 
            this.timerRefresh.Enabled = true;
            this.timerRefresh.Interval = 1000;
            this.timerRefresh.Tick += new System.EventHandler(this.timerRefresh_Tick);
            // 
            // columnHeaderTags
            // 
            this.columnHeaderTags.Text = "Tags";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1252, 598);
            this.Controls.Add(this.splitContainerMain);
            this.Name = "MainForm";
            this.Text = "GeoWebCore Runner";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.splitContainerMain.Panel1.ResumeLayout(false);
            this.splitContainerMain.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerMain)).EndInit();
            this.splitContainerMain.ResumeLayout(false);
            this.panelTools.ResumeLayout(false);
            this.panelTools.PerformLayout();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.tabControlInstance.ResumeLayout(false);
            this.tabPageConsole.ResumeLayout(false);
            this.tabPageConsole.PerformLayout();
            this.flowLayoutPanel2.ResumeLayout(false);
            this.flowLayoutPanel2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainerMain;
        private System.Windows.Forms.ListView listViewInstances;
        private System.Windows.Forms.ColumnHeader columnHeaderInstance;
        private System.Windows.Forms.ColumnHeader columnHeaderState;
        private System.Windows.Forms.ColumnHeader columnHeaderMemory;
        private System.Windows.Forms.ColumnHeader columnHeaderCPU;
        private System.Windows.Forms.Panel panelTools;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Button buttonNewInstance;
        private System.Windows.Forms.Button buttonKillInstance;
        private System.Windows.Forms.TabControl tabControlInstance;
        private System.Windows.Forms.TabPage tabPageConsole;
        private System.Windows.Forms.RichTextBox richTextBoxConsole;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Timer timerRefresh;
        private System.Windows.Forms.Button buttonImport;
        private System.Windows.Forms.Button buttonValidateChecksum;
        private System.Windows.Forms.Button buttonSettings;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.ColumnHeader columnHeaderDetails;
        private System.Windows.Forms.ColumnHeader columnHeaderTags;
    }
}

