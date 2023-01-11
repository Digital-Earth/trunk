using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MultiG2TestBed
{
    public partial class FormTestBedMain : Form
    {
        private int G2ProcessNumber;

        public FormTestBedMain()
        {
            InitializeComponent();
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            String procName = "G2 Process " + G2ProcessNumber.ToString();
            G2Process newProcess = new G2Process(procName, 6700 + G2ProcessNumber);
            listBoxServents.Items.Add(newProcess);
            newProcess.Start();
            G2ProcessNumber++;
        }

        private void cLBServents_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void buttonRemove_Click(object sender, EventArgs e)
        {
            if (listBoxServents.SelectedIndex != -1)
            {
                if (listBoxServents.Items[listBoxServents.SelectedIndex] is G2Process)
                {
                    ((G2Process)(listBoxServents.Items[listBoxServents.SelectedIndex])).Stop();
                }

                listBoxServents.Items.RemoveAt(listBoxServents.SelectedIndex);
            }
        }

        private void FormTestBedMain_Load(object sender, EventArgs e)
        {
            String parentDir = System.Windows.Forms.Application.StartupPath;
            foreach (String dirName in System.IO.Directory.GetDirectories(parentDir))
            {
                if (System.IO.File.Exists(
                    System.IO.Path.Combine(dirName, "coresettings.config")))
                {
                    String procName = dirName.Substring(parentDir.Length + 1);
                    G2Process newProcess = new G2Process(procName);
                    listBoxServents.Items.Add(newProcess);
                    newProcess.Start();
                }
            }
        }
    }
}