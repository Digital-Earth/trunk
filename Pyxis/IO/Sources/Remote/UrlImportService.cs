using System;
using System.Collections.Specialized;
using System.IO;
using System.Net;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Sources.Local;
using File = System.IO.File;

namespace Pyxis.IO.Sources.Remote
{
    public class UrlImportService : IDataSetImportService
    {
        public IPermit Permit { get; private set; }
        public DataSet DataSet { get; private set; }

        public string LocalFileExtension { get; set; }

        public event EventHandler<string> PostProcessingLocalFile;

        public UrlImportService(DataSet dataSet, IPermit permit = null, string fileExtension = "json")
        {
            DataSet = dataSet;
            Permit = permit;
            LocalFileExtension = fileExtension;
        }

        public virtual string CreateLocalFileName(WebHeaderCollection headers = null)
        {
            var tag =  DateTime.Now.Ticks.ToString();

            if (headers != null)
            {
                var md5 = headers[HttpResponseHeader.ContentMd5];
                var etag = headers[HttpResponseHeader.ETag];
                
                if (md5.HasContent())
                {
                    tag = md5.Replace('/', '-').Replace('+', '_');
                }
                else if (etag.HasContent())
                {
                    tag = etag;
                }
            }

            string filename;
            if (DataSet.Uri.EndsWith(LocalFileExtension))
            {
                filename = DataSet.Uri.Substring(DataSet.Uri.LastIndexOf('/') + 1);
                filename = filename.Substring(0, filename.Length - LocalFileExtension.Length);
            }
            else
            {
                filename = new Uri(DataSet.Uri).Host;
            }

            //make it unique;
            filename += "." + tag + "." + LocalFileExtension.TrimStart('.');

            return filename.RemoveAllChars(Path.GetInvalidFileNameChars());
        }

        public string DownloadToLocalFile(IImportSettingProvider settingsProvider)
        {
            var downloadLocallyTask = settingsProvider.ProvideSetting(typeof(DownloadLocallySetting),
                new ProvideImportSettingArgs());

            var downloadLocally = downloadLocallyTask != null
                ? downloadLocallyTask.Result as DownloadLocallySetting
                : null;

            if (downloadLocally == null)
            {
                throw new Exception("Please provide DownloadLocallySetting to import remote urls at the moment");
            }

            var headers = GetContentHeads();

            var localFile = CreateLocalFileName(headers);

            localFile = Path.GetFullPath(Path.Combine(downloadLocally.Path, localFile));

            if (!Directory.Exists(downloadLocally.Path))
            {
                Directory.CreateDirectory(downloadLocally.Path);
            }

            Console.WriteLine(localFile);

            //download the file is not found already
            if (!File.Exists(localFile))
            {
                DownloadUrl(localFile);
            }

            return localFile;
        }

        public virtual IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            var localFile = DownloadToLocalFile(settingsProvider);

            var localService = new LocalDataSetDiscoveryService();
            var datasets = localService.GetDataSets(localFile, Permit);

            var localPipeline =
                localService.GetDataSetImportService(datasets[0], Permit).BuildPipeline(engine, settingsProvider);

            return localPipeline;
        }

        protected WebHeaderCollection GetContentHeads()
        {
            try
            {
                var webRquest = WebRequest.Create(DataSet.Uri);

                webRquest.Method = WebRequestMethods.Http.Head;

                var response = webRquest.GetResponse();

                return response.Headers;
            }
            catch (Exception)
            {
                return null;
            }
        }

        protected virtual void DownloadUrl(string localFile)
        {
            using (var webClient = new WebClient())
            {
                var networkPermit = Permit as INetworkPermit;
                if (networkPermit != null)
                {
                    webClient.Credentials = networkPermit.Credentials;
                }

                Console.WriteLine("Downloading " + DataSet.Uri);
                webClient.DownloadFile(DataSet.Uri, localFile + ".temp");
                Console.WriteLine("Download compeleted " + DataSet.Uri);

                if (File.Exists(localFile))
                {
                    File.Delete(localFile);
                }
                File.Move(localFile + ".temp", localFile);
            }

            if (PostProcessingLocalFile != null)
            {
                PostProcessingLocalFile(this, localFile);
            }
        }

       
        public virtual void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
    
        }
    }
}
