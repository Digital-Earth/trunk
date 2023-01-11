using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;
using PyxNet;
using PyxNet.DataHandling;

namespace PyxNet.Pyxis
{
    /// <summary>
    /// A class for querying for data accross PyxNet and then timing a sample download
    /// from each data source, and for combinations of data sources.
    /// </summary>
    public class CoverageDownloadPerformance
    {
        #region Tracer

        private readonly NumberedTraceTool<CoverageDownloadPerformance> m_tracer
            = new NumberedTraceTool<CoverageDownloadPerformance>(TraceTool.GlobalTraceLogEnabled);

        public NumberedTraceTool<CoverageDownloadPerformance> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        private String m_queryString;

        private Stack m_stack;

        private string m_TileRootIndex;

        /// <summary>
        /// Gets or sets the index of the root tile.
        /// </summary>
        /// <value>The index of the root tile.</value>
        public string TileRootIndex
        {
            get { return m_TileRootIndex; }
            set { m_TileRootIndex = value; }
        }

        private int m_tileDepth = 11;

        /// <summary>
        /// Gets or sets the tile depth used when requesting data.
        /// </summary>
        /// <value>The tile depth.</value>
        public int TileDepth
        {
            get { return m_tileDepth; }
            set { m_tileDepth = value; }
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="CoverageDownloadPerformance"/> class.
        /// </summary>
        /// <param name="query">The query.</param>
        public CoverageDownloadPerformance(string query, Stack stack)
        {
            m_queryString = query;
            m_stack = stack;
        }

        /// <summary>
        /// A helper class which describes the results of test downloading from a PyxNet coverage data source. 
        /// </summary>
        public class ResultEntry
        {
            private string m_operation;

            /// <summary>
            /// Gets or sets the operation.
            /// </summary>
            /// <value>The operation.</value>
            public string Operation
            {
                get { return m_operation; }
                set { m_operation = value; }
            }

            private string m_description;

            /// <summary>
            /// Gets or sets the description.
            /// </summary>
            /// <value>The description.</value>
            public string Description
            {
                get { return m_description; }
                set { m_description = value; }
            }

            private string m_dataSetID;

            public string DataSetID
            {
                get { return m_dataSetID; }
                set { m_dataSetID = value; }
            }

            private int m_resolution;

            /// <summary>
            /// Gets or sets the resolution.
            /// </summary>
            /// <value>The resolution.</value>
            public int Resolution
            {
                get { return m_resolution; }
                set { m_resolution = value; }
            }

            private string m_tileRootIndex;

            /// <summary>
            /// Gets or sets the index of the tile root.
            /// </summary>
            /// <value>The index of the tile root.</value>
            public string TileRootIndex
            {
                get { return m_tileRootIndex; }
                set { m_tileRootIndex = value; }
            }

            private string m_status;

            /// <summary>
            /// Gets or sets the status.
            /// </summary>
            /// <value>The status.</value>
            public string Status
            {
                get { return m_status; }
                set { m_status = value; }
            }

            internal List<TimeSpan> m_duration = new List<TimeSpan>();

            /// <summary>
            /// Gets the first duration.
            /// </summary>
            public TimeSpan Duration1
            {
                get
                {
                    if (m_duration.Count >= 1)
                    {
                        return m_duration[0];
                    }
                    return TimeSpan.Zero;
                }
            }

            /// <summary>
            /// Gets the second duration.
            /// </summary>
            public TimeSpan Duration2
            {
                get
                {
                    if (m_duration.Count >= 2)
                    {
                        return m_duration[1];
                    }
                    return TimeSpan.Zero;
                }
            }

            /// <summary>
            /// Gets the third duration.
            /// </summary>
            public TimeSpan Duration3
            {
                get
                {
                    if (m_duration.Count >= 3)
                    {
                        return m_duration[2];
                    }
                    return TimeSpan.Zero;
                }
            }

            private long m_tileBytes = 0;

            /// <summary>
            /// Gets or sets the tile bytes.
            /// </summary>
            /// <value>The tile bytes.</value>
            public long TileBytes
            {
                get { return m_tileBytes; }
                set { m_tileBytes = value; }
            }

            private string m_transferRate;

            /// <summary>
            /// Gets or sets the transfer rate.
            /// </summary>
            /// <value>The transfer rate.</value>
            public string TransferRate
            {
                get { return m_transferRate; }
                set { m_transferRate = value; }
            }

        }

        private readonly List<ResultEntry> m_results = new List<ResultEntry>();

        private readonly object m_listLock = new object();

        /// <summary>
        /// Gets the results.
        /// </summary>
        public List<ResultEntry> Results
        {
            get 
            {
                lock (m_listLock)
                {
                    List<ResultEntry> listCopy = new List<ResultEntry>(m_results);
                    return listCopy;
                }
            }
        }

        /// <summary>
        /// Add an entry.
        /// </summary>
        /// <param name="entry">The entry.</param>
        public void AddEntry(ResultEntry entry)
        {
            lock (m_listLock)
            {
                m_results.Add(entry);
            }

            OnResultAdded(entry);
        }

        #region ResultAdded Event
        /// <summary> EventArgs for a ResultAdded event. </summary>    
        public class ResultAddedEventArgs : EventArgs
        {
            private ResultEntry m_ResultEntry;

            /// <summary>The ResultEntry.</summary>
            public ResultEntry ResultEntry
            {
                get { return m_ResultEntry; }
                set { m_ResultEntry = value; }
            }

            internal ResultAddedEventArgs(ResultEntry theResultEntry)
            {
                m_ResultEntry = theResultEntry;
            }
        }

        /// <summary> Event handler for ResultAdded. </summary>
        public event EventHandler<ResultAddedEventArgs> ResultAdded
        {
            add
            {
                m_ResultAdded.Add(value);
            }
            remove
            {
                m_ResultAdded.Remove(value);
            }
        }
        private EventHelper<ResultAddedEventArgs> m_ResultAdded = new EventHelper<ResultAddedEventArgs>();

        /// <summary>
        /// Raises the ResultAdded event.
        /// </summary>
        /// <param name="theResultEntry"></param>
        public void OnResultAdded( ResultEntry theResultEntry)
        {
            m_ResultAdded.Invoke( this, new ResultAddedEventArgs(theResultEntry));
        }
        #endregion ResultAdded Event

	
        /// <summary>
        /// Clears the list.
        /// </summary>
        public void ClearList()
        {
            lock (m_listLock)
            {
                m_results.Clear();
            }
        }

        /// <summary>
        /// Builds the message.
        /// </summary>
        /// <param name="coverageRequest">The coverage request.</param>
        /// <returns>A CoverageRequestMessage which will request a tile in the geometry.</returns>
        private CoverageRequestMessage BuildMessage(CoverageRequestMessage coverageRequest)
        {
            CoverageRequestMessage extraInfo = new CoverageRequestMessage();
            extraInfo.Mode = CoverageRequestMessage.TransferMode.Tile;
            extraInfo.ProcessVersion = coverageRequest.ProcessVersion;
            extraInfo.CacheTileDepth = TileDepth;
            extraInfo.UsageCertificate = coverageRequest.UsageCertificate;
            bool CalculateIndex = true;
            try
            {
                PYXIcosIndex requestIndex = new PYXIcosIndex(TileRootIndex);
                PYXTile_SPtr requestTile = PYXTile.create(requestIndex, extraInfo.CacheTileDepth + requestIndex.getResolution());
                if (coverageRequest.Geometry.intersects(requestTile.get()))
                {
                    extraInfo.TileIndex = requestIndex.toString();
                    CalculateIndex = false;
                }
            }
            catch
            {
            }
            if (CalculateIndex)
            {
                PYXGeometry_SPtr geoClone = coverageRequest.Geometry.clone();
                int rootResolution = coverageRequest.CellResolution - extraInfo.CacheTileDepth;
                if (rootResolution < 2)
                {
                    rootResolution = 2;
                }
                geoClone.setCellResolution(rootResolution);
                PYXIterator_SPtr spIt = geoClone.getIterator();
                if (!spIt.end())
                {
                    extraInfo.TileIndex = spIt.getIndex().toString();
                }
            }
            return extraInfo;
        }

        /// <summary>
        /// Times the download.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <param name="extraInfo">The extra info.</param>
        /// <param name="providers">The providers.</param>
        /// <param name="re">The ResultEntry.</param>
        /// <returns>The elapsed time for the download.</returns>
        private TimeSpan TimeDownload(DataGuid id, CoverageRequestMessage extraInfo, List<NodeInfo> providers, ResultEntry re)
        {
            DateTime startTime = DateTime.Now;
            TimeSpan duration;
            if (re.Duration1.TotalSeconds >= 30.0)
            {
                re.Status += "-";
                duration = TimeSpan.Zero;
            }
            else
            {
                Tracer.DebugWriteLine("Starting download {0}", re.Operation);
                DataDownloader downloader = new DataDownloader(id, m_stack, providers, extraInfo.ToMessage());

                //--
                //-- register event handler to log received data
                //--
                downloader.OnDataReceived += new EventHandler<DataDownloader.DownloadDataReceivedEventArgs>(OnDataReceived);

                downloader.HoldTime = TimeSpan.FromSeconds(30);
                System.IO.FileInfo tempFile = new System.IO.FileInfo(System.IO.Path.GetTempFileName());
                if (downloader.DownloadFile(tempFile, 30))
                {
                    re.Status += "S";
                    re.TileBytes += new System.IO.FileInfo( tempFile.FullName).Length;
                }
                else
                {
                    re.Status += "F";
                }
                duration = DateTime.Now - startTime;

                // Delete the temp file ignoring any errors.
                try
                {
                    tempFile.Delete();
                }
                catch
                {
                }
            }
            return duration;
        }

        /// <summary>
        /// Called when coverage data downloaded from pyxnet.
        /// Event contains information about the chunk and the sender that should be audited.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs"/> instance containing the event data.</param>
        void OnDataReceived(object sender, DataDownloader.DownloadDataReceivedEventArgs e)
        {
           UsageReports.CoverageBytesDownloaded( e.Sender.RemoteNodeInfo.FriendlyName,
                                                  e.Sender.RemoteNodeInfo.NodeGUID,
                                                  m_stack.NodeInfo.FriendlyName,
                                                  m_stack.NodeInfo.NodeGUID,
                                                  e.Chunk.DataSetID,
                                                  e.Chunk.ChunkSize
                                                  );
      
        }

        /// <summary>
        /// Generates the results.
        /// </summary>
        public void GenerateResults()
        {
            ClearList();

            if (m_queryString.Length == 0)
            {
                return;
            }

            QueryResultList results = new QueryResultList(m_stack, m_queryString);
            // Big delay here as we want to see all the providers up front.  
            results.WaitForResults(100, 10);
            if (results.Count == 0)
            {
                ResultEntry re = new ResultEntry();
                re.Operation = "No search results found.";
                AddEntry(re);
                return;
            }

            // Download from each individually.
            int index = 0;
            while (index < results.Count)
            {
                ResultEntry re1 = new ResultEntry();
                re1.Description = results[index].MatchingDescription;
                CoverageRequestMessage coverageRequest;
                try
                {
                    coverageRequest = new CoverageRequestMessage(results[index].ExtraInfo);
                    re1.DataSetID = results[index].MatchingDataSetID.ToString();
                    re1.Resolution = coverageRequest.CellResolution;
                    if (coverageRequest.CellResolution < 2)
                    {
                        re1.Operation = "Bad resolution reported by " + results[index].ResultNode.ToString();
                        re1.Status = "Bad";
                        AddEntry(re1);
                        results.RemoveAt(index);
                    }
                    else
                    {
                        CoverageRequestMessage extraInfo = BuildMessage(coverageRequest);
                        re1.TileRootIndex = extraInfo.TileIndex;
                        re1.Operation = "Single download from " + results[index].ResultNode.ToString();

                        List<NodeInfo> providers = new List<NodeInfo>();
                        providers.Add(results[index].ResultNode);

                        for (int downloadTry = 0; downloadTry < 3; ++downloadTry)
                        {
                            re1.m_duration.Add(TimeDownload(results[index].MatchingDataSetID, extraInfo, providers, re1));
                        }
                        CalculateTransferRate(re1);
                        AddEntry(re1);
                        ++index;
                    }
                }
                catch (Exception ex)
                {
                    re1.Operation = string.Format( "Not a coverage. {0}", ex.ToString());
                    re1.Status = "Unknown";
                    AddEntry(re1);
                    results.RemoveAt(index);
                }
            }

            // Download from pairs.
            index = 0;
            while (index + 1 < results.Count)
            {
                try
                {
                    CoverageRequestMessage coverageRequest;
                    coverageRequest = new CoverageRequestMessage(results[index].ExtraInfo);
                    for (int secondIndex = index + 1; secondIndex < results.Count; ++secondIndex)
                    {
                        if (results[index].MatchingDataSetID.Equals(results[secondIndex].MatchingDataSetID))
                        {
                            ResultEntry re2 = new ResultEntry();
                            re2.Description = results[index].MatchingDescription;
                            CoverageRequestMessage extraInfo = BuildMessage(coverageRequest);
                            re2.TileRootIndex = extraInfo.TileIndex;
                            re2.Operation = "Double download from " + results[index].ResultNode.ToString()
                                + " and " + results[secondIndex].ResultNode.ToString();
                            re2.DataSetID = results[index].MatchingDataSetID.ToString();
                            re2.Resolution = coverageRequest.CellResolution;

                            List<NodeInfo> providers = new List<NodeInfo>();
                            providers.Add(results[index].ResultNode);
                            providers.Add(results[secondIndex].ResultNode);

                            for (int downloadTry = 0; downloadTry < 3; ++downloadTry)
                            {
                                re2.m_duration.Add(TimeDownload(results[index].MatchingDataSetID, extraInfo, providers, re2));
                            }
                            CalculateTransferRate(re2);
                            AddEntry(re2);

                            for (int thirdIndex = secondIndex + 1; thirdIndex < results.Count; ++thirdIndex)
                            {
                                if (results[index].MatchingDataSetID.Equals(results[thirdIndex].MatchingDataSetID))
                                {
                                    ResultEntry re3 = new ResultEntry();
                                    re3.Description = results[index].MatchingDescription;
                                    re3.TileRootIndex = extraInfo.TileIndex;
                                    re3.Operation = "Triple download from " + results[index].ResultNode.ToString()
                                        + " , " + results[secondIndex].ResultNode.ToString()
                                        + " and " + results[thirdIndex].ResultNode.ToString();
                                    re3.DataSetID = results[index].MatchingDataSetID.ToString();
                                    re3.Resolution = coverageRequest.CellResolution;

                                    providers.Add(results[thirdIndex].ResultNode);

                                    for (int downloadTry = 0; downloadTry < 3; ++downloadTry)
                                    {
                                        re3.m_duration.Add(TimeDownload(results[index].MatchingDataSetID, extraInfo, providers, re3));
                                    }
                                    CalculateTransferRate(re3);
                                    AddEntry(re3);
                                }
                            }
                        }
                    }
                }
                catch (Exception)
                {
                }
                ++index;
            }
        }

        /// <summary>
        /// Calculates the transfer rate.
        /// </summary>
        /// <param name="re">The re.</param>
        private void CalculateTransferRate(ResultEntry re)
        {
            double totalDuration = re.Duration1.TotalSeconds +
                re.Duration2.TotalSeconds + re.Duration3.TotalSeconds;
            if (totalDuration > 0.0)
            {
                double transferRate = (re.TileBytes / 1024) / totalDuration;
                re.TransferRate = String.Format("{0}K bytes / second", (int)transferRate);
            }
        }
    }
}
