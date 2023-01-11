using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace LeakClarification
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            Close();
        }

        static ClassLeak1 example1;

        private void buttonExample1_Click(object sender, EventArgs e)
        {
            // this will create 1000 objects that will hang around eating memory
            // this will only leak once, as the second time the static object will get replaced
            // and the first instance of the static object will be freed, including all 1000
            // of it's referenced objects.
            example1 = new ClassLeak1();
        }

        private void buttonExample2_Click(object sender, EventArgs e)
        {
            // this will NOT create 1000 objects that will hang around eating memory
            // because the root object "example2" will go away and thus the referenced
            // objects will also go away.
            ClassLeak1 example2 = new ClassLeak1();
        }

        static ClassNoLeak1 example3;

        private void buttonExample3_Click(object sender, EventArgs e)
        {
            // this will NOT create 1000 objects that will hang around eating memory
            // even though object example3 will be around.
            example3 = new ClassNoLeak1();
        }

        static ClassLeak2 example4;

        private void buttonExample4_Click(object sender, EventArgs e)
        {
            // this will create 1000 objects that will hang around eating memory

            // only leaks once per app run.
            example4 = new ClassLeak2();
        }

        static ClassNoLeak2 example5;

        private void buttonExample5_Click(object sender, EventArgs e)
        {
            // this will NOT create 1000 objects that will hang around eating memory
            example5 = new ClassNoLeak2();
        }

        static List<ClassLeak1> example6 = new List<ClassLeak1>();
        private void buttonExample6_Click(object sender, EventArgs e)
        {
            // this will create 1000 objects that will hang around eating memory

            // this will leak 1000 objects every time this function is called because we 
            // keep the root objects around in a list.
            example6.Add( new ClassLeak1());
        }

    }
}
