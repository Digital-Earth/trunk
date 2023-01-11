using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Utilities;

namespace PyxisCLI.Operations.Old
{
    class FilesOperation : IOperationMode
    {
        class Options
        {
            public bool Register { get; set; }
            public bool Remove { get; set; }
            public bool Download { get; set; }
        }

        public string Command
        {
            get { return "old files"; }
        }

        public string Description
        {
            get { return "show/download raw files for a GeoSource"; }
        }

        public void Run(string[] args)
        {
            var options = new Options();

            args = ArgsParser.Parse(args, 
                new ArgsParser.Option("d|download", (value) => options.Download = true),
                new ArgsParser.Option("r|register", (value) => options.Register = true),
                new ArgsParser.Option("del|delete", (value) => options.Remove = true));

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-d|download] [-r|register] [-del|delete] geosource|dir|file", Command);
                return;
            }

            foreach (var id in args.Skip(1))
            {
                Guid guid;

                if (Guid.TryParse(id.Replace(".json",""), out guid))
                {
                    var geoSource =
                        JsonConvert.DeserializeObject<GeoSource>(
                            System.IO.File.ReadAllText(String.Format("{0}.json", guid)));

                    if (geoSource != null)
                    {
                        ListFiles(Program.Engine,geoSource, options);
                    }
                }
                else if (System.IO.Directory.Exists(id))
                {
                    foreach (var file in Directory.EnumerateFiles(id))
                    {
                        RegisterLocalFileChecksum(file);
                    }
                }
                else if (System.IO.File.Exists(id))
                {
                    RegisterLocalFileChecksum(id);
                }

            }
        }

        private List<ManifestEntry> EnumerateFilesForGeoSource(Engine engine, GeoSource geoSource)
        {
            var process = PipeManager.readPipelineFromString(geoSource.Definition);

            var files = process.WalkPipelines().ExtractManifests().SelectMany(manifest => manifest.Entries).ToList();

            return files;
        }

        private void ListFiles(Engine engine, GeoSource geoSource, Options options)
        {
            foreach (var manifestEntry in EnumerateFilesForGeoSource(engine, geoSource))
            {
                if (options.Download)
                {
                    DownloadFile(geoSource, manifestEntry);
                }

                ShowFileStatus(manifestEntry);
            }
        }

        private void DownloadFile(GeoSource geoSource, ManifestEntry entry)
        {
            var path = ChecksumSingleton.Checksummer.FindFileMatchingChecksum(entry.FileStamp);

            if (path.HasContent())
            {
                //file allready been downloaded
                return;
            }

            var fileClient = new Pyxis.Storage.FileSystemStorage.FileSystemStorage(new Pyxis.Storage.BlobProviders.PyxisBlobProvider());

            var directory = geoSource.Id.ToString();

            if (!System.IO.Directory.Exists(directory))
            {
                System.IO.Directory.CreateDirectory(directory);
            }

            var target = directory + Path.DirectorySeparatorChar + entry.FileName;

            ShowFileStatus(entry, "0%", ConsoleColor.Yellow);
            Console.SetCursorPosition(0, Console.CursorTop - 1);

            var downloadTask = fileClient.DownloadFileAsync(entry.FileStamp, target);

            downloadTask.ProgressMade += (tracker) =>
            {
                ShowFileStatus(entry,tracker.Percent + "%",ConsoleColor.Yellow);
                Console.SetCursorPosition(0,Console.CursorTop - 1);
            };

            if (!downloadTask.Task.Result)
            {
                ShowFileStatus(entry, "Failed to download", ConsoleColor.Red);
            }
            else
            {
                var checkSum = ChecksumSingleton.Checksummer.getFileCheckSum_synch(target);

                if (checkSum != entry.FileStamp)
                {
                    ShowFileStatus(entry, "Failed to download", ConsoleColor.Red);
                }
            }
        }

        private void RegisterLocalFileChecksum(string path)
        {
            var checksum = ChecksumSingleton.Checksummer.getFileCheckSum_synch(path);

            ShowFileStatus(path);
        }

        private void ShowFileStatus(string path)
        {
            var checksum = ChecksumSingleton.Checksummer.getFileCheckSum_synch(path);

            ShowFileStatus(new ManifestEntry(new FileInfo(path), Path.GetDirectoryName(path)));
        }

        private void ShowFileStatus(ManifestEntry entry, string status, ConsoleColor statusColor)
        {
            Console.Write("{0} - {1} - {2} bytes", entry.FileStamp, entry.FileName, entry.FileSize);
            if (status.HasContent())
            {
                var oldColor = Console.ForegroundColor;
                Console.Write(" - ");
                Console.ForegroundColor = statusColor;
                Console.Write(status);
                Console.ForegroundColor = oldColor;
            }

            Console.WriteLine();                
        }

        private void ShowFileStatus(ManifestEntry entry)
        {
            var path = ChecksumSingleton.Checksummer.FindFileMatchingChecksum(entry.FileStamp);
            if (path.HasContent())
            {
                ShowFileStatus(entry, "local", ConsoleColor.Green);
            }
            else
            {
                ShowFileStatus(entry, null, ConsoleColor.White);
            }
        }
    }
}
