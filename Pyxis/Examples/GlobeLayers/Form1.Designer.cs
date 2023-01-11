namespace GlobeLayers
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
            this.mapLayersGroupBox = new System.Windows.Forms.GroupBox();
            this.saveButton = new System.Windows.Forms.Button();
            this.toporamaCheckBox = new System.Windows.Forms.CheckBox();
            this.lidarGotoButton = new System.Windows.Forms.Button();
            this.globalPipelinesCheckBox = new System.Windows.Forms.CheckBox();
            this.bingStylesComboBox = new System.Windows.Forms.ComboBox();
            this.worldPoliticalCheckBox = new System.Windows.Forms.CheckBox();
            this.naturalEarthCheckBox = new System.Windows.Forms.CheckBox();
            this.bingCheckBox = new System.Windows.Forms.CheckBox();
            this.lidarCheckBox = new System.Windows.Forms.CheckBox();
            this.elevation30CheckBox = new System.Windows.Forms.CheckBox();
            this.elevation2CheckBox = new System.Windows.Forms.CheckBox();
            this.loadingPanel = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.pyxisView1 = new Pyxis.UI.PyxisView();
            this.pyxisEngineApiFactory1 = new Pyxis.UI.PyxisEngineApiFactory(this.components);
            this.mapLayersGroupBox.SuspendLayout();
            this.loadingPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // mapLayersGroupBox
            // 
            this.mapLayersGroupBox.Controls.Add(this.saveButton);
            this.mapLayersGroupBox.Controls.Add(this.toporamaCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.lidarGotoButton);
            this.mapLayersGroupBox.Controls.Add(this.globalPipelinesCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.bingStylesComboBox);
            this.mapLayersGroupBox.Controls.Add(this.worldPoliticalCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.naturalEarthCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.bingCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.lidarCheckBox);
            this.mapLayersGroupBox.Controls.Add(this.elevation30CheckBox);
            this.mapLayersGroupBox.Controls.Add(this.elevation2CheckBox);
            this.mapLayersGroupBox.Location = new System.Drawing.Point(12, 12);
            this.mapLayersGroupBox.Name = "mapLayersGroupBox";
            this.mapLayersGroupBox.Size = new System.Drawing.Size(200, 286);
            this.mapLayersGroupBox.TabIndex = 1;
            this.mapLayersGroupBox.TabStop = false;
            this.mapLayersGroupBox.Text = "Map";
            this.mapLayersGroupBox.Visible = false;
            // 
            // saveButton
            // 
            this.saveButton.Location = new System.Drawing.Point(6, 257);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(188, 23);
            this.saveButton.TabIndex = 10;
            this.saveButton.Text = "Save To Image...";
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // toporamaCheckBox
            // 
            this.toporamaCheckBox.AutoSize = true;
            this.toporamaCheckBox.Location = new System.Drawing.Point(6, 161);
            this.toporamaCheckBox.Name = "toporamaCheckBox";
            this.toporamaCheckBox.Size = new System.Drawing.Size(138, 19);
            this.toporamaCheckBox.TabIndex = 9;
            this.toporamaCheckBox.Text = "TOPORAMA NRCAN";
            this.toporamaCheckBox.UseVisualStyleBackColor = true;
            // 
            // lidarGotoButton
            // 
            this.lidarGotoButton.Location = new System.Drawing.Point(59, 61);
            this.lidarGotoButton.Name = "lidarGotoButton";
            this.lidarGotoButton.Size = new System.Drawing.Size(47, 23);
            this.lidarGotoButton.TabIndex = 8;
            this.lidarGotoButton.Text = "GoTo";
            this.lidarGotoButton.UseVisualStyleBackColor = true;
            // 
            // globalPipelinesCheckBox
            // 
            this.globalPipelinesCheckBox.AutoSize = true;
            this.globalPipelinesCheckBox.Location = new System.Drawing.Point(6, 226);
            this.globalPipelinesCheckBox.Name = "globalPipelinesCheckBox";
            this.globalPipelinesCheckBox.Size = new System.Drawing.Size(115, 19);
            this.globalPipelinesCheckBox.TabIndex = 7;
            this.globalPipelinesCheckBox.Text = "Global pipelines";
            this.globalPipelinesCheckBox.UseVisualStyleBackColor = true;
            // 
            // bingStylesComboBox
            // 
            this.bingStylesComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.bingStylesComboBox.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.bingStylesComboBox.FormattingEnabled = true;
            this.bingStylesComboBox.Location = new System.Drawing.Point(59, 111);
            this.bingStylesComboBox.Name = "bingStylesComboBox";
            this.bingStylesComboBox.Size = new System.Drawing.Size(121, 21);
            this.bingStylesComboBox.TabIndex = 6;
            // 
            // worldPoliticalCheckBox
            // 
            this.worldPoliticalCheckBox.AutoSize = true;
            this.worldPoliticalCheckBox.Location = new System.Drawing.Point(6, 203);
            this.worldPoliticalCheckBox.Name = "worldPoliticalCheckBox";
            this.worldPoliticalCheckBox.Size = new System.Drawing.Size(163, 19);
            this.worldPoliticalCheckBox.TabIndex = 5;
            this.worldPoliticalCheckBox.Text = "World Political Boundries";
            this.worldPoliticalCheckBox.UseVisualStyleBackColor = true;
            // 
            // naturalEarthCheckBox
            // 
            this.naturalEarthCheckBox.AutoSize = true;
            this.naturalEarthCheckBox.Location = new System.Drawing.Point(6, 138);
            this.naturalEarthCheckBox.Name = "naturalEarthCheckBox";
            this.naturalEarthCheckBox.Size = new System.Drawing.Size(98, 19);
            this.naturalEarthCheckBox.TabIndex = 4;
            this.naturalEarthCheckBox.Text = "Natural Earth";
            this.naturalEarthCheckBox.UseVisualStyleBackColor = true;
            // 
            // bingCheckBox
            // 
            this.bingCheckBox.AutoSize = true;
            this.bingCheckBox.Location = new System.Drawing.Point(6, 113);
            this.bingCheckBox.Name = "bingCheckBox";
            this.bingCheckBox.Size = new System.Drawing.Size(51, 19);
            this.bingCheckBox.TabIndex = 3;
            this.bingCheckBox.Text = "Bing";
            this.bingCheckBox.UseVisualStyleBackColor = true;
            // 
            // lidarCheckBox
            // 
            this.lidarCheckBox.AutoSize = true;
            this.lidarCheckBox.Location = new System.Drawing.Point(6, 65);
            this.lidarCheckBox.Name = "lidarCheckBox";
            this.lidarCheckBox.Size = new System.Drawing.Size(54, 19);
            this.lidarCheckBox.TabIndex = 2;
            this.lidarCheckBox.Text = "Lidar";
            this.lidarCheckBox.UseVisualStyleBackColor = true;
            // 
            // elevation30CheckBox
            // 
            this.elevation30CheckBox.AutoSize = true;
            this.elevation30CheckBox.Location = new System.Drawing.Point(6, 42);
            this.elevation30CheckBox.Name = "elevation30CheckBox";
            this.elevation30CheckBox.Size = new System.Drawing.Size(144, 19);
            this.elevation30CheckBox.TabIndex = 1;
            this.elevation30CheckBox.Text = "Elevation 30 Seconds";
            this.elevation30CheckBox.UseVisualStyleBackColor = true;
            // 
            // elevation2CheckBox
            // 
            this.elevation2CheckBox.AutoSize = true;
            this.elevation2CheckBox.Location = new System.Drawing.Point(6, 19);
            this.elevation2CheckBox.Name = "elevation2CheckBox";
            this.elevation2CheckBox.Size = new System.Drawing.Size(133, 19);
            this.elevation2CheckBox.TabIndex = 0;
            this.elevation2CheckBox.Text = "Elevation 2 Minutes";
            this.elevation2CheckBox.UseVisualStyleBackColor = true;
            // 
            // loadingPanel
            // 
            this.loadingPanel.Controls.Add(this.label1);
            this.loadingPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.loadingPanel.Location = new System.Drawing.Point(0, 353);
            this.loadingPanel.Name = "loadingPanel";
            this.loadingPanel.Size = new System.Drawing.Size(1023, 59);
            this.loadingPanel.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(1023, 59);
            this.label1.TabIndex = 0;
            this.label1.Text = "Loading...";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // pyxisView1
            // 
            this.pyxisView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pyxisView1.Location = new System.Drawing.Point(0, 0);
            this.pyxisView1.Name = "pyxisView1";
            this.pyxisView1.Size = new System.Drawing.Size(1023, 412);
            this.pyxisView1.TabIndex = 0;
            // 
            // pyxisEngineApiFactory1
            // 
            this.pyxisEngineApiFactory1.ApplicationKey = "502EC10B8A6D490E9B81663B59CB8693";
            this.pyxisEngineApiFactory1.EnablePyxNet = true;
            this.pyxisEngineApiFactory1.UserEmail = "info@pyxisinnovation.com";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1023, 412);
            this.Controls.Add(this.loadingPanel);
            this.Controls.Add(this.mapLayersGroupBox);
            this.Controls.Add(this.pyxisView1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.mapLayersGroupBox.ResumeLayout(false);
            this.mapLayersGroupBox.PerformLayout();
            this.loadingPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private Pyxis.UI.PyxisView pyxisView1;
        private System.Windows.Forms.GroupBox mapLayersGroupBox;
        private System.Windows.Forms.Button lidarGotoButton;
        private System.Windows.Forms.CheckBox globalPipelinesCheckBox;
        private System.Windows.Forms.ComboBox bingStylesComboBox;
        private System.Windows.Forms.CheckBox worldPoliticalCheckBox;
        private System.Windows.Forms.CheckBox naturalEarthCheckBox;
        private System.Windows.Forms.CheckBox bingCheckBox;
        private System.Windows.Forms.CheckBox lidarCheckBox;
        private System.Windows.Forms.CheckBox elevation30CheckBox;
        private System.Windows.Forms.CheckBox elevation2CheckBox;
        private System.Windows.Forms.Panel loadingPanel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox toporamaCheckBox;
        private Pyxis.UI.PyxisEngineApiFactory pyxisEngineApiFactory1;
        private System.Windows.Forms.Button saveButton;
    }
}

