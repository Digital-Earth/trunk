namespace MD5Generator
{
    partial class MD5Form
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
            this.textBoxSelectedFile = new System.Windows.Forms.TextBox();
            this.buttonSelectFile = new System.Windows.Forms.Button();
            this.labelMD5 = new System.Windows.Forms.Label();
            this.labelGeneratedMD5 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBoxSelectedFile.Location = new System.Drawing.Point(12, 12);
            this.textBoxSelectedFile.Name = "textBox1";
            this.textBoxSelectedFile.Size = new System.Drawing.Size(329, 20);
            this.textBoxSelectedFile.TabIndex = 0;
            // 
            // buttonSelectFile
            // 
            this.buttonSelectFile.Location = new System.Drawing.Point(371, 10);
            this.buttonSelectFile.Name = "buttonSelectFile";
            this.buttonSelectFile.Size = new System.Drawing.Size(75, 23);
            this.buttonSelectFile.TabIndex = 1;
            this.buttonSelectFile.Text = "Select File...";
            this.buttonSelectFile.UseVisualStyleBackColor = true;
            this.buttonSelectFile.Click += new System.EventHandler(this.ButtonSelectFile_Click);
            // 
            // labelMD5
            // 
            this.labelMD5.AutoSize = true;
            this.labelMD5.Location = new System.Drawing.Point(12, 49);
            this.labelMD5.Name = "labelMD5";
            this.labelMD5.Size = new System.Drawing.Size(33, 13);
            this.labelMD5.TabIndex = 2;
            this.labelMD5.Text = "MD5:";
            // 
            // labelGeneratedMD5
            // 
            this.labelGeneratedMD5.AutoSize = true;
            this.labelGeneratedMD5.Location = new System.Drawing.Point(51, 49);
            this.labelGeneratedMD5.Name = "labelGeneratedMD5";
            this.labelGeneratedMD5.Size = new System.Drawing.Size(60, 13);
            this.labelGeneratedMD5.TabIndex = 3;
            this.labelGeneratedMD5.Text = "select file...";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(468, 80);
            this.Controls.Add(this.labelGeneratedMD5);
            this.Controls.Add(this.labelMD5);
            this.Controls.Add(this.buttonSelectFile);
            this.Controls.Add(this.textBoxSelectedFile);
            this.Name = "Form1";
            this.Text = "MD5 Generator";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxSelectedFile;
        private System.Windows.Forms.Button buttonSelectFile;
        private System.Windows.Forms.Label labelMD5;
        private System.Windows.Forms.Label labelGeneratedMD5;
    }
}

