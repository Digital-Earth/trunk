namespace ApplicationUtility
{
    partial class PYXAboutBox
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
            this.m_appNameText = new System.Windows.Forms.Label();
            this.m_versionLabel = new System.Windows.Forms.Label();
            this.m_versionText = new System.Windows.Forms.Label();
            this.m_copyrightText = new System.Windows.Forms.Label();
            this.m_copyrightTextBox = new System.Windows.Forms.TextBox();
            this.btnClose = new System.Windows.Forms.Button();
            this.websiteLink = new System.Windows.Forms.LinkLabel();
            this.feedbackLink = new System.Windows.Forms.LinkLabel();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // m_appNameText
            // 
            this.m_appNameText.AutoSize = true;
            this.m_appNameText.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_appNameText.Location = new System.Drawing.Point(198, 9);
            this.m_appNameText.Name = "m_appNameText";
            this.m_appNameText.Size = new System.Drawing.Size(149, 20);
            this.m_appNameText.TabIndex = 0;
            this.m_appNameText.Text = "Application Name";
            // 
            // m_versionLabel
            // 
            this.m_versionLabel.AutoSize = true;
            this.m_versionLabel.Location = new System.Drawing.Point(199, 38);
            this.m_versionLabel.Name = "m_versionLabel";
            this.m_versionLabel.Size = new System.Drawing.Size(45, 13);
            this.m_versionLabel.TabIndex = 1;
            this.m_versionLabel.Text = "Version:";
            // 
            // m_versionText
            // 
            this.m_versionText.AutoSize = true;
            this.m_versionText.Location = new System.Drawing.Point(249, 38);
            this.m_versionText.Name = "m_versionText";
            this.m_versionText.Size = new System.Drawing.Size(97, 13);
            this.m_versionText.TabIndex = 2;
            this.m_versionText.Text = "Major.Minor.0.Build";
            // 
            // m_copyrightText
            // 
            this.m_copyrightText.AutoSize = true;
            this.m_copyrightText.Location = new System.Drawing.Point(12, 116);
            this.m_copyrightText.Name = "m_copyrightText";
            this.m_copyrightText.Size = new System.Drawing.Size(51, 13);
            this.m_copyrightText.TabIndex = 5;
            this.m_copyrightText.Text = "Copyright";
            // 
            // m_copyrightTextBox
            // 
            this.m_copyrightTextBox.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.m_copyrightTextBox.HideSelection = false;
            this.m_copyrightTextBox.Location = new System.Drawing.Point(12, 132);
            this.m_copyrightTextBox.MaxLength = 10000;
            this.m_copyrightTextBox.Multiline = true;
            this.m_copyrightTextBox.Name = "m_copyrightTextBox";
            this.m_copyrightTextBox.ReadOnly = true;
            this.m_copyrightTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.m_copyrightTextBox.Size = new System.Drawing.Size(423, 136);
            this.m_copyrightTextBox.TabIndex = 6;
            // 
            // btnClose
            // 
            this.btnClose.Location = new System.Drawing.Point(353, 274);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(82, 26);
            this.btnClose.TabIndex = 7;
            this.btnClose.Text = "Close";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // websiteLink
            // 
            this.websiteLink.AutoSize = true;
            this.websiteLink.Location = new System.Drawing.Point(199, 61);
            this.websiteLink.Name = "websiteLink";
            this.websiteLink.Size = new System.Drawing.Size(129, 13);
            this.websiteLink.TabIndex = 9;
            this.websiteLink.TabStop = true;
            this.websiteLink.Text = "www.pyxisinnovation.com";
            this.websiteLink.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.websiteLink_Clicked);
            // 
            // feedbackLink
            // 
            this.feedbackLink.AutoSize = true;
            this.feedbackLink.Location = new System.Drawing.Point(198, 84);
            this.feedbackLink.Name = "feedbackLink";
            this.feedbackLink.Size = new System.Drawing.Size(158, 13);
            this.feedbackLink.TabIndex = 10;
            this.feedbackLink.TabStop = true;
            this.feedbackLink.Text = "feedback@pyxisinnovation.com";
            this.feedbackLink.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.mailto_LinkClicked);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::ApplicationUtility.Properties.Resources.Full_Logo_no_background;
            this.pictureBox1.Location = new System.Drawing.Point(12, 9);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(180, 104);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 8;
            this.pictureBox1.TabStop = false;
            // 
            // PYXAboutBox
            // 
            this.AcceptButton = this.btnClose;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(443, 306);
            this.ControlBox = false;
            this.Controls.Add(this.feedbackLink);
            this.Controls.Add(this.websiteLink);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.m_copyrightTextBox);
            this.Controls.Add(this.m_copyrightText);
            this.Controls.Add(this.m_versionText);
            this.Controls.Add(this.m_versionLabel);
            this.Controls.Add(this.m_appNameText);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PYXAboutBox";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "About ";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label m_appNameText;
        private System.Windows.Forms.Label m_versionLabel;
        private System.Windows.Forms.Label m_versionText;
        private System.Windows.Forms.Label m_copyrightText;
        private System.Windows.Forms.TextBox m_copyrightTextBox;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.LinkLabel websiteLink;
        private System.Windows.Forms.LinkLabel feedbackLink;

    }
}