using ApplicationUtility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Controllers;
using GeoWebCore.Services;
using Newtonsoft.Json;
using Pyxis.IO.Import;
using Pyxis.Utilities;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.CLI
{
    /// <summary>
    /// Provide CLI (command line interface) for importing Files and GeoSources
    /// </summary>
    static class ImportGeoSource
    {
        private static string SanitizeGeoSourceName(GeoSource geoSource)
        {
            var fixedName = "";
            
            foreach (var c in geoSource.Metadata.Name)
            {
                if (Char.IsLetterOrDigit(c))
                {
                    fixedName += c;
                }
                else
                {
                    if (fixedName.Last() != '_') 
                    {
                        fixedName += '_';
                    }
                }

                if (fixedName.Length > 40)
                {
                    break;
                }
            }

            return fixedName + "-" + geoSource.Id.ToString().Substring(0, 4);
        }

        public static bool ImportFromGallery(Guid geoSourceId)
        {
            var geoSource = Program.Engine.GetChannel().GeoSources.GetById(geoSourceId);

            if (geoSource == null)
            {
                Console.WriteLine("[ERROR]: couldn't find GeoSource " + geoSourceId );
                return false;
            }

            var start = DateTime.Now;
            Console.WriteLine("Process " + geoSource.Id.ToString() + " is " + geoSource.Metadata.Name);

            var process = Program.Engine.GetProcess(geoSource);

            Console.WriteLine("Process " + geoSource.Id.ToString() + " took " + (DateTime.Now - start).TotalSeconds + " [sec]");

            List<ManifestEntry> files = process.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests().SelectMany(manifest => manifest.Entries).ToList();

            foreach (var file in files)
            {
                var path = ChecksumSingleton.Checksummer.FindFileMatchingChecksum(file.FileStamp);
                if (String.IsNullOrEmpty(path))
                {
                    Console.WriteLine("file " + file.FileName + " with checksum " + file.FileStamp + " couldn't be located");
                }
                else
                {
                    Console.WriteLine("file " + file.FileName + " was found at: " + path);
                }
            }

            if (process == null || process.isNull())
            {
                Console.WriteLine("[ERROR]: failed to import GeoSource " + geoSource.Id.ToString());
                return false;
            }
            else
            {
                Console.WriteLine("[SUCCESS]: import GeoSource " + geoSource.Id.ToString() + " completed");
                return true;
            }
        }

        public static bool DownloadFromGallery(Guid geoSourceId,string root)
        {
            var geoSource = Program.Engine.GetChannel().GeoSources.GetById(geoSourceId);

            if (geoSource.Metadata.Providers == null || geoSource.Metadata.Providers.Count == 0) 
            {
                Console.WriteLine("[ERROR]: GeoSource " + geoSource.Id.ToString() + " has no gallery associated with it");
                return false;
            }

            var provider = geoSource.Metadata.Providers[0];

            Console.WriteLine("Process " + geoSource.Id.ToString() + " is " + geoSource.Metadata.Name);

            //we simple just load it and not even try to initialize it, as we only just need file refs           
            var process = PipeManager.readPipelineFromString(geoSource.Definition);

            List<ManifestEntry> files = process.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests().SelectMany(manifest => manifest.Entries).ToList();

            var fileClient = new Pyxis.Storage.FileSystemStorage.FileSystemStorage(new Pyxis.Storage.BlobProviders.PyxisBlobProvider());

            var ok = true;

            foreach (var file in files)
            {
                Console.WriteLine(file.FileName + " ( " + file.FileSize + " ) = " + file.FileStamp);

                var directory = System.IO.Path.Combine(root, provider.Id.ToString());

                if (!System.IO.Directory.Exists(directory))
                {
                    //add a nice metadata file with gallery name for us humans
                    System.IO.Directory.CreateDirectory(directory);
                    System.IO.File.WriteAllText(System.IO.Path.Combine(directory, provider.Name + ".json"), Newtonsoft.Json.JsonConvert.SerializeObject(provider));
                }

                var target = System.IO.Path.Combine(directory, "Files", SanitizeGeoSourceName(geoSource), file.RelativePath);

                if (!System.IO.File.Exists(target) || new System.IO.FileInfo(target).Length != file.FileSize)
                {
                    fileClient.DownloadFile(file.FileStamp, target);
                }

                if (!System.IO.File.Exists(target))
                {
                    ok = false;
                    Console.WriteLine("[ERROR]: failed to download file " + file.FileName); ;
                }
                else
                {
                    var checkSum = ChecksumSingleton.Checksummer.getFileCheckSum_synch(target);

                    if (checkSum != file.FileStamp)
                    {
                        ok = false;
                        Console.WriteLine("[ERROR]: checksum doesn't match " + file.FileName); ;
                    }
                }
            }

            return ok;
        }

        public static bool Discover(string url)
        {
            var Engine = Program.Engine;

            var catalog = Engine.BuildCatalog(url);

            if (catalog != null && catalog.DataSets != null && catalog.DataSets.Count > 0)
            {
                Console.WriteLine(url + " is supported");

                if (catalog.Metadata != null)
                {
                    Console.WriteLine(JsonConvert.SerializeObject(catalog.Metadata,Formatting.Indented));
                }

                foreach (var dataset in catalog.DataSets)
                {
                    //Console.WriteLine("layer " + dataset.Layer + " : " + dataset.Type + " : " + dataset.Fields.Count + " fields");
                    Console.WriteLine(JsonConvert.SerializeObject(dataset, Formatting.Indented));
                }
                return true;
            }
            return false;
        }

        public static bool DiscoverAndProcess(string url)
        {
            var Engine = Program.Engine;

            var catalog = Engine.BuildCatalog(url);

            if (catalog != null && catalog.DataSets != null && catalog.DataSets.Count > 0)
            {
                Console.WriteLine(url + " is supported");
                foreach (var dataset in catalog.DataSets)
                {
                    Console.WriteLine("layer " + dataset.Metadata.Name + " : " + dataset.Type + " : " + dataset.Fields.Count + " fields");

                    var start = DateTime.Now;
                    var geoSource = Engine.BeginImport(dataset).Task.Result;
                    var end = DateTime.Now;
                    Console.WriteLine("imported in " + (end - start).TotalSeconds + " [s]");

                    var pipeline = Engine.GetProcess(geoSource);
                    var geometry = pyxlib.QueryInterface_IFeature(pipeline.getOutput()).getGeometry();
                    var coverage = pyxlib.QueryInterface_ICoverage(pipeline.getOutput());

                    var imageContoller = new ImageController();

                    start = DateTime.Now;

                    GeoSourceInitializer.Initialize(geoSource);
                    imageContoller.RGB("01", 244, geoSource.Id);
                    end = DateTime.Now;
                    Console.WriteLine("generate 1 rhombus in " + (end - start).TotalSeconds + " [s]");

                    //if (coverage.isNotNull())
                    //{
                    //    var tileCollection = new PYXTileCollection();
                    //    geometry.copyTo(tileCollection, Math.Max(2, geometry.getCellResolution() - 11));

                    //    var iterator = tileCollection.getIterator();

                    //    while (!iterator.end())
                    //    {
                    //        Console.WriteLine(iterator.getIndex());
                    //        var result = coverage.getCoverageTile(PYXTile.create(iterator.getIndex(), geometry.getCellResolution()).get());
                    //        var stats = result.calcStatistics();

                    //        Console.WriteLine(iterator.getIndex() + " : avg=" + stats.avgValue.getString() + ", values = " + (100.0 * stats.nValues / stats.nCells) + "%");
                    //        iterator.next();
                    //    }
                    //}
                }
                return true;
            }
            return false;
        }
    }
}
