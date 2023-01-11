#region File Header
//
//      FILE:   ProgressForm.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace Infralution.Controls
{
	/// <summary>
	/// Provides a form for display progress of a length operation.
	/// </summary>
	/// <remarks>
	/// Provides a progress bar and a cancel button to allow the user to cancel
	/// the process.
	/// </remarks>
	public class ProgressForm : System.Windows.Forms.Form
	{
        #region Designer Member Variables

        protected System.Windows.Forms.Button cancellationButton;
        protected System.Windows.Forms.ProgressBar progressBar;
        protected System.Windows.Forms.Label label;
		private System.ComponentModel.Container components = null;

        #endregion

        #region Member Variables
        
        bool _cancelled = false;
        bool _closed = false;
     
        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new instance of the form
        /// </summary>
		public ProgressForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
		}

        /// <summary>
        /// Return true if the user has clicked on the cancel button
        /// </summary>
        /// 
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool Cancelled
        {
            get 
            {
                Refresh();
                Application.DoEvents();
                return _cancelled; 
            }
        }

        /// <summary>
        /// Set/Get the message to display on the form
        /// </summary>
        [Description("The message to display on the form")]
        [Category("Appearance")]
        public string Message
        {
            get { return label.Text; }
            set { label.Text = value; }
        }

        /// <summary>
        /// The maximum value for the progress bar
        /// </summary>
        [Description("The maximum value for the progress bar")]
        [Category("Behavior")]
        public int Maximum
        {
            get { return progressBar.Maximum; }
            set { progressBar.Maximum = value; }
        }

        /// <summary>
        /// The minimum value for the progress bar
        /// </summary>
        [Description("The minimum value for the progress bar")]
        [Category("Behavior")]
        public int Minimum
        {
            get { return progressBar.Minimum; }
            set { progressBar.Minimum = value; }
        }

        /// <summary>
        /// The value (progress indicator) for the progress bar
        /// </summary>
        [Description("The value (progress indicator) for the progress bar")]
        [Category("Behavior")]
        public int Value
        {
            get	{ return progressBar.Value; }
            set	{ progressBar.Value = value; }
        }

        /// <summary>
        /// The amount to step the progress bar each time PerformStep is called
        /// </summary>
        [Description("The amount to step the progress bar each time PerformStep is called")]
        [Category("Behavior")]
        public int Step
        {
            get { return progressBar.Step; }
            set { progressBar.Step = value; }
        }

        /// <summary>
        /// Increment the progress bar by the value of Step.
        /// </summary>
        public void PerformStep()
        {
            progressBar.PerformStep();
        }

        /// <summary>
        /// Close the form once the process has completed
        /// </summary>
        public new void Close()
        {
            _closed = true;
            base.Close();
        }

        #endregion

		#region Windows Form Designer generated code
	
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if(components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose( disposing );
        }
        
        /// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ProgressForm));
            this.cancellationButton = new System.Windows.Forms.Button();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.label = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // _cancelButton
            // 
            this.cancellationButton.AccessibleDescription = null;
            this.cancellationButton.AccessibleName = null;
            resources.ApplyResources(this.cancellationButton, "_cancelButton");
            this.cancellationButton.BackgroundImage = null;
            this.cancellationButton.Font = null;
            this.cancellationButton.Name = "_cancelButton";
            this.cancellationButton.Click += new System.EventHandler(this._cancelButton_Click);
            // 
            // _progressBar
            // 
            this.progressBar.AccessibleDescription = null;
            this.progressBar.AccessibleName = null;
            resources.ApplyResources(this.progressBar, "_progressBar");
            this.progressBar.BackgroundImage = null;
            this.progressBar.Font = null;
            this.progressBar.Minimum = 1;
            this.progressBar.Name = "_progressBar";
            this.progressBar.Step = 1;
            this.progressBar.Value = 1;
            // 
            // _label
            // 
            this.label.AccessibleDescription = null;
            this.label.AccessibleName = null;
            resources.ApplyResources(this.label, "_label");
            this.label.Font = null;
            this.label.Name = "_label";
            // 
            // ProgressForm
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.BackgroundImage = null;
            this.Controls.Add(this.label);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.cancellationButton);
            this.Font = null;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ProgressForm";
            this.ResumeLayout(false);

        }
		#endregion

        #region Local Methods

        private void Cancel()
        {
            _cancelled = true;
            Cursor = Cursors.WaitCursor;
            Refresh();
            Application.DoEvents();
        }

        /// <summary>
        /// Handle user clicking on the dialog close button
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing (e);

            // check if this the user clicking on the dialog close cross
            // if so cancel and wait for the form to be closed programmatically\
            //
            if (!_closed)
            {
                Cancel();
                e.Cancel = true;
            }

        }

        private void _cancelButton_Click(object sender, System.EventArgs e)
        {
            Cancel();
        }

        #endregion

	}
}
