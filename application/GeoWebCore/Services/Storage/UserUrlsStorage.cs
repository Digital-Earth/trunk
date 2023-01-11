using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Security.Policy;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Search;
using File = System.IO.File;

namespace GeoWebCore.Services.Storage
{
    /// <summary>
    /// UrlDiscoeryStatus - Consider remove this class
    /// </summary>
    public class UrlDiscoveryStatus
    {
        public string Id { get; set; }
        public string Uri { get; set; }

        public string Status { get; set; }

        public int DataSetCount { get; set; }
        public int VerifiedDataSetCount { get; set; }
        public int BrokenDataSetCount { get; set; }

        public DateTime LastDiscovered { get; set; }
        public SimpleMetadata Metadata { get; set; }

        [DefaultValue(typeof(TimeSpan), "1.00:00:00")]   
        public TimeSpan RefreshRate { get; set; }

        public static UrlDiscoveryStatus FromUri(string uri)
        {
            return new UrlDiscoveryStatus()
            {
                Uri = uri,
                Id = UniqueHashGenerator.FromObject(uri),
                LastDiscovered = DateTime.UtcNow,
                DataSetCount = 0,
                VerifiedDataSetCount = 0,
                BrokenDataSetCount = 0,
                RefreshRate = TimeSpan.FromDays(7),
                Status = "Empty",
                Metadata = new SimpleMetadata()
                {
                    Name = new Uri(uri).Host,
                    Description = ""
                }
            };
        }
    }

    /// <summary>
    /// urls discovery stroage provides a presistence layer of all urls the user assoicate with the gallery
    /// </summary>
    internal class UserUrlsStorage
    {
        private const string UrlFilenameSuffix = ".url.json";
        private const string UrlDataSetsFilenameSuffix = ".datasets.json";

        public static int Segment = 0;
        public static int NumberOfSegments = 0;

        /// <summary>
        /// Get all import requests keys for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <returns>list of import requests ids</returns>
        public List<UrlDiscoveryStatus> GetAllUrls(string galleryId)
        {
            var rootDirectory = GetPath(galleryId);
            var result = new List<UrlDiscoveryStatus>();

            if (!Directory.Exists(rootDirectory))
            {
                return result;
            }

            foreach (var file in Directory.EnumerateFiles(rootDirectory, "*" + UrlFilenameSuffix))
            {
                result.Add(JsonConvert.DeserializeObject<UrlDiscoveryStatus>(File.ReadAllText(file)));
            }

            return result;
        }

        public List<DataSet> GetDataSets(string galleryId, UrlDiscoveryStatus url)
        {
            var filePath = GetDataSetPath(galleryId, url);

            if (!File.Exists(filePath))
            {
                return null;
            }

            return JsonConvert.DeserializeObject<List<DataSet>>(File.ReadAllText(filePath));
        }

        public IGazetteer CreatePublicGazetteer()
        {
            var multiSourceGazetteer = new MultiSourceGazetteer();

            multiSourceGazetteer.AddSource("public",
                Pyxis.IO.Search.Sources.LocalFolderGazetteer.CreateFromFolder(
                    Path.Combine(Program.RunInformation.GalleryFilesRoot, "Public", "Urls"), "*.json", NumberOfSegments, Segment));

            multiSourceGazetteer.AddSource("ucar",
                new Pyxis.IO.Sources.UCAR.UcarGazetteer(GeoSourceInitializer.Engine));

            return multiSourceGazetteer;   
        }

        public IGazetteer CreateUserGazetteer(string galleryId)
        {
            var gazetteer = new Gazetteer();

            var dataSets = GetAllUrls(galleryId).SelectMany(url => GetDataSets(galleryId, url)).RemoveDuplicates();

            gazetteer.AddRange(dataSets);

            return gazetteer;
        }

