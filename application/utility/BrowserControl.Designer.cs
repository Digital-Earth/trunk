namespace ApplicationUtility
{
    partial class BrowserControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(BrowserControl));
            this.m_webBrowser = new System.Windows.Forms.WebBrowser();
            this.SuspendLayout();
            // 
            // m_webBrowser
            // 
            this.m_webBrowser.AllowWebBrowserDrop = false;
            this.m_webBrowser.Dock = System.Windows.Forms.DockStyle.Fill;
            this.m_webBrowser.Location = new System.Drawing.Point(0, 0);
            this.m_webBrowser.MinimumSize = new System.Drawing.Size(20, 20);
            this.m_webBrowser.Name = "m_webBrowser";
            this.m_webBrowser.Size = new System.Drawing.Size(1014, 733);
            this.m_webBrowser.TabIndex = 0;
            // 
            // BrowserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1014, 733);
            this.Controls.Add(this.m_webBrowser);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "BrowserControl";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.WebBrowser m_webBrowser;
    }
}

