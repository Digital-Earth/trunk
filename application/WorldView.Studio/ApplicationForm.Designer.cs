namespace Pyxis.WorldView.Studio
{
    partial class ApplicationForm
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
                components = null;
            }

            if (disposing && (Engine != null))
            {
                // Wait until the engine has started before stopping
                if (StartEngine != null)
                {
                    StartEngine.Wait();
                }

                // Stop the engine
                Engine.Stop();
                Engine = null;
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ApplicationForm));
            this.windowTitlePanel = new System.Windows.Forms.Panel();
            this.minimizeButton = new System.Windows.Forms.Button();
            this.maximizeButton = new System.Windows.Forms.Button();
            this.closeButton = new System.Windows.Forms.Button();
            this.urlPanel = new System.Windows.Forms.Panel();
            this.urlTextBox = new System.Windows.Forms.TextBox();
            this.PyxisView = new Pyxis.UI.PyxisView();
            this.windowTitlePanel.SuspendLayout();
            this.urlPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // windowTitlePanel
            // 
            this.windowTitlePanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.windowTitlePanel.Controls.Add(this.minimizeButton);
            this.windowTitlePanel.Controls.Add(this.maximizeButton);
            this.windowTitlePanel.Controls.Add(this.closeButton);
            this.windowTitlePanel.Cursor = System.Windows.Forms.Cursors.Arrow;
            this.windowTitlePanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.windowTitlePanel.Location = new System.Drawing.Point(2, 2);
            this.windowTitlePanel.Name = "windowTitlePanel";
            this.windowTitlePanel.Padding = new System.Windows.Forms.Padding(0, 0, 10, 0);
            this.windowTitlePanel.Size = new System.Drawing.Size(1196, 22);
            this.windowTitlePanel.TabIndex = 1;
            this.windowTitlePanel.Paint += new System.Windows.Forms.PaintEventHandler(this.windowTitlePanel_Paint);
            this.windowTitlePanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.windowTitlePanel_MouseMove);
            // 
            // minimizeButton
            // 
            this.minimizeButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.minimizeButton.FlatAppearance.BorderSize = 0;
            this.minimizeButton.FlatAppearance.MouseOverBackColor = System.Drawing.Color.Silver;
            this.minimizeButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.minimizeButton.Image = ((System.Drawing.Image)(resources.GetObject("minimizeButton.Image")));
            this.minimizeButton.Location = new System.Drawing.Point(1096, 0);
            this.minimizeButton.Name = "minimizeButton";
            this.minimizeButton.Size = new System.Drawing.Size(30, 22);
            this.minimizeButton.TabIndex = 1;
            this.minimizeButton.TabStop = false;
            this.minimizeButton.UseVisualStyleBackColor = true;
            this.minimizeButton.Click += new System.EventHandler(this.minimizeButton_Click);
            // 
            // maximizeButton
            // 
            this.maximizeButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.maximizeButton.FlatAppearance.BorderSize = 0;
            this.maximizeButton.FlatAppearance.MouseOverBackColor = System.Drawing.Color.Silver;
            this.maximizeButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.maximizeButton.Image = ((System.Drawing.Image)(resources.GetObject("maximizeButton.Image")));
            this.maximizeButton.Location = new System.Drawing.Point(1126, 0);
            this.maximizeButton.Name = "maximizeButton";
            this.maximizeButton.Size = new System.Drawing.Size(30, 22);
            this.maximizeButton.TabIndex = 2;
            this.maximizeButton.TabStop = false;
            this.maximizeButton.UseVisualStyleBackColor = true;
            this.maximizeButton.Click += new System.EventHandler(this.maximizeButton_Click);
            // 
            // closeButton
            // 
            this.closeButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.closeButton.FlatAppearance.BorderSize = 0;
            this.closeButton.FlatAppearance.MouseOverBackColor = System.Drawing.Color.Silver;
            this.closeButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.closeButton.Image = ((System.Drawing.Image)(resources.GetObject("closeButton.Image")));
            this.closeButton.Location = new System.Drawing.Point(1156, 0);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(30, 22);
            this.closeButton.TabIndex = 3;
            this.closeButton.TabStop = false;
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // urlPanel
            // 
            this.urlPanel.BackColor = System.Drawing.Color.Gainsboro;
            this.urlPanel.Controls.Add(this.urlTextBox);
            this.urlPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.urlPanel.Location = new System.Drawing.Point(2, 24);
            this.urlPanel.Name = "urlPanel";
            this.urlPanel.Padding = new System.Windows.Forms.Padding(2);
            this.urlPanel.Size = new System.Drawing.Size(1196, 28);
            this.urlPanel.TabIndex = 2;
            this.urlPanel.Visible = false;
            // 
            // urlTextBox
            // 
            this.urlTextBox.AcceptsReturn = true;
            this.urlTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.urlTextBox.Location = new System.Drawing.Point(2, 2);
            this.urlTextBox.Name = "urlTextBox";
            this.urlTextBox.Size = new System.Drawing.Size(1192, 21);
            this.urlTextBox.TabIndex = 0;
            this.urlTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.urlTextBox_KeyDown);
            this.urlTextBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.urlTextBox_KeyUp);
            // 
            // PyxisView
            // 
            this.PyxisView.AllowDrop = true;
            this.PyxisView.BackColor = System.Drawing.SystemColors.Control;
            this.PyxisView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PyxisView.Location = new System.Drawing.Point(2, 52);
            this.PyxisView.Margin = new System.Windows.Forms.Padding(6);
            this.PyxisView.Name = "PyxisView";
            this.PyxisView.Size = new System.Drawing.Size(1196, 746);
            this.PyxisView.TabIndex = 0;
            this.PyxisView.KeyUp += new System.Windows.Forms.KeyEventHandler(this.PyxisView_KeyUp);
            // 
            // ApplicationForm
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.ClientSize = new System.Drawing.Size(1200, 800);
            this.Controls.Add(this.PyxisView);
            this.Controls.Add(this.urlPanel);
            this.Controls.Add(this.windowTitlePanel);
            this.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ApplicationForm";
            this.Padding = new System.Windows.Forms.Padding(2);
            this.Text = "PYXIS WorldView Studio";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.ApplicationForm_FormClosed);
            this.Load += new System.EventHandler(this.ApplicationForm_Load);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.ApplicationForm_MouseMove);
            this.Resize += new System.EventHandler(this.ApplicationForm_Resize);
            this.windowTitlePanel.ResumeLayout(false);
            this.urlPanel.ResumeLayout(false);
            this.urlPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel windowTitlePanel;
        private System.Windows.Forms.Button minimizeButton;
        private System.Windows.Forms.Button maximizeButton;
        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.Panel urlPanel;
        private System.Windows.Forms.TextBox urlTextBox;
        internal UI.PyxisView PyxisView;
    }
}