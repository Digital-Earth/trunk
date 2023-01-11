namespace ApplicationUtility.Visualization.Controls
{
    partial class HistogramControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.m_toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.SuspendLayout();
            // 
            // HistogramControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.DoubleBuffered = true;
            this.Name = "HistogramControl";
            this.Size = new System.Drawing.Size(237, 148);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.HistogramControl_Paint);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.HistogramControl_MouseClick);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.HistogramControl_MouseMove);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ToolTip m_toolTip;
    }
}
