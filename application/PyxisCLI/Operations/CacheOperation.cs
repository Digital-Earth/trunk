using System;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class CacheOperation : IOperationMode
    {
        public string Command
        {
            get { return "cache"; }
        }

        public string Description
        {
            get { return "display local cache folder for a geoSource"; }
        }

        public void Run(string[] args)
        {
            var clear = false;
            var size = false;

            args = ArgsParser.Parse(args, new ArgsParser.Option("d|del", (value) => clear = true));
            args = ArgsParser.Parse(args, new ArgsParser.Option("s|size", (value) => size = true));


            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-d|del] geosource", Command);
                return;
            }

            foreach (var id in args.Skip(1))
            {
                var reference = new Reference(id);

                if (Program.Workspaces.WorkspaceExists(reference.Workspace))
                {
                    var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                    ShowCacheDirectories(geoSource, clear, size);
                }
                else
                {
                    Guid guid;

                    if (Guid.TryParse(id.Replace(".json", ""), out guid))
                    {
                        var geoSource =
                            JsonConvert.DeserializeObject<GeoSource>(
                                System.IO.File.ReadAllText(String.Format("{0}.json", guid)));

                        if (geoSource != null)
                        {
                            ShowCacheDirectories(geoSource, clear, size);
                        }
                    }    
                }
            }
        }

        private void ShowCacheDirectories(GeoSource geoSource, bool clear, bool size)
        {
            var process = Program.Engine.GetProcess(geoSource);

            foreach (var source in process.ImmediateGeoPacketSources())
            {
                var cache = pyxlib.QueryInterface_ICache(pyxlib.QueryInterface_PYXCOM_IUnknown(source));
                var dir = cache.getCacheDir();
                if (Directory.Exists(dir))
                {
                    if (clear)
                    {
                        Console.WriteLine("deleting " + dir + " ...");
                        System.IO.Directory.Delete(dir, true);
                    }
                    else if (size)
                    {
                        Console.WriteLine("{0}: {1:N} MB",dir, DirSize(new DirectoryInfo(dir)) / 1024.0 / 1024);
                    }
                    else
                    {
                        Console.WriteLine(dir);
                    }
                }
                
            }
        }

        private static long DirSize(DirectoryInfo d)
        {
            long size = 0;
            // Add file sizes.
            FileInfo[] fis = d.GetFiles();
            foreach (FileInfo fi in fis)
            {
                size += fi.Length;
            }
            // Add subdirectory sizes.
            DirectoryInfo[] dis = d.GetDirectories();
            foreach (DirectoryInfo di in dis)
            {
                size += DirSize(di);
            }
            return size;
        }
    }
}
