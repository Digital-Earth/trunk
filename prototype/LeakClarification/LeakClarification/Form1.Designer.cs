namespace LeakClarification
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
            this.buttonExample1 = new System.Windows.Forms.Button();
            this.buttonQuit = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.buttonExample4 = new System.Windows.Forms.Button();
            this.buttonExample5 = new System.Windows.Forms.Button();
            this.buttonExample6 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonExample1
            // 
            this.buttonExample1.Location = new System.Drawing.Point(35, 26);
            this.buttonExample1.Name = "buttonExample1";
            this.buttonExample1.Size = new System.Drawing.Size(206, 23);
            this.buttonExample1.TabIndex = 0;
            this.buttonExample1.Text = "Run Example 1";
            this.buttonExample1.UseVisualStyleBackColor = true;
            this.buttonExample1.Click += new System.EventHandler(this.buttonExample1_Click);
            // 
            // buttonQuit
            // 
            this.buttonQuit.Location = new System.Drawing.Point(89, 346);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(86, 23);
            this.buttonQuit.TabIndex = 1;
            this.buttonQuit.Text = "&Quit";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(35, 55);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(206, 23);
            this.button1.TabIndex = 2;
            this.button1.Text = "Run Example 2";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.buttonExample2_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(35, 84);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(206, 23);
            this.button2.TabIndex = 3;
            this.button2.Text = "Run Example 3";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.buttonExample3_Click);
            // 
            // buttonExample4
            // 
            this.buttonExample4.Location = new System.Drawing.Point(35, 113);
            this.buttonExample4.Name = "buttonExample4";
            this.buttonExample4.Size = new System.Drawing.Size(206, 23);
            this.buttonExample4.TabIndex = 4;
            this.buttonExample4.Text = "Run Example 4";
            this.buttonExample4.UseVisualStyleBackColor = true;
            this.buttonExample4.Click += new System.EventHandler(this.buttonExample4_Click);
            // 
            // buttonExample5
            // 
            this.buttonExample5.Location = new System.Drawing.Point(35, 142);
            this.buttonExample5.Name = "buttonExample5";
            this.buttonExample5.Size = new System.Drawing.Size(206, 23);
            this.buttonExample5.TabIndex = 5;
            this.buttonExample5.Text = "Run Example 5";
            this.buttonExample5.UseVisualStyleBackColor = true;
            this.buttonExample5.Click += new System.EventHandler(this.buttonExample5_Click);
            // 
            // buttonExample6
            // 
            this.buttonExample6.Location = new System.Drawing.Point(35, 171);
            this.buttonExample6.Name = "buttonExample6";
            this.buttonExample6.Size = new System.Drawing.Size(206, 23);
            this.buttonExample6.TabIndex = 6;
            this.buttonExample6.Text = "Run Example 6";
            this.buttonExample6.UseVisualStyleBackColor = true;
            this.buttonExample6.Click += new System.EventHandler(this.buttonExample6_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(274, 381);
            this.Controls.Add(this.buttonExample6);
            this.Controls.Add(this.buttonExample5);
            this.Controls.Add(this.buttonExample4);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.buttonQuit);
            this.Controls.Add(this.buttonExample1);
            this.Name = "MainForm";
            this.Text = "Memory Leaking Examples";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonExample1;
        private System.Windows.Forms.Button buttonQuit;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button buttonExample4;
        private System.Windows.Forms.Button buttonExample5;
        private System.Windows.Forms.Button buttonExample6;
    }
}

