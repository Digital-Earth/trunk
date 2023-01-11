namespace PyxNet.Diagnostics
{
    partial class TrafficListenerForm
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.StopButton = new System.Windows.Forms.Button();
            this.StartButton = new System.Windows.Forms.Button();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.MessageListView = new System.Windows.Forms.ListView();
            this.TimeStampColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.SourceColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.DestinationColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.MessageIDColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.MessageSizeColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.MessageTreeView = new System.Windows.Forms.TreeView();
            this.MessageBitsTextBox = new System.Windows.Forms.TextBox();
            this.MessageDescriptionColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.panel1.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.StopButton);
            this.panel1.Controls.Add(this.StartButton);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(1118, 36);
            this.panel1.TabIndex = 0;
            // 
            // StopButton
            // 
            this.StopButton.Location = new System.Drawing.Point(93, 7);
            this.StopButton.Name = "StopButton";
            this.StopButton.Size = new System.Drawing.Size(75, 23);
            this.StopButton.TabIndex = 1;
            this.StopButton.Text = "Stop";
            this.StopButton.UseVisualStyleBackColor = true;
            this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // StartButton
            // 
            this.StartButton.Location = new System.Drawing.Point(12, 7);
            this.StartButton.Name = "StartButton";
            this.StartButton.Size = new System.Drawing.Size(75, 23);
            this.StartButton.TabIndex = 0;
            this.StartButton.Text = "Start";
            this.StartButton.UseVisualStyleBackColor = true;
            this.StartButton.Click += new System.EventHandler(this.StartButton_Click);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 36);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.MessageListView);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
            this.splitContainer1.Size = new System.Drawing.Size(1118, 491);
            this.splitContainer1.SplitterDistance = 715;
            this.splitContainer1.TabIndex = 1;
            // 
            // MessageListView
            // 
            this.MessageListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.TimeStampColumnHeader,
            this.SourceColumnHeader,
            this.DestinationColumnHeader,
            this.MessageIDColumnHeader,
            this.MessageSizeColumnHeader,
            this.MessageDescriptionColumnHeader});
            this.MessageListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MessageListView.Location = new System.Drawing.Point(0, 0);
            this.MessageListView.Name = "MessageListView";
            this.MessageListView.Size = new System.Drawing.Size(715, 491);
            this.MessageListView.TabIndex = 0;
            this.MessageListView.UseCompatibleStateImageBehavior = false;
            this.MessageListView.View = System.Windows.Forms.View.Details;
            this.MessageListView.SelectedIndexChanged += new System.EventHandler(this.MessageListView_SelectedIndexChanged);
            // 
            // TimeStampColumnHeader
            // 
            this.TimeStampColumnHeader.Text = "Time";
            this.TimeStampColumnHeader.Width = 103;
            // 
            // SourceColumnHeader
            // 
            this.SourceColumnHeader.Text = "Source";
            this.SourceColumnHeader.Width = 161;
            // 
            // DestinationColumnHeader
            // 
            this.DestinationColumnHeader.Text = "Destination";
            this.DestinationColumnHeader.Width = 151;
            // 
            // MessageIDColumnHeader
            // 
            this.MessageIDColumnHeader.Text = "ID";
            // 
            // MessageSizeColumnHeader
            // 
            this.MessageSizeColumnHeader.Text = "Size";
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.MessageTreeView);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.MessageBitsTextBox);
            this.splitContainer2.Size = new System.Drawing.Size(399, 491);
            this.splitContainer2.SplitterDistance = 303;
            this.splitContainer2.TabIndex = 0;
            // 
            // MessageTreeView
            // 
            this.MessageTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MessageTreeView.Location = new System.Drawing.Point(0, 0);
            this.MessageTreeView.Name = "MessageTreeView";
            this.MessageTreeView.Size = new System.Drawing.Size(399, 303);
            this.MessageTreeView.TabIndex = 0;
            // 
            // MessageBitsTextBox
            // 
            this.MessageBitsTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MessageBitsTextBox.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MessageBitsTextBox.Location = new System.Drawing.Point(0, 0);
            this.MessageBitsTextBox.Multiline = true;
            this.MessageBitsTextBox.Name = "MessageBitsTextBox";
            this.MessageBitsTextBox.ReadOnly = true;
            this.MessageBitsTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.MessageBitsTextBox.Size = new System.Drawing.Size(399, 184);
            this.MessageBitsTextBox.TabIndex = 0;
            // 
            // MessageDescriptionColumnHeader
            // 
            this.MessageDescriptionColumnHeader.Text = "Description";
            this.MessageDescriptionColumnHeader.Width = 176;
            // 
            // TrafficListenerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1118, 527);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.panel1);
            this.Name = "TrafficListenerForm";
            this.Text = "PyxNet Traffic Viewer";
            this.Load += new System.EventHandler(this.TrafficListenerForm_Load);
            this.panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            this.splitContainer2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ListView MessageListView;
        private System.Windows.Forms.ColumnHeader TimeStampColumnHeader;
        private System.Windows.Forms.ColumnHeader SourceColumnHeader;
        private System.Windows.Forms.ColumnHeader DestinationColumnHeader;
        private System.Windows.Forms.ColumnHeader MessageIDColumnHeader;
        private System.Windows.Forms.ColumnHeader MessageSizeColumnHeader;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.TreeView MessageTreeView;
        private System.Windows.Forms.TextBox MessageBitsTextBox;
        private System.Windows.Forms.Button StopButton;
        private System.Windows.Forms.Button StartButton;
        private System.Windows.Forms.ColumnHeader MessageDescriptionColumnHeader;
    }
}