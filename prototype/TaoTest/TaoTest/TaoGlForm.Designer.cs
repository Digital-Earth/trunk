namespace TaoTest
{
    partial class TaoGlForm
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
                m_glControl.DestroyContexts();
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
            this.m_glControl = new Tao.Platform.Windows.SimpleOpenGlControl();
            this.SuspendLayout();
            // 
            // m_glControl
            // 
            this.m_glControl.AccumBits = ((byte)(0));
            this.m_glControl.AutoCheckErrors = false;
            this.m_glControl.AutoFinish = false;
            this.m_glControl.AutoMakeCurrent = true;
            this.m_glControl.AutoSwapBuffers = true;
            this.m_glControl.BackColor = System.Drawing.Color.Black;
            this.m_glControl.ColorBits = ((byte)(32));
            this.m_glControl.DepthBits = ((byte)(16));
            this.m_glControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.m_glControl.Location = new System.Drawing.Point(0, 0);
            this.m_glControl.Name = "m_glControl";
            this.m_glControl.Size = new System.Drawing.Size(491, 337);
            this.m_glControl.StencilBits = ((byte)(0));
            this.m_glControl.TabIndex = 0;
            this.m_glControl.Paint += new System.Windows.Forms.PaintEventHandler(this.m_glControl_Paint);
            // 
            // TaoGlForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(491, 337);
            this.Controls.Add(this.m_glControl);
            this.Name = "TaoGlForm";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion

        private Tao.Platform.Windows.SimpleOpenGlControl m_glControl;
    }
}

