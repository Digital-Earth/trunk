using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using Pyxis.Utilities;
using PyxNet.DataHandling;

namespace PyxNet.Publishing.Files
{
    public class PublishedFileInfo : Publisher.IPublishedItemInfo
    {

        private readonly string m_SHA256ChecksumString;

        private string m_lowerCaseDescription;

        private Dictionary<FileInformation, int> m_publishedSourceFiles;

        private object m_lockObj = new object();

        public DataInfo DataInfo { get; private set; }

        public FileInformation FileInformation
        {
            get
            {
                lock (m_lockObj)
                {
                    return m_publishedSourceFiles.Keys.FirstOrDefault();
                }
            }
        }

        public int PublishedCount
        {
            get
            {
                lock (m_lockObj)
                {
                    return m_publishedSourceFiles.Values.Sum();
                }
            }
        }

        public string SHA256Checksum
        {
            get
            {
                return m_SHA256ChecksumString;
            }
        }

        public IEnumerable<FileInformation> SourceFiles
        {
            get
            {
                return m_publishedSourceFiles.Keys.ToList();
            }
        }

        public event EventHandler KeywordsChanged;

        public PublishedFileInfo(FileInformation fileInformation)
            :this(fileInformation, 1)
        {
        }

        public PublishedFileInfo(FileInformation fileInformation, int timesPublished)
        {
            m_publishedSourceFiles = new Dictionary<FileInformation, int>();
            IncreaseSource(fileInformation, timesPublished);
            m_SHA256ChecksumString = Pyxis.Utilities.ChecksumSingleton.Checksummer.getFileCheckSum_synch(fileInformation.FullName);
            DataInfo = new DataInfo();
        }

        public PublishedFileInfo(FileInformation file, string description, DataInfo dataInfo, int timesPublished)
            : this(file, timesPublished)
        {
            m_lowerCaseDescription = description.ToLower();

            if (dataInfo != null)
            {
                DataInfo = dataInfo;
            }
        }

        public void DecreaseSource(FileInformation fileInfo)
        {
            lock (m_lockObj)
            {
                if (m_publishedSourceFiles.ContainsKey(fileInfo) &&
                    m_publishedSourceFiles[fileInfo] > 0
                    )
                {
                    m_publishedSourceFiles[fileInfo]--;
                    if (m_publishedSourceFiles[fileInfo] == 0)
                    {
                        m_publishedSourceFiles.Remove(fileInfo);
                        RaiseKeywordsChanged();
                    }
                }
            }
        }

        public int DeleteSource(FileInformation fileInfo)
        {
            bool exists = false;
            int result;
            lock (m_lockObj)
            {
                result = m_publishedSourceFiles[fileInfo];
                if (m_publishedSourceFiles.Remove(fileInfo))
                {
                    exists = true;
                }
            }
            if (exists && !m_publishedSourceFiles.Keys.Any(x => x.Name == fileInfo.Name))
            {
                RaiseKeywordsChanged();
            }
            return result;
        }
        public void IncreaseSource(FileInformation fileInfo)
        {
            IncreaseSource(fileInfo, 1);
        }
        public void IncreaseSource(FileInformation fileInfo, int timesPublished)
        {
            lock (m_lockObj)
            {
                if (m_publishedSourceFiles.ContainsKey(fileInfo))
                {
                    m_publishedSourceFiles[fileInfo] += timesPublished;
                }
                else
                {
                    m_publishedSourceFiles.Add(fileInfo, 1);
                    RaiseKeywordsChanged();
                }
            }
        }

        /// <summary>
        /// Does this PublishedFileInfo match the given string?
        /// </summary>
        /// <param name="testAgainst">The string to test against.</param>
        /// <returns></returns>
        public bool Matches(string testAgainst)
        {
            string lowerCaseTestAgainst = testAgainst.ToLower();

            if (m_SHA256ChecksumString.Equals(testAgainst))
            {
                return true;
            }

            lock (m_lockObj)
            {
                if (m_publishedSourceFiles.Keys.Any(x => x.Name.Equals(lowerCaseTestAgainst)))
                {
                    return true;
                }
            }
            return false;
        }

        private void RaiseKeywordsChanged()
        {
            System.Threading.Tasks.Task.Factory.StartNew(() => KeywordsChanged.SafeInvoke(this));
        }

        #region IPublishedItemInfo Members

        public IEnumerable<string> Keywords
        {
            get
            {
                foreach (var publishedFile in SourceFiles)
                {
                    yield return publishedFile.Name;
                }

                yield return m_SHA256ChecksumString;
            }
        }

        public QueryResult Matches(Query query, Stack stack)
        {
            if (Matches(query.Contents))
            {
                stack.Tracer.WriteLine("Query {0} matched {1}.",
                    query.Contents, FileInformation.Name);

                var connectedNodeInfo = stack.KnownHubList.ConnectedHubs.FirstOrDefault();

                if (connectedNodeInfo == null)
                {
                    stack.Tracer.WriteLine("The connected node info is null.");
                }
                else
                {
                    stack.Tracer.WriteLine("Connected node info: {0}", connectedNodeInfo.FriendlyName);
                }

                QueryResult queryResult =
                    new QueryResult(query.Guid, query.OriginNode, stack.NodeInfo, connectedNodeInfo);

                queryResult.MatchingContents = FileInformation.Name;
                queryResult.MatchingDescription = m_lowerCaseDescription;
                queryResult.MatchingDataSetID = DataInfo.DataSetID;

                queryResult.HashCodeType = 2; // TODO: Decide on hash code types.
                queryResult.HashCode = Convert.FromBase64String(SHA256Checksum);

                //--
                //-- create manifest resource fact, and search for similar
                //-- fact with a certificate
                //--
                Pyxis.Utilities.ManifestEntry manifestEntry = new Pyxis.Utilities.ManifestEntry(FileInformation.ToFileInfo(), ".");
                PyxNet.Service.ResourceInstanceFact manifestFact =
                    new PyxNet.Service.ResourceInstanceFact(manifestEntry);

                //--
                //-- Search for certificate.
                //--
                PyxNet.Service.ResourceInstanceFact certifiedFact =
                    PyxNet.Service.ResourceInstanceFact.FindCertifiedFact(
                        PyxNet.StackSingleton.Stack,
                        manifestFact);

                //--
                //-- append certificate to query result
                //--
                if (certifiedFact != null && certifiedFact.Certificate != null)
                {
                    queryResult.ExtraInfo = certifiedFact.Certificate.ToMessage();
                }

                return queryResult;
            }

            return null;
        }

        #endregion IPublishedItemInfo Members
    }
}