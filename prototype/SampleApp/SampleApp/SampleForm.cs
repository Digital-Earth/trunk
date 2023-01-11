using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace SampleApp
{
    public partial class SampleForm : Form
    {
        public SampleForm()
        {
            InitializeComponent();
        }

        /// <summary>
        /// This is method is called when the button is pressed
        /// on the form. Developers can choose to override this
        /// behavior to test and experiment with new funcitonality.
        /// </summary>
        private void button1_Click(object sender, EventArgs e)
        {
            // No function defined yet
            this.button1.Text = "Nothing To Do!";
        }
    }
}