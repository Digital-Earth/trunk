using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Tao.OpenGl;
using Tao.Platform.Windows;
 
namespace TaoTest 
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
            Application.Run(new TaoGlForm());
        }

    }
}