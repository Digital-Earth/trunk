namespace GeoWebCoreRunner
{
    partial class ImportForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.numericUpDownInstanceCount = new System.Windows.Forms.NumericUpDown();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.textBoxGeoSourceId = new System.Windows.Forms.TextBox();
            this.listBoxGeoSources = new System.Windows.Forms.ListBox();
            this.label3 = new System.Windows.Forms.Label();
            this.buttonVectors = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.buttonCoverages = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.checkBoxDownload = new System.Windows.Forms.CheckBox();
            this.buttonLastWeek = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownInstanceCount)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(82, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Instance Count:";
            // 
            // numericUpDownInstanceCount
            // 
            this.numericUpDownInstanceCount.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDownInstanceCount.Location = new System.Drawing.Point(100, 7);
            this.numericUpDownInstanceCount.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.numericUpDownInstanceCount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownInstanceCount.Name = "numericUpDownInstanceCount";
            this.numericUpDownInstanceCount.Size = new System.Drawing.Size(276, 20);
            this.numericUpDownInstanceCount.TabIndex = 1;
            this.numericUpDownInstanceCount.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // buttonStart
            // 
            this.buttonStart.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonStart.Location = new System.Drawing.Point(301, 371);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 4;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(220, 371);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // textBoxGeoSourceId
            // 
            this.textBoxGeoSourceId.AcceptsReturn = true;
            this.textBoxGeoSourceId.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxGeoSourceId.Location = new System.Drawing.Point(100, 329);
            this.textBoxGeoSourceId.Name = "textBoxGeoSourceId";
            this.textBoxGeoSourceId.Size = new System.Drawing.Size(276, 20);
            this.textBoxGeoSourceId.TabIndex = 6;
            this.textBoxGeoSourceId.TextChanged += new System.EventHandler(this.textBoxGeoSourceId_TextChanged);
            this.textBoxGeoSourceId.KeyDown += new System.Windows.Forms.KeyEventHandler(this.textBoxGeoSourceId_KeyDown);
            this.textBoxGeoSourceId.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBoxGeoSourceId_KeyPress);
            this.textBoxGeoSourceId.KeyUp += new System.Windows.Forms.KeyEventHandler(this.textBoxGeoSourceId_KeyUp);
            this.textBoxGeoSourceId.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.textBoxGeoSourceId_PreviewKeyDown);
            // 
            // listBoxGeoSources
            // 
            this.listBoxGeoSources.FormattingEnabled = true;
            this.listBoxGeoSources.Location = new System.Drawing.Point(100, 33);
            this.listBoxGeoSources.Name = "listBoxGeoSources";
            this.listBoxGeoSources.Size = new System.Drawing.Size(276, 290);
            this.listBoxGeoSources.TabIndex = 8;
            this.listBoxGeoSources.KeyUp += new System.Windows.Forms.KeyEventHandler(this.listBoxGeoSources_KeyUp);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 33);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(69, 13);
            this.label3.TabIndex = 9;
            this.label3.Text = "GeoSources:";
            // 
            // buttonVectors
            // 
            this.buttonVectors.Location = new System.Drawing.Point(12, 49);
            this.buttonVectors.Name = "buttonVectors";
            this.buttonVectors.Size = new System.Drawing.Size(75, 23);
            this.buttonVectors.TabIndex = 10;
            this.buttonVectors.Text = "Vectors";
            this.buttonVectors.UseVisualStyleBackColor = true;
            this.buttonVectors.Click += new System.EventHandler(this.buttonVectors_Click);
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(25, 310);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(69, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "0 ";
            this.label4.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // buttonCoverages
            // 
            this.buttonCoverages.Location = new System.Drawing.Point(12, 78);
            this.buttonCoverages.Name = "buttonCoverages";
            this.buttonCoverages.Size = new System.Drawing.Size(75, 23);
            this.buttonCoverages.TabIndex = 12;
            this.buttonCoverages.Text = "Coverages";
            this.buttonCoverages.UseVisualStyleBackColor = true;
            this.buttonCoverages.Click += new System.EventHandler(this.buttonCoverages_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(12, 136);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 13;
            this.buttonClear.Text = "Clear";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // checkBoxDownload
            // 
            this.checkBoxDownload.AutoSize = true;
            this.checkBoxDownload.Location = new System.Drawing.Point(100, 375);
            this.checkBoxDownload.Name = "checkBoxDownload";
            this.checkBoxDownload.Size = new System.Drawing.Size(113, 17);
            this.checkBoxDownload.TabIndex = 14;
            this.checkBoxDownload.Text = "download raw files";
            this.checkBoxDownload.UseVisualStyleBackColor = true;
            // 
            // buttonLastWeek
            // 
            this.buttonLastWeek.Location = new System.Drawing.Point(12, 107);
            this.buttonLastWeek.Name = "buttonLastWeek";
            this.buttonLastWeek.Size = new System.Drawing.Size(75, 23);
            this.buttonLastWeek.TabIndex = 15;
            this.buttonLastWeek.Text = "Last Week";
            this.buttonLastWeek.UseVisualStyleBackColor = true;
            this.buttonLastWeek.Click += new System.EventHandler(this.buttonLastWeek_Click);
            // 
            // ImportForm
            // 
            this.AcceptButton = this.buttonStart;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(388, 406);
            this.Controls.Add(this.buttonLastWeek);
            this.Controls.Add(this.checkBoxDownload);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonCoverages);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.buttonVectors);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.listBoxGeoSources);
            this.Controls.Add(this.textBoxGeoSourceId);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.numericUpDownInstanceCount);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "ImportForm";
            this.Text = "ImportForm";
            this.Load += new System.EventHandler(this.ImportForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownInstanceCount)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numericUpDownInstanceCount;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.TextBox textBoxGeoSourceId;
        private System.Windows.Forms.ListBox listBoxGeoSources;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button buttonVectors;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button buttonCoverages;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.CheckBox checkBoxDownload;
        private System.Windows.Forms.Button buttonLastWeek;
    }
}