namespace StudioLauncher
{
    partial class ControlModeForm
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
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.title = new System.Windows.Forms.Label();
            this.urlLabel = new System.Windows.Forms.Label();
            this.startupURLComboBox = new System.Windows.Forms.ComboBox();
            this.versionDataGridView = new System.Windows.Forms.DataGridView();
            this.testLabel = new System.Windows.Forms.Label();
            this.testDirectoryTextBox = new System.Windows.Forms.TextBox();
            this.browseButton = new System.Windows.Forms.Button();
            this.clearCacheCheckBox = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.versionDataGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(451, 553);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 1;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(355, 553);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 2;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // title
            // 
            this.title.AutoSize = true;
            this.title.Location = new System.Drawing.Point(23, 18);
            this.title.Name = "title";
            this.title.Size = new System.Drawing.Size(296, 17);
            this.title.TabIndex = 3;
            this.title.Text = "Select the version of WorldView Studio to run:";
            // 
            // urlLabel
            // 
            this.urlLabel.AutoSize = true;
            this.urlLabel.Location = new System.Drawing.Point(23, 407);
            this.urlLabel.Name = "urlLabel";
            this.urlLabel.Size = new System.Drawing.Size(270, 17);
            this.urlLabel.TabIndex = 4;
            this.urlLabel.Text = "Enter the startup URL (empty for default):";
            // 
            // startupURLComboBox
            // 
            this.startupURLComboBox.FormattingEnabled = true;
            this.startupURLComboBox.Location = new System.Drawing.Point(26, 438);
            this.startupURLComboBox.Name = "startupURLComboBox";
            this.startupURLComboBox.Size = new System.Drawing.Size(500, 24);
            this.startupURLComboBox.TabIndex = 6;
            // 
            // versionDataGridView
            // 
            this.versionDataGridView.AllowUserToAddRows = false;
            this.versionDataGridView.AllowUserToDeleteRows = false;
            this.versionDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.versionDataGridView.Location = new System.Drawing.Point(26, 49);
            this.versionDataGridView.Name = "versionDataGridView";
            this.versionDataGridView.ReadOnly = true;
            this.versionDataGridView.RowTemplate.Height = 24;
            this.versionDataGridView.Size = new System.Drawing.Size(500, 338);
            this.versionDataGridView.TabIndex = 7;
            // 
            // testLabel
            // 
            this.testLabel.AutoSize = true;
            this.testLabel.Location = new System.Drawing.Point(23, 475);
            this.testLabel.Name = "testLabel";
            this.testLabel.Size = new System.Drawing.Size(347, 17);
            this.testLabel.TabIndex = 8;
            this.testLabel.Text = "Enter the geosource test directory (empty for no test):";
            // 
            // testDirectoryTextBox
            // 
            this.testDirectoryTextBox.Location = new System.Drawing.Point(26, 505);
            this.testDirectoryTextBox.Name = "testDirectoryTextBox";
            this.testDirectoryTextBox.Size = new System.Drawing.Size(404, 22);
            this.testDirectoryTextBox.TabIndex = 9;
            // 
            // browseButton
            // 
            this.browseButton.Location = new System.Drawing.Point(451, 505);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(75, 23);
            this.browseButton.TabIndex = 10;
            this.browseButton.Text = "Browse...";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // clearCacheCheckBox
            // 
            this.clearCacheCheckBox.AutoSize = true;
            this.clearCacheCheckBox.Location = new System.Drawing.Point(26, 553);
            this.clearCacheCheckBox.Name = "clearCacheCheckBox";
            this.clearCacheCheckBox.Size = new System.Drawing.Size(137, 21);
            this.clearCacheCheckBox.TabIndex = 11;
            this.clearCacheCheckBox.Text = "Clear data cache";
            this.clearCacheCheckBox.UseVisualStyleBackColor = true;
            // 
            // ControlModeForm
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(555, 597);
            this.Controls.Add(this.clearCacheCheckBox);
            this.Controls.Add(this.browseButton);
            this.Controls.Add(this.testDirectoryTextBox);
            this.Controls.Add(this.testLabel);
            this.Controls.Add(this.versionDataGridView);
            this.Controls.Add(this.startupURLComboBox);
            this.Controls.Add(this.urlLabel);
            this.Controls.Add(this.title);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Name = "ControlModeForm";
            this.Text = "WorldView Studio Launcher Control Mode";
            ((System.ComponentModel.ISupportInitialize)(this.versionDataGridView)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Label title;
        private System.Windows.Forms.Label urlLabel;
        private System.Windows.Forms.ComboBox startupURLComboBox;
        private System.Windows.Forms.DataGridView versionDataGridView;
        private System.Windows.Forms.Label testLabel;
        private System.Windows.Forms.TextBox testDirectoryTextBox;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.CheckBox clearCacheCheckBox;
    }
}