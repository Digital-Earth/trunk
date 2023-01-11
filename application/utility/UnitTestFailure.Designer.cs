namespace ApplicationUtility
{
    partial class UnitTestFailure
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UnitTestFailure));
            this.label1 = new System.Windows.Forms.Label();
            this.CloseButton = new System.Windows.Forms.Button();
            this.RunAllButton = new System.Windows.Forms.Button();
            this.resultSplitContainer = new System.Windows.Forms.SplitContainer();
            this.TestsListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.AssertsListView = new System.Windows.Forms.ListView();
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.closeIfNoFailedUnitTestsCheckBox = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.resultSplitContainer)).BeginInit();
            this.resultSplitContainer.Panel1.SuspendLayout();
            this.resultSplitContainer.Panel2.SuspendLayout();
            this.resultSplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(342, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Select Unit test to show unit test asserts. Double click to run a unit test.";
            // 
            // CloseButton
            // 
            this.CloseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.CloseButton.Location = new System.Drawing.Point(529, 355);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(75, 23);
            this.CloseButton.TabIndex = 1;
            this.CloseButton.Text = "Close";
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // RunAllButton
            // 
            this.RunAllButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.RunAllButton.Location = new System.Drawing.Point(448, 355);
            this.RunAllButton.Name = "RunAllButton";
            this.RunAllButton.Size = new System.Drawing.Size(75, 23);
            this.RunAllButton.TabIndex = 2;
            this.RunAllButton.Text = "Run All";
            this.RunAllButton.UseVisualStyleBackColor = true;
            this.RunAllButton.Click += new System.EventHandler(this.RunAllButton_Click);
            // 
            // resultSplitContainer
            // 
            this.resultSplitContainer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.resultSplitContainer.Location = new System.Drawing.Point(12, 29);
            this.resultSplitContainer.Name = "resultSplitContainer";
            this.resultSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // resultSplitContainer.Panel1
            // 
            this.resultSplitContainer.Panel1.Controls.Add(this.TestsListView);
            // 
            // resultSplitContainer.Panel2
            // 
            this.resultSplitContainer.Panel2.Controls.Add(this.AssertsListView);
            this.resultSplitContainer.Size = new System.Drawing.Size(592, 320);
            this.resultSplitContainer.SplitterDistance = 137;
            this.resultSplitContainer.TabIndex = 3;
            // 
            // TestsListView
            // 
            this.TestsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
            this.TestsListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TestsListView.Location = new System.Drawing.Point(0, 0);
            this.TestsListView.Name = "TestsListView";
            this.TestsListView.Size = new System.Drawing.Size(592, 137);
            this.TestsListView.TabIndex = 0;
            this.TestsListView.UseCompatibleStateImageBehavior = false;
            this.TestsListView.View = System.Windows.Forms.View.Details;
            this.TestsListView.SelectedIndexChanged += new System.EventHandler(this.TestsListView_SelectedIndexChanged);
            this.TestsListView.DoubleClick += new System.EventHandler(this.TestsListView_DoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Class";
            this.columnHeader1.Width = 331;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "File";
            this.columnHeader2.Width = 133;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Status";
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Time";
            // 
            // AssertsListView
            // 
            this.AssertsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5,
            this.columnHeader6,
            this.columnHeader7});
            this.AssertsListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.AssertsListView.Location = new System.Drawing.Point(0, 0);
            this.AssertsListView.Name = "AssertsListView";
            this.AssertsListView.Size = new System.Drawing.Size(592, 179);
            this.AssertsListView.TabIndex = 1;
            this.AssertsListView.UseCompatibleStateImageBehavior = false;
            this.AssertsListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Status";
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Line";
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Assert";
            this.columnHeader7.Width = 468;
            // 
            // closeIfNoFailedUnitTestsCheckBox
            // 
            this.closeIfNoFailedUnitTestsCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.closeIfNoFailedUnitTestsCheckBox.AutoSize = true;
            this.closeIfNoFailedUnitTestsCheckBox.Location = new System.Drawing.Point(12, 355);
            this.closeIfNoFailedUnitTestsCheckBox.Name = "closeIfNoFailedUnitTestsCheckBox";
            this.closeIfNoFailedUnitTestsCheckBox.Size = new System.Drawing.Size(174, 17);
            this.closeIfNoFailedUnitTestsCheckBox.TabIndex = 4;
            this.closeIfNoFailedUnitTestsCheckBox.Text = "Close dialog if all unit tests pass";
            this.closeIfNoFailedUnitTestsCheckBox.UseVisualStyleBackColor = true;
            // 
            // UnitTestFailure
            // 
            this.AcceptButton = this.CloseButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(616, 390);
            this.Controls.Add(this.closeIfNoFailedUnitTestsCheckBox);
            this.Controls.Add(this.resultSplitContainer);
            this.Controls.Add(this.RunAllButton);
            this.Controls.Add(this.CloseButton);
            this.Controls.Add(this.label1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "UnitTestFailure";
            this.Text = "Unit Tests";
            this.Load += new System.EventHandler(this.UnitTestFailure_Load);
            this.resultSplitContainer.Panel1.ResumeLayout(false);
            this.resultSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.resultSplitContainer)).EndInit();
            this.resultSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.Button RunAllButton;
        private System.Windows.Forms.SplitContainer resultSplitContainer;
        private System.Windows.Forms.ListView TestsListView;
        private System.Windows.Forms.ListView AssertsListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.CheckBox closeIfNoFailedUnitTestsCheckBox;
    }
}