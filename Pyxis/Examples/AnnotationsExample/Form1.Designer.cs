namespace AnnotationsExample
{
    partial class Form1
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
            this.pyxisEngineApiFactory1 = new Pyxis.UI.PyxisEngineApiFactory(this.components);
            this.pyxisView1 = new Pyxis.UI.PyxisView();
            this.infoTableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.wellIdLabel = new System.Windows.Forms.Label();
            this.wellOperatorLabel = new System.Windows.Forms.Label();
            this.wellStatusLabel = new System.Windows.Forms.Label();
            this.wellDepthLabel = new System.Windows.Forms.Label();
            this.wellProductionLabel = new System.Windows.Forms.Label();
            this.infoTableLayoutPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // pyxisEngineApiFactory1
            // 
            this.pyxisEngineApiFactory1.ApplicationKey = "502EC10B8A6D490E9B81663B59CB8693";
            this.pyxisEngineApiFactory1.EnablePyxNet = false;
            this.pyxisEngineApiFactory1.UserEmail = "info@pyxisinnovation.com";
            // 
            // pyxisView1
            // 
            this.pyxisView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pyxisView1.Location = new System.Drawing.Point(0, 0);
            this.pyxisView1.Name = "pyxisView1";
            this.pyxisView1.Size = new System.Drawing.Size(991, 557);
            this.pyxisView1.TabIndex = 0;
            // 
            // infoTableLayoutPanel
            // 
            this.infoTableLayoutPanel.ColumnCount = 2;
            this.infoTableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.infoTableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.infoTableLayoutPanel.Controls.Add(this.label1, 0, 0);
            this.infoTableLayoutPanel.Controls.Add(this.label2, 0, 1);
            this.infoTableLayoutPanel.Controls.Add(this.label3, 0, 2);
            this.infoTableLayoutPanel.Controls.Add(this.label4, 0, 3);
            this.infoTableLayoutPanel.Controls.Add(this.label5, 0, 4);
            this.infoTableLayoutPanel.Controls.Add(this.wellIdLabel, 1, 0);
            this.infoTableLayoutPanel.Controls.Add(this.wellOperatorLabel, 1, 1);
            this.infoTableLayoutPanel.Controls.Add(this.wellStatusLabel, 1, 2);
            this.infoTableLayoutPanel.Controls.Add(this.wellDepthLabel, 1, 3);
            this.infoTableLayoutPanel.Controls.Add(this.wellProductionLabel, 1, 4);
            this.infoTableLayoutPanel.Location = new System.Drawing.Point(24, 21);
            this.infoTableLayoutPanel.Name = "infoTableLayoutPanel";
            this.infoTableLayoutPanel.RowCount = 5;
            this.infoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
            this.infoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
            this.infoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
            this.infoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
            this.infoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
            this.infoTableLayoutPanel.Size = new System.Drawing.Size(281, 168);
            this.infoTableLayoutPanel.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(58, 33);
            this.label1.TabIndex = 0;
            this.label1.Text = "Well ID";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 33);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 33);
            this.label2.TabIndex = 1;
            this.label2.Text = "Operator";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 66);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 33);
            this.label3.TabIndex = 2;
            this.label3.Text = "Status";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 99);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(58, 33);
            this.label4.TabIndex = 3;
            this.label4.Text = "Depth";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 132);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(58, 36);
            this.label5.TabIndex = 4;
            this.label5.Text = "Production";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // wellIdLabel
            // 
            this.wellIdLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wellIdLabel.AutoSize = true;
            this.wellIdLabel.Location = new System.Drawing.Point(67, 0);
            this.wellIdLabel.Name = "wellIdLabel";
            this.wellIdLabel.Size = new System.Drawing.Size(211, 33);
            this.wellIdLabel.TabIndex = 5;
            this.wellIdLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // wellOperatorLabel
            // 
            this.wellOperatorLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wellOperatorLabel.AutoSize = true;
            this.wellOperatorLabel.Location = new System.Drawing.Point(67, 33);
            this.wellOperatorLabel.Name = "wellOperatorLabel";
            this.wellOperatorLabel.Size = new System.Drawing.Size(211, 33);
            this.wellOperatorLabel.TabIndex = 6;
            this.wellOperatorLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // wellStatusLabel
            // 
            this.wellStatusLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wellStatusLabel.AutoSize = true;
            this.wellStatusLabel.Location = new System.Drawing.Point(67, 66);
            this.wellStatusLabel.Name = "wellStatusLabel";
            this.wellStatusLabel.Size = new System.Drawing.Size(211, 33);
            this.wellStatusLabel.TabIndex = 7;
            this.wellStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // wellDepthLabel
            // 
            this.wellDepthLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wellDepthLabel.AutoSize = true;
            this.wellDepthLabel.Location = new System.Drawing.Point(67, 99);
            this.wellDepthLabel.Name = "wellDepthLabel";
            this.wellDepthLabel.Size = new System.Drawing.Size(211, 33);
            this.wellDepthLabel.TabIndex = 8;
            this.wellDepthLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // wellProductionLabel
            // 
            this.wellProductionLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wellProductionLabel.AutoSize = true;
            this.wellProductionLabel.Location = new System.Drawing.Point(67, 132);
            this.wellProductionLabel.Name = "wellProductionLabel";
            this.wellProductionLabel.Size = new System.Drawing.Size(211, 36);
            this.wellProductionLabel.TabIndex = 9;
            this.wellProductionLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(991, 557);
            this.Controls.Add(this.infoTableLayoutPanel);
            this.Controls.Add(this.pyxisView1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.infoTableLayoutPanel.ResumeLayout(false);
            this.infoTableLayoutPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private Pyxis.UI.PyxisEngineApiFactory pyxisEngineApiFactory1;
        private Pyxis.UI.PyxisView pyxisView1;
        private System.Windows.Forms.TableLayoutPanel infoTableLayoutPanel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label wellIdLabel;
        private System.Windows.Forms.Label wellOperatorLabel;
        private System.Windows.Forms.Label wellStatusLabel;
        private System.Windows.Forms.Label wellDepthLabel;
        private System.Windows.Forms.Label wellProductionLabel;
    }
}

