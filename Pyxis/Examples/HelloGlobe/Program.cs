using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HelloGlobe
{
    static class Program
    {
        /// <summary>
        /// Store the active Pyxis.Core.Engine object
        /// </summary>
        public static Pyxis.Core.Engine PyxisEngine { get; set; }
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // We start by creating a config using ApiKey
            // This method would invoke a token request to api.pyxis.worldview.gallery to validate the email and key
            var config = Pyxis.Core.EngineConfig.FromApiKey(
                new Pyxis.Publishing.ApiKey("info@pyxisinnovation.com", "502EC10B8A6D490E9B81663B59CB8693"));  

            // Once config has been created using the ApiKey, we can create an engine
            PyxisEngine = Pyxis.Core.Engine.Create(config);

            // Call Engine.Start() to load all Pyxis Engine technology into memory
            // This function would throw an exception if something went wrong
            PyxisEngine.Start();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());

            // Once the job is done - you might want to stop the engine
            PyxisEngine.Stop();
        }
    }
}
