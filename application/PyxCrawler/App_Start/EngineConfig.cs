using System.IO;
using System.Web;
using Pyxis.Core;
using Pyxis.IO.Import;

namespace PyxCrawler
{
    public static class EngineConfig
    {
        public static Engine Engine { get; private set; }

        public static void Start()
        {
            var config = Pyxis.Core.EngineConfig.WorldViewDefault;
            config.UsePyxnet = false;
            config.ClearCache = true;
            copyPlugins(config.ApplicationDirectory);
            Engine = Engine.Create(config);

            try
            {
                Engine.Start();
                Engine.WhenReady(() =>
                {
                    Engine.EnableAllImports();
                });
            }
            catch (System.Exception e)
            {
                Engine.Stop();
            }
        }

        // Allow the plugins to be found in the same directory as Pyxis.Core.dll which gets shadow copied
        // to its own directory during ASP.NET application load in IIS
        private static void copyPlugins(string destinationDirectory)
        {
            //Now Create all of the directories
            var sourcePath = HttpRuntime.AppDomainAppPath + "bin\\plugins";
            var destinationPath = destinationDirectory + "\\plugins";
            Directory.CreateDirectory(destinationPath);
            foreach (string dirPath in Directory.GetDirectories(sourcePath, "*",
                SearchOption.AllDirectories))
                Directory.CreateDirectory(dirPath.Replace(sourcePath, destinationPath));

            //Copy all the files & Replaces any files with the same name
            foreach (string newPath in Directory.GetFiles(sourcePath, "*.*",
                SearchOption.AllDirectories))
            {
                var newDestinationPath = newPath.Replace(sourcePath, destinationPath);

                if (!File.Exists(newDestinationPath) ||
                    new FileInfo(newPath).Length != new FileInfo(newDestinationPath).Length)
                {
                    File.Copy(newPath, newDestinationPath, true);    
                }
            }
        }
    }
}