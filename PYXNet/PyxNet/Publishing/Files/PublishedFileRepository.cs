using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Timers;
using Pyxis.Utilities;
using PyxNet.DataHandling;

namespace PyxNet.Publishing.Files
{
    public class PublishedFileRepository : Publisher.IDynamicPublishedItem
    {
        private static TimeSpan s_invalidationInterval = TimeSpan.FromMinutes(10);
        private static TimeSpan s_keywordsChangedRaiseDelay = TimeSpan.FromSeconds(5);
        private Dictionary<string, PublishedFileInfo> m_checkSumLUT = new Dictionary<string, PublishedFileInfo>();
        private Dictionary<DataGuid, PublishedFileInfo> m_datasetLUT = new Dictionary<DataGuid, PublishedFileInfo>();
        private Timer m_invalidateTimer;
        private Timer m_keywordChangedTimer;
        private Object m_lockObj = new Object();

        public IEnumerable<PublishedFileInfo> PublishedFileInfos
        {
            get
            {
                lock (m_lockObj)
                {
                    return m_checkSumLUT.Values.ToList();
                }
            }
        }

        public PublishedFileRepository()
        {
            Keywords = new List<string>();
            m_keywordChangedTimer = new Timer(s_keywordsChangedRaiseDelay.TotalMilliseconds);
            m_keywordChangedTimer.AutoReset = false;
            m_keywordChangedTimer.Elapsed += delegate(object sender, System.Timers.ElapsedEventArgs e)
            {
                lock (m_lockObj)
                {
                    Keywords = PublishedFileInfos.SelectMany(x => x.Keywords).Distinct().ToList();
                }
                System.Threading.Tasks.Task.Factory.StartNew(() => KeywordsChanged.SafeInvoke(this));
            };

            m_invalidateTimer = new Timer(s_invalidationInterval.TotalMilliseconds);
            m_invalidateTimer.AutoReset = false;
            m_invalidateTimer.Elapsed += delegate(object sender, System.Timers.ElapsedEventArgs e)
            {
                try
                {
                    InvalidatePublishedFiles();
                }
                catch (Exception ex)
                {
                    Logging.Categories.Publishing.Error(ex);
                }
                finally
                {
                    m_invalidateTimer.Enabled = true;
                }
            };
            m_invalidateTimer.Enabled = true;
        }

        public void Clear()
        {
            bool keywordsChanged = false;
            lock (m_lockObj)
            {
                if (m_checkSumLUT.Keys.Count > 0)
                {
                    keywordsChanged = true;
                }
                m_checkSumLUT.Clear();
                m_datasetLUT.Clear();
                Keywords = new List<string>();
            }
            if (keywordsChanged)
            {
                RaiseKeywordsChanged();
            }
        }

        public int Delete(FileInformation fileInfo)
        {
            int result = -1;
            lock (m_lockObj)
            {
                var pfi = GetPFIByPath(fileInfo);
                if (pfi != null)
                {
                    result = pfi.DeleteSource(fileInfo);
                    if (pfi.PublishedCount == 0)
                    {
                        m_datasetLUT.Remove(pfi.DataInfo.DataSetID);
                        m_checkSumLUT.Remove(pfi.SHA256Checksum);
                    }
                }
            }
            return result;
        }

        public PublishedFileInfo GetPFIByDataSetID(DataGuid datasetID)
        {
            lock (m_lockObj)
            {
                PublishedFileInfo pfi;
                m_datasetLUT.TryGetValue(datasetID, out pfi);
                return pfi;
            }
        }

        public DataInfo Publish(FileInformation fileInfo)
        {
            return Publish(fileInfo, "", null);
        }

