using Pyxis.Storage.BlobProviders;
using Pyxis.Storage.FileSystemStorage;
using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

namespace PyxNet.Pyxis.GeoPackets
{
    /// <summary>
    /// This class is responsible to upload and download coverage tiles.
    /// Tiles are stored on storage server defined in ctor.
    /// Key to retrieve the tile is defined in GenerateKey method
    /// </summary>
    public class CoverageCacheClient
    {
        private Guid m_coverageDataGuid;
        private ICache_SPtr m_spCache;
        private FileSystemStorage m_fileClient;

        /// <summary>
        /// Lock object for active downloads
        /// </summary>
        private object m_activeDownloadsLock = new object();
        
        /// <summary>
        /// Tracking all active downloads, this is used to merge downloads of the same tile.
        /// </summary>
        private Dictionary<string, Task<bool>> m_activeDownloads;

        public CoverageCacheClient()
        {
            var blobClient = new PyxisBlobProvider();
            m_fileClient = new FileSystemStorage(blobClient);

            m_activeDownloads = new Dictionary<string, Task<bool>>();
        }

        public static CoverageCacheClient FromProcess(IProcess_SPtr process)
        {
            var result = new CoverageCacheClient();
            result.m_spCache = pyxlib.QueryInterface_ICache(process.getOutput());
            if (result.m_spCache.get() == null)
            {
                return null;
            };
            result.m_coverageDataGuid = new Guid(pyxlib.guidToStr(process.getProcID()));
            return result;
        }

        public Task AddTileToStorageAsync(PYXTile pYXTile)
        {
            //find file for this tile.
            string tileFileName = m_spCache.toFileName(pYXTile);
            if (!File.Exists(tileFileName))
            {
                throw new Exception("Tile file does not exist: " + tileFileName);
            }

            //find key for this tile.
            string key = GenerateKey(pYXTile);

            //upload

            var progress = m_fileClient.UploadFileWithKeyAsync(tileFileName, key);
            progress.Task.ContinueWith(t =>
            {
                if (t.IsFaulted)
                {
                    Trace.debug("Cache File Upload Failed: " + key + " " + t.Exception);
                }
            });
            return progress.Task;
        }

        public void StoreToBlob(PYXTile pYXTile)
            {
                m_spCache.forceCoverageTile(pYXTile);
            }

        public void StoreToBlobAsync(PYXTile pYXTile)
        {
            Task.Factory.StartNew(()=>
            m_spCache.forceCoverageTile(pYXTile));
        }

        public bool TileExistsOnStorage(PYXTile pYXTile)
        {
            string key = GenerateKey(pYXTile);

            return m_fileClient.FileExists(key);
        }

        public bool TryDownloadCacheFromStorage(PYXTile pYXTile)
        {
            string tileFileName = m_spCache.toFileName(pYXTile);
            if (File.Exists(tileFileName))
            {
                return true;
            }

            string key = GenerateKey(pYXTile);

            Task<bool> downloadTask;
            lock (m_activeDownloadsLock)
            {
                if (m_activeDownloads.TryGetValue(key, out downloadTask))
                {
                    //we will reuse that task as download for this tile is already in progress
                    Trace.info("merging simultaneous downloads of: " + key);
                }
                else
                {
                    m_activeDownloads[key] = downloadTask = Task<bool>.Factory.StartNew(() => m_fileClient.DownloadFile(key, tileFileName));

                    //make sure we clean up m_activeDownloads
                    downloadTask.ContinueWith((task) =>
                    {
                        lock (m_activeDownloadsLock)
                        {
                            m_activeDownloads.Remove(key);
                        }
                    });
                }
            }

            return downloadTask.Result;
        }

        private string GenerateKey(PYXTile pYXTile)
        {
            string key = "Version1:" + m_coverageDataGuid + "-Depth:" + pYXTile.getDepth() + "-Index:" + pYXTile.getRootIndex().toString();
            return key;
        }

        private string GenerateKey(CoverageRequestMessage.TransferMode mode)
        {
            string key = "Version1:" + m_coverageDataGuid + " definition file: " + mode;
            return key;
        }

        public string FilePathFromMode(CoverageRequestMessage.TransferMode mode)
        {
            return m_spCache.getCacheDir() + "/" + CoverageRequestHelper.FileName(mode);
        }

        public bool TryDownloadDefinitionsFromStorage(CoverageRequestMessage.TransferMode mode)
        {
            return m_fileClient.DownloadFile(GenerateKey(mode), FilePathFromMode(mode));
        }

        public global::Pyxis.Storage.ProgressTracker<string> UploadDefinitionFilesAsync(CoverageRequestMessage.TransferMode mode)
        {
            return m_fileClient.UploadFileWithKeyAsync(FilePathFromMode(mode), GenerateKey(mode));
        }
    }
}