        private DataSet GenerateDiscoveryReportIfMissing(DataSet dataSet)
        {
            if (dataSet.Specification != null && 
                dataSet.Specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage &&
                dataSet.DiscoveryReport == null )
            {
                dataSet.DiscoveryReport = new DataSetDiscoveryReport();
            }
            return dataSet;
        }

        /// <summary>
        /// Save ImportDataSetRequest to disk with optionally a result GeoSource
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="url">url to link to gallery</param>
        public void SaveUrl(string galleryId, UrlDiscoveryStatus url, List<DataSet> foundDataSets)
        {
            //ensure directory exists
            var rootDirectory = GetPath(galleryId);
            if (!Directory.Exists(rootDirectory))
            {
                Directory.CreateDirectory(rootDirectory);
            }

            //update dataset counters
            if (foundDataSets != null)
            {
                url.DataSetCount = foundDataSets.Count;

                url.VerifiedDataSetCount =
                    foundDataSets.Count(x => x.DiscoveryReport != null && x.DiscoveryReport.Status == DataSetDiscoveryStatus.Successful);

                url.BrokenDataSetCount =
                    foundDataSets.Count(
                        x =>
                            x.DiscoveryReport != null && x.DiscoveryReport.Status == DataSetDiscoveryStatus.Failed &&
                            x.DiscoveryReport.Issues != null && x.DiscoveryReport.Issues.Count > 0);
            }

            var filePath = GetPath(galleryId, url);
            AtomicFile.WriteAllText(filePath, JsonConvert.SerializeObject(url, Formatting.Indented));

            if (foundDataSets != null)
            {
                var dataSetsPath = GetDataSetPath(galleryId, url);
                AtomicFile.WriteAllText(dataSetsPath, JsonConvert.SerializeObject(foundDataSets, Formatting.Indented));
            }
        }

        /// <summary>
        /// Delete url from gallery
        /// </summary>
        /// <param name="galleryId">galery Id</param>
        /// <param name="url">Server url to unlink from gallery</param>
        /// <returns></returns>
        public bool DeleteUrl(string galleryId, UrlDiscoveryStatus url)
        {
            var filePath = GetPath(galleryId, url);

            if (!File.Exists(filePath))
            {
                return false;
            }

            File.Delete(filePath);
            return true;
        }

        private string GetPath(string galleryId, UrlDiscoveryStatus url)
        {
            return GetPath(galleryId, UniqueHashGenerator.FromObject(url.Uri) + UrlFilenameSuffix);
        }

        private string GetDataSetPath(string galleryId, UrlDiscoveryStatus url)
        {
            return GetPath(galleryId, UniqueHashGenerator.FromObject(url.Uri) + UrlDataSetsFilenameSuffix);
        }

        public string GetPathForLocalDownload(string galleryId, string uri)
        {
            return GetPath(galleryId, UniqueHashGenerator.FromObject(uri) + Path.DirectorySeparatorChar + DateTime.UtcNow.ToString("yyyy-MM-dd-HH-mm-ss"));
        }

        /// <summary>
        /// Get the system path based on the gallery id and path. Relative paths are not allowed.
        /// </summary>
        /// <param name="galleryId">Optional gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns></returns>
        private string GetPath(string galleryId, string path = null)
        {
            // common root for gallery files
            var basePath = Program.RunInformation.GalleryFilesRoot;

            // add gallery and path if supplied
            if (!String.IsNullOrEmpty(galleryId))
            {
                basePath = Path.Combine(basePath, galleryId, "Urls");

                if (!String.IsNullOrEmpty(path))
                {
                    // strip double quotes from path
                    basePath = Path.Combine(basePath, path);
                }
            }

            // security check - make sure path is within gallery files directory
            var fullPath = Path.GetFullPath(basePath);
            if (!fullPath.StartsWith(Program.RunInformation.GalleryFilesRoot))
            {
                throw new Exception("Invalid gallery id: " + (galleryId ?? "") + " or path: " + (path ?? ""));
            }

            return fullPath;
        }
    }
}