using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace PyxNetReportsApplication
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            PyxNetReports report;
            do
            {
                // Note: This won't change anything about the StackSingleton.
                report = new PyxNetReports();
                Application.Run(report);
            } while (report.DialogResult == DialogResult.Retry);
        }
    }
}