        public DataInfo Publish(FileInformation fileInfo, int timesPublisehd)
        {
            return Publish(fileInfo, "", null, timesPublisehd);
        }
        public DataInfo Publish(FileInformation file, string description, DataInfo dataInfo)
        {
            return Publish(file, description, dataInfo, 1);
        }
        public DataInfo Publish(FileInformation file, string description, DataInfo dataInfo, int timesPublished)
        {
            lock (m_lockObj)
            {
                var toPublish = new PublishedFileInfo(file, description, dataInfo, timesPublished);
                var existingChecksumPFI = GetPFIByCheckSum(toPublish.SHA256Checksum);
                var existingPathPFI = GetPFIByPath(toPublish.FileInformation);
                if (existingPathPFI != existingChecksumPFI)
                {
                    Invalidate(toPublish.FileInformation);
                }

                if (existingChecksumPFI != null)
                {
                    existingChecksumPFI.IncreaseSource(toPublish.FileInformation, timesPublished);
                    return existingChecksumPFI.DataInfo;
                }
                else
                {
                    toPublish.KeywordsChanged += delegate(object sender, EventArgs e)
                    {
                        RaiseKeywordsChanged();
                    };
                    m_datasetLUT.Add(toPublish.DataInfo.DataSetID, toPublish);
                    m_checkSumLUT.Add(toPublish.SHA256Checksum, toPublish);
                    RaiseKeywordsChanged();
                    return toPublish.DataInfo;
                }
            }
        }

        public bool Unpublish(FileInformation fileInfo)
        {
            var pfi = new PublishedFileInfo(fileInfo);
            var existingChecksumPFI = GetPFIByCheckSum(pfi.SHA256Checksum);
            var existingPathPFI = GetPFIByPath(pfi.FileInformation);

            if (existingPathPFI != existingChecksumPFI)
            {
                Invalidate(pfi.FileInformation);
            }

            if (existingChecksumPFI != null)
            {
                existingChecksumPFI.DecreaseSource(pfi.FileInformation);
                if (existingChecksumPFI.PublishedCount == 0)
                {
                    m_datasetLUT.Remove(pfi.DataInfo.DataSetID);
                    m_checkSumLUT.Remove(pfi.SHA256Checksum);
                }
                return true;
            }
            return false;
        }

        private PublishedFileInfo GetPFIByCheckSum(string checkSum)
        {
            lock (m_lockObj)
            {
                PublishedFileInfo pfi;
                m_checkSumLUT.TryGetValue(checkSum, out pfi);
                return pfi;
            }
        }

        private IEnumerable<PublishedFileInfo> GetPFIByFileName(string fileName)
        {
            fileName = fileName.ToLower();

            lock (m_lockObj)
            {
                return PublishedFileInfos.Where(x => x.SourceFiles.Any(y => y.Name == fileName)).ToList();
            }
        }

        private PublishedFileInfo GetPFIByPath(FileInformation path)
        {
            lock (m_lockObj)
            {
                return GetPFIByFileName(path.Name).FirstOrDefault(x => x.SourceFiles.Any(y => y.FullName == path.FullName));
            }
        }

        private void InvalidatePublishedFiles()
        {
            foreach (var pfi in PublishedFileInfos)
            {
                foreach (var fileInfo in pfi.SourceFiles)
                {
                    Invalidate(fileInfo);
                }
            }
        }

        public void Invalidate(FileInformation fileInfo)
        {
            lock (m_lockObj)
            {
                if (!File.Exists(fileInfo.FullName))
                {
                    Delete(fileInfo);
                    return;
                }

                if (fileInfo.IsModified())
                {
                    int timesPublished = Delete(fileInfo);
                    Publish(fileInfo, timesPublished);
                }
            }
        }

        private void RaiseKeywordsChanged()
        {
            if (m_keywordChangedTimer.Enabled)
            {
                return;
            }
            else
            {
                m_keywordChangedTimer.Enabled = true;
            }
        }

        #region IDynamicPublishedItem Members

        public IEnumerable<string> Keywords { get; private set; }

        public event EventHandler KeywordsChanged;

        public QueryResult Matches(Query query, Stack stack)
        {
            var pfi = GetPFIByCheckSum(query.Contents);

            if (pfi == null)
            {
                pfi = GetPFIByFileName(query.Contents).FirstOrDefault();
            }
            if (pfi != null)
            {
                return pfi.Matches(query, stack);
            }
            return null;
        }

        #endregion IDynamicPublishedItem Members
    }
}