using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GeoWebCoreRunner
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            RunnerConfiguration configuration;
            var configurationFileName = "nodes.json";

            if (File.Exists(configurationFileName))
            {
                try
                {
                    configuration = JsonConvert.DeserializeObject<RunnerConfiguration>(File.ReadAllText(configurationFileName));
                }
                catch (Exception e)
                {
                    MessageBox.Show("Failed To parse configuration file: " + e.Message, "Bad Configuration");
                    return;
                }
            }
            else
            {
                configuration = new RunnerConfiguration("http://localhost", 4000, 4020);
                File.WriteAllText(configurationFileName,JsonConvert.SerializeObject(configuration, Formatting.Indented));
            }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm() { Configuration = configuration });
        }
    }
}
