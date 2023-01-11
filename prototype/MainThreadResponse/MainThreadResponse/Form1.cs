using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MainThreadResponse
{
    public partial class Form1 : Form
    {
        int updateCount = 0;

        public void EatUpTenSeconds()
        {
            DateTime startTime = DateTime.Now;
            bool endThisInsanity = false;
            while (!endThisInsanity)
            {
                TimeSpan timePassed = DateTime.Now - startTime;
                endThisInsanity = (timePassed.Seconds >= 10);
            }
        }

        public void StartBusyThread()
        {
            System.Threading.Thread aThread = new System.Threading.Thread(EatUpTenSeconds);
            aThread.Start();
        }

        public Form1()
        {
            InitializeComponent();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            ++updateCount;
            label1.Text = "Update count = " + updateCount.ToString();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            EatUpTenSeconds();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            StartBusyThread();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            int count;
            for (count = 0; count < 10; ++count)
            {
                StartBusyThread();
            }
        }
    }
}
