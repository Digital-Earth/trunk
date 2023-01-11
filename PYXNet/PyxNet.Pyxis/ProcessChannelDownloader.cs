using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using PyxNet.DataHandling;
using Pyxis.Utilities;

namespace PyxNet.Pyxis
{
    public class ProcessChannelDownloader : IProcessChannelKeyProvider
    {
        // A TraceTool that keeps a log of where we got all our pieces from.
        private NumberedTraceTool<CoverageDownloader> m_tracer;

        internal const int SecondsToHoldConnections = 20;

        static int s_WaitForPublisherSeconds = 45;

        static int s_RequeryTimeWhenResultsFoundInSeconds = 180;

        #region Properties and Members

        /// <summary>
        /// The stack that we are using to communicate.
        /// </summary>
        private readonly Stack m_stack;

        public ProcessChannelIdentifier ChannelIdentifier;

        /// <summary>
        /// The search string that we will use to find this coverage.
        /// </summary>
        private readonly String m_queryString;

        private CertificateRetainer m_certificateRetainer;

        /// <summary>
        /// Storage for the coverage dataset guid, which is the same as the process ID.
        /// </summary>
        private readonly DataGuid m_processDataGuid = new DataGuid();

        /// <summary>
        /// The version of this process.
        /// </summary>
        private int m_procVersion;

        /// <summary>
        /// A list of PyxNet nodes that are publishing this data.
        /// </summary>
        private readonly List<NodeInfo> m_publishers = new List<NodeInfo>();

        /// <summary>
        /// Indicate when the m_publishers need to be recreated
        /// </summary>
        private bool m_publishersNeedRefersh = true;

        /// <summary>
        /// A list of PyxNet nodes that are been return from the current query
        /// </summary>
        private List<NodeInfo> m_newPublishers = new List<NodeInfo>();

        /// <summary>
        /// A list of PyxNet nodes that are publishing that was returend from the last query
        /// </summary>
        private List<NodeInfo> m_oldPublishers = new List<NodeInfo>();

        /// <summary>
        /// A list of publishers that is populated from the QueryResultsList as
        /// it is accessed.
        /// </summary>
        public List<NodeInfo> Publishers
        {
            get
            {
                lock (m_publishers)
                {
                    while (m_newPublishers.Count < m_queryResults.Count)
                    {
                        m_newPublishers.Add(m_queryResults[m_newPublishers.Count].ResultNode);
                        m_publishersNeedRefersh = true;
                    }

                    if (m_publishersNeedRefersh)
                    {
                        m_publishers.Clear();
                        m_publishers.AddRange(m_oldPublishers);
                        foreach (var item in m_newPublishers)
                        {
                            if (!m_publishers.Contains(item))
                            {
                                m_publishers.Add(item);
                            }
                        }
                        m_publishersNeedRefersh = false;
                    }
                }
                return m_publishers;
            }
        }

        /// <summary>
        /// Thread safe access to the count of the publishers.
        /// </summary>
        private int PublisherCount
        {
            get
            {
                int returnValue;
                lock (m_publishers)
                {
                    returnValue = Publishers.Count;
                }
                return returnValue;
            }
        }

        private bool m_firstTimeCallFoundRemotly = true;

        private object m_queryResultsLock = new object();

        /// <summary>
        /// The PyxNet query agent that is finding places to get this coverage from.
        /// </summary>
        private readonly QueryResultList m_queryResults = null;

        /// <summary>
        /// The time that we started the last query.
        /// </summary>
        private DateTime m_queryStartTime;

        #endregion       

        #region Service

        private static readonly Guid ProcessChannelDataReaderServiceId = new Guid(
            "{F1AC6EBD-9C32-4fc3-BBCB-093ABBD20F9F}");

        private PyxNet.Service.ServiceId GetServiceId()
        {
            PyxNet.Service.ServiceId result = new PyxNet.Service.ServiceId(
                ProcessChannelDataReaderServiceId, this.m_processDataGuid.Guid);
            return result;
        }

        #endregion

        /// <summary>
        /// Constructor - create the downloader and get ready to download. Will throw an ArgumentException
        /// if the supplied IProcess_SPtr is not a process that implements a cache.
        /// </summary>
        /// <param name="stack">The stack that we will use to communicate with PyxNet.</param>
        /// <param name="channelIdentifier">The cache that we will be populating with data.</param>
        public ProcessChannelDownloader(Stack stack, ProcessChannelIdentifier channelIdentifier)
        {
            m_stack = stack;
            ChannelIdentifier = channelIdentifier;

            m_tracer = new NumberedTraceTool<CoverageDownloader>(stack.Tracer.Enabled);
            //Uncomment the next line for debugging.
            //m_tracer.Enabled = true;

            m_queryString = pyxlib.guidToStr(channelIdentifier.ProcRef.getProcID());
            m_procVersion = channelIdentifier.ProcRef.getProcVersion();

            m_processDataGuid.Guid = new Guid(pyxlib.guidToStr(channelIdentifier.ProcRef.getProcID()));

            m_queryResults = new QueryResultList(m_stack, m_queryString);
            m_queryResults.AddingElement += HandleAddingResult;

            m_certificateRetainer = new CertificateRetainer(stack, channelIdentifier.ProcRef.ToString());

            // keep track of all downloaders
            ProcessChannelDownloaderManager.Manager.Add(this);
        }

        private PyxNet.Service.Certificate GetCertificate()
        {
            return m_certificateRetainer.GetCertificate();
        }
        private object locker = new object();

        internal class KeyRequest : ProcessChannelKeyRequest
        {
            public DateTime RequestedTime { get; set; }
            public DateTime FoundedTime { get; set; }
            public ProcessChannelMultiKeyInfoMessage.KeyInfo KeyInfo { get; set; }
            public Dictionary<NodeInfo, bool> FoundAt { get; set; }
            public DataInfo DataInfo { get; set; }

            protected List<DownloadChunkKeyRequest> ChunksRequests { get; set; }

            public MemoryStream DataStream { get; private set; }
            public int DataWriten { get; set; }

            public bool Found
            {
                get
                {
                    if (KeyInfo == null)
                        return false;

                    return KeyInfo.Found;
                }
            }

            public KeyRequest(string key) : base(key)
            {
                RequestedTime = DateTime.Now;
                FoundAt = new Dictionary<NodeInfo, bool>();
                DataWriten = 0;
                DataStream = null;
            }

            public void AddFoundResult(DataInfo dataInfo, NodeInfo provider, ProcessChannelMultiKeyInfoMessage.KeyInfo keyInfo)
            {
                if (keyInfo.Found)
                {
                    if (KeyInfo == null)
                    {
                        KeyInfo = new ProcessChannelMultiKeyInfoMessage.KeyInfo(keyInfo.Key);
                    }

                    if (!KeyInfo.Found)
                    {
                        KeyInfo.Found = true;
                        KeyInfo.Length = keyInfo.Length;
                        DataInfo = dataInfo;
                    }
                    else
                    {
                        if (keyInfo.Length != KeyInfo.Length)
                        {
                            throw new Exception("Got different length for the same key");
                        }
                    }
                }
                FoundAt[provider] = keyInfo.Found;
            }

            public void WriteChunk(DataChunk dataChunk)
            {
                if (DataStream == null)
                {
                    DataStream = new MemoryStream();
                }
                dataChunk.WriteFileChunk(DataStream);
                DataWriten += dataChunk.ChunkSize;

                if (DataWriten == KeyInfo.Length)
                {
                    CompleteWithValue(DataStream.GetBuffer());
                }
            }

            public void SetData(byte[] data)
            {
                DataStream = new MemoryStream(data,0,data.Length,false,true);
                DataWriten = data.Length;
                CompleteWithValue(data);
            }

            public List<DownloadChunkKeyRequest> GenerateDownloadRequests()
            {
                if (!Found)
                {
                    throw new Exception("Can't generate chunk requests for a key that not been found");
                }

                if (ChunksRequests == null)
                {
                    var chunksCount = (int) Math.Ceiling(((double) KeyInfo.Length)/DataInfo.DataChunkSize);

                    ChunksRequests = new List<DownloadChunkKeyRequest>();

                    for (int i = 0; i < chunksCount; ++i)
                    {
                        ChunksRequests.Add(new DownloadChunkKeyRequest(this, i));
                    }
                }

                return ChunksRequests;
            }

            public bool FoundForPublisher(PublisherState publisher)
            {
                return FoundAt.ContainsKey(publisher.NodeInfo) && FoundAt[publisher.NodeInfo];
            }
        }

        internal class DownloadChunkKeyRequest
        {
            public KeyRequest Request { get; private set; }
            public int Offset { get; private set; }
            public int ChunkSize { get; private set; }

            public int DownloadRetryCount { get; set; }

            public DownloadChunkKeyRequest(KeyRequest request, int chunkNumber)
            {
                Request = request;
                Offset = chunkNumber*request.DataInfo.DataChunkSize;
                ChunkSize = Math.Min(request.DataInfo.DataChunkSize, request.KeyInfo.Length - Offset);
                DownloadRetryCount = 0;
            }
        }    

        internal abstract class PendingRequest
        {
            public DateTime SentTime { get; set; }
            public PublisherState Publisher { get; set; }

            public abstract Message GenerateMessage();
            public abstract void Cancel();

            protected PendingRequest(PublisherState publisher)
            {
                SentTime = DateTime.Now;
                Publisher = publisher;
            }
        }

        internal class SearchKeysPendingRequest : PendingRequest
        {
            public Dictionary<string,KeyRequest> RequestedKeys;

            public SearchKeysPendingRequest(PublisherState publisher, IEnumerable<KeyRequest> requestedKeys)
                : base(publisher)
            {
                RequestedKeys = new Dictionary<string, KeyRequest>();
                foreach(var request in requestedKeys)
                {
                    RequestedKeys[request.Key] = request;
                }
            }

            public override Message GenerateMessage()
            {
                var keysInfoRequest = new ProcessChannelMultiKeyRequestMessage
                                          {
                                              ProcessVersion = Publisher.Context.m_procVersion,
                                              DataCode = Publisher.Context.ChannelIdentifier.DataCode,
                                              RequestedKeys = new List<string>()
                                          };

                foreach(var request in RequestedKeys)
                {
                    keysInfoRequest.RequestedKeys.Add(request.Key);
                    request.Value.RequestedTime = DateTime.Now;
                }

                var messsage = new DataInfoRequest(Publisher.Context.m_processDataGuid, keysInfoRequest.ToMessage());

                return messsage.ToMessage();
            }

            public override void Cancel()
            {
                Publisher.AddPendingSearches(RequestedKeys.Values);
            }

            public void MergeResults(DataInfo info, ProcessChannelMultiKeyInfoMessage searchResults)
            {
                foreach (var keyInfo in searchResults.Keys)
                {
                    KeyRequest request;
                    if (RequestedKeys.TryGetValue(keyInfo.Key, out request))
                    {
                        if (keyInfo.Found)
                        {
                            Publisher.Context.m_tracer.WriteLine("found key " + keyInfo.Key + " at publisher " + Publisher.NodeInfo.FriendlyName);
                        }

                        request.AddFoundResult(info, Publisher.NodeInfo, keyInfo);
                        RequestedKeys.Remove(request.Key);

                        if (request.FoundForPublisher(Publisher))
                        {
                            Publisher.AddPendingDownload(request);
                        }
                        else
                        {
                            Publisher.Context.TrySearchAgain(request);
                        }
                    }
                }
            }
        }        

        internal class DownloadKeysPendingRequest : PendingRequest
        {
            public List<DownloadChunkKeyRequest> RequestedChunks;

            public DownloadKeysPendingRequest(PublisherState publisher, IEnumerable<DownloadChunkKeyRequest> requestedKeys)
                : base(publisher)
            {
                RequestedChunks = new List<DownloadChunkKeyRequest>(requestedKeys);
            }

            public override Message GenerateMessage()
            {
                if (RequestedChunks.Count==1)
                {
                    var request = RequestedChunks[0];

                    var message = new DataChunkRequest(Publisher.Context.m_processDataGuid,
                                                       request.Offset,
                                                       request.ChunkSize,
                                                       request.Request.DataInfo.UseEncryption,
                                                       request.Request.DataInfo.UseSigning,
                                                       Publisher.Context.GetCertificate())
                                      {
                                          ExtraInfo = new ProcessChannelRequestMessage
                                                          {
                                                              DataCode = Publisher.Context.ChannelIdentifier.DataCode,
                                                              ProcessVersion = Publisher.Context.m_procVersion,
                                                              RequestedKey = request.Request.Key
                                                          }.ToMessage()
                                      };


                    return message.ToMessage();
                }

                var extraInfo = new ProcessChannelMultiKeyRequestMessage
                                    {
                                        DataCode = Publisher.Context.ChannelIdentifier.DataCode,
                                        ProcessVersion = Publisher.Context.m_procVersion,
                                        RequestedKeys = new List<string>()
                                    };

                int totalLength = 0;
                foreach(var request in RequestedChunks)
                {
                    if (request.Offset != 0)
                    {
                        throw new Exception("Offset != 0 is not supported in a multikey request");
                    }

                    extraInfo.RequestedKeys.Add(request.Request.Key);
                    totalLength += request.ChunkSize;
                }

                var multiMessage = new DataChunkRequest(Publisher.Context.m_processDataGuid,
                                                        0,
                                                        totalLength,
                                                        RequestedChunks[0].Request.DataInfo.UseEncryption,
                                                        RequestedChunks[0].Request.DataInfo.UseSigning,
                                                        Publisher.Context.GetCertificate())
                                       {ExtraInfo = extraInfo.ToMessage()};


                return multiMessage.ToMessage();
            }

            public override void Cancel()
            {
                var retryList = new List<DownloadChunkKeyRequest>();

                foreach(var chunk in RequestedChunks)
                {
                    chunk.DownloadRetryCount++;
                    if (chunk.DownloadRetryCount<2)
                    {
                        retryList.Add(chunk);
                    }
                    else
                    {
                        Publisher.Context.TrySearchAgain(chunk.Request);
                    }
                }

                Publisher.AddPendingDownloads(retryList);
            }

            public DownloadChunkKeyRequest FindRequest(string key)
            {
                foreach(var chunk in RequestedChunks)
                {
                    if (chunk.Request.Key == key)
                    {
                        return chunk;
                    }
                }
                return null;
            }

            public void MergeResults(DataChunk dataChunk)
            {
                if (dataChunk.ExtraInfo.Identifier == ProcessChannelRequestMessage.MessageID)
                {
                    var extraInfo = new ProcessChannelRequestMessage(dataChunk.ExtraInfo);

                    if (RequestedChunks[0].ChunkSize == dataChunk.ChunkSize && 
                        RequestedChunks[0].Offset == dataChunk.Offset && 
                        extraInfo.RequestedKey == RequestedChunks[0].Request.Key)
                    {
                        Publisher.Context.m_tracer.WriteLine("downloaded chunk for " + RequestedChunks[0].Request.Key + " from publisher " + Publisher.NodeInfo.FriendlyName);
                        RequestedChunks[0].Request.WriteChunk(dataChunk);
                        RequestedChunks.RemoveAt(0);
                    }
                }
                else if (dataChunk.ExtraInfo.Identifier == ProcessChannelMultiKeyRequestMessage.MessageID)
                {
                    var extraInfo = new ProcessChannelMultiKeyRequestMessage(dataChunk.ExtraInfo);
                    var readOffset = 0;
                    var completedChunks = new List<DownloadChunkKeyRequest>();
                    foreach(var key in extraInfo.RequestedKeys)
                    {
                        var chunk = FindRequest(key);

                        if (chunk == null)
                        {
                            break;
                        }

                        if (chunk.Offset != 0)
                        {
                            throw new Exception("Non zero offset is not supported");
                        }

                        Publisher.Context.m_tracer.WriteLine("downloaded chunk for " + chunk.Request.Key + " from publisher " + Publisher.NodeInfo.FriendlyName);

                        var buffer = new byte[chunk.ChunkSize];
                        Buffer.BlockCopy(dataChunk.Data.Data, readOffset, buffer, 0, chunk.ChunkSize);
                        chunk.Request.SetData(buffer);
                        readOffset += chunk.ChunkSize;

                        completedChunks.Add(chunk);
                    }

                    foreach(var chunk in completedChunks)
                    {
                        RequestedChunks.Remove(chunk);
                    }
                }
            }
        }

        internal class PublisherState
        {
            public ProcessChannelDownloader Context { get; private set; }

            readonly List<KeyRequest> m_pendingSearchRequests = new List<KeyRequest>();
            readonly List<DownloadChunkKeyRequest> m_pendingDownloadRequests = new List<DownloadChunkKeyRequest>();

            readonly List<SearchKeysPendingRequest> m_searchRequests = new List<SearchKeysPendingRequest>();
            readonly List<DownloadKeysPendingRequest> m_downloadRequests = new List<DownloadKeysPendingRequest>(); 

            const int MaxParralelSearchRequests = 5;
            const int MaxParralelDownloadRequests = 5;

            public bool AvailableForSearching
            {
                get { return m_searchRequests.Count < MaxParralelSearchRequests; }
            }

            public bool AvailableForDownloading
            {
                get { return m_downloadRequests.Count < MaxParralelDownloadRequests; }
            }

            public NodeInfo NodeInfo { get; set; }
            public StackConnection Connection { get; set; }

            public int Load
            {
                get { return m_pendingDownloadRequests.Count + m_pendingSearchRequests.Count;  }                
            }

            public PublisherState(ProcessChannelDownloader context, NodeInfo node)
            {
                Context = context;
                NodeInfo = node;
                Connection = context.m_stack.GetConnection(NodeInfo, false, TimeSpan.FromSeconds(20));

                if (Connection != null)
                {
                    Context.m_tracer.WriteLine("Create a connection to publisher " + NodeInfo.FriendlyName);
                }

                AverageRoundTrip = MaxRoundTrip;
                CurrentTimeout = MaxRoundTrip;
            }

            #region RoundTrip calculations

            public static readonly TimeSpan MaxRoundTrip = TimeSpan.FromSeconds(30);
            public static readonly TimeSpan MinRoundTrip = TimeSpan.FromSeconds(5);

            public List<TimeSpan> LastRoundTrips = new List<TimeSpan>();

            public TimeSpan AverageRoundTrip { get; set; }

            public TimeSpan CurrentTimeout { get; set; }

            public void AddRoundTrip(TimeSpan roundTrip)
            {
                LastRoundTrips.Add(roundTrip);
                if (LastRoundTrips.Count > 10)
                {
                    LastRoundTrips.RemoveAt(0);
                }
                if (LastRoundTrips.Count > 3)
                {
                    long seconds = 0;
                    foreach (var trip in LastRoundTrips)
                    {
                        seconds += trip.Ticks;
                    }

                    AverageRoundTrip = new TimeSpan(seconds / LastRoundTrips.Count);

                    CurrentTimeout = new TimeSpan(AverageRoundTrip.Ticks*2);

                    //clamp timeout...
                    if (CurrentTimeout < MinRoundTrip)
                    {
                        CurrentTimeout = MinRoundTrip;
                    }
                    else if (CurrentTimeout > MaxRoundTrip)
                    {
                        CurrentTimeout = MaxRoundTrip;
                    }
                }
            }

            #endregion


            public void Search(IEnumerable<KeyRequest> requests)
            {
                m_pendingSearchRequests.AddRange(requests);

                SendNextSearchRequest();
            }


            public void Search(KeyRequest request)
            {
                m_pendingSearchRequests.Add(request);

                SendNextSearchRequest();
            }

            public void AddPendingSearches(IEnumerable<KeyRequest> requests)
            {
                m_pendingSearchRequests.AddRange(requests);
            }

            public void AddPendingSearch(KeyRequest request)
            {
                m_pendingSearchRequests.Add(request);
            }

            private void SendNextSearchRequest()
            {
                if (m_pendingSearchRequests.Count > 0 && AvailableForSearching)
                {
                    var requestChunk = new List<KeyRequest>();
                    int count = Math.Min(m_pendingSearchRequests.Count, 10);
                    for (int i = 0; i < count; ++i)
                    {
                        requestChunk.Add(m_pendingSearchRequests[i]);
                    }
                    var searchRequest = new SearchKeysPendingRequest(this, requestChunk);

                    if (Connection.SendMessage(searchRequest.GenerateMessage()))
                    {
                        Context.m_tracer.WriteLine("Sent search request to publisher " + NodeInfo.FriendlyName + " for " + requestChunk.Count + " keys");
                        m_searchRequests.Add(searchRequest);
                        m_pendingSearchRequests.RemoveRange(0, count);
                    }
                    else
                    {
                        Context.m_tracer.WriteLine("Failed to sent search request to publisher " + NodeInfo.FriendlyName + " for " + requestChunk.Count + " keys");
                    }
                }
            }

            public void HandleSearchResult(DataInfo info, ProcessChannelMultiKeyInfoMessage searchResults)
            {
                Context.m_tracer.WriteLine("Handing search result from publisher " + NodeInfo.FriendlyName);
                var doneRequests = new List<SearchKeysPendingRequest>();

                foreach(var pendingRequest in m_searchRequests)
                {
                    pendingRequest.MergeResults(info,searchResults);

                    if (pendingRequest.RequestedKeys.Count == 0)
                    {
                        doneRequests.Add(pendingRequest);
                        AddRoundTrip(DateTime.Now - pendingRequest.SentTime);
                    }
                }

                foreach(var doneRequest in doneRequests)
                {
                    m_searchRequests.Remove(doneRequest);
                }

                SendNextSearchRequest();
                SendNextDownloadRequest();
            }

            public void AddPendingDownloads(IEnumerable<DownloadChunkKeyRequest> requests)
            {
                m_pendingDownloadRequests.AddRange(requests);
            }

            public void AddPendingDownload(KeyRequest request)
            {
                m_pendingDownloadRequests.AddRange(request.GenerateDownloadRequests());
            }

            public void Download(DownloadChunkKeyRequest request)
            {
                m_pendingDownloadRequests.Add(request);

                SendNextDownloadRequest();
            }

            public void Download(IEnumerable<DownloadChunkKeyRequest> requests)
            {
                m_pendingDownloadRequests.AddRange(requests);

                SendNextDownloadRequest();
            }

            private void SendNextDownloadRequest()
            {
                if (m_pendingDownloadRequests.Count > 0 && AvailableForDownloading)
                {
                    int totalSize = 0;
                    var requests = new List<DownloadChunkKeyRequest>();
                    var completedRequests = new List<DownloadChunkKeyRequest>();

                    foreach(var request in m_pendingDownloadRequests)
                    {
                        if (request.Request.Completed)
                        {
                            completedRequests.Add(request);
                            continue;
                        }
                        if (request.Offset != 0)
                        {
                            //request of a key with offset != 0 is not supported in a multikey request right now...
                            if (requests.Count==0)
                            {
                                //make a request of a single key
                                requests.Add(request);
                                break;
                            }
                        }
                        else if (request.ChunkSize + totalSize <= request.Request.DataInfo.DataChunkSize)
                        {
                            requests.Add(request);
                            totalSize += request.ChunkSize;
                        }
                        //else
                        //{
                        //    break;
                        //}
                    }

                    //send request
                    if (requests.Count > 0)
                    {
                        var downloadRequest = new DownloadKeysPendingRequest(this, requests);

                        if (Connection.SendMessage(downloadRequest.GenerateMessage()))
                        {
                            Context.m_tracer.WriteLine("Sent download request to publisher " + NodeInfo.FriendlyName +
                                                       " for " + requests.Count + " chunks");
                            m_downloadRequests.Add(downloadRequest);
                            foreach (var request in requests)
                            {
                                m_pendingDownloadRequests.Remove(request);
                            }
                        }
                        else
                        {
                            Context.m_tracer.WriteLine("Failed to send download request to publisher " +
                                                       NodeInfo.FriendlyName + " for " + requests.Count + " chunks");
                        }
                    }

                    //remove all already completed request from list
                    if (completedRequests.Count > 0)
                    {
                        foreach (var request in completedRequests)
                        {
                            m_pendingDownloadRequests.Remove(request);
                        }
                    }
                }
            }

            public void HandleDownloadResult(DataChunk dataChunk)
            {
                var doneRequests = new List<DownloadKeysPendingRequest>();

                foreach (var pendingRequest in m_downloadRequests)
                {
                    pendingRequest.MergeResults(dataChunk);

                    if (pendingRequest.RequestedChunks.Count == 0)
                    {
                        doneRequests.Add(pendingRequest);
                    }
                }

                foreach (var doneRequest in doneRequests)
                {
                    m_downloadRequests.Remove(doneRequest);
                }

                SendNextDownloadRequest();
            }

            public void Tick()
            {
                //cancel all timed out search requests
                var searchRequestsToRemove = new List<SearchKeysPendingRequest>();
                foreach(var searchRequest in m_searchRequests)
                {
                    if (DateTime.Now - searchRequest.SentTime > CurrentTimeout)
                    {
                        searchRequest.Cancel();
                        searchRequestsToRemove.Add(searchRequest);
                    }
                }
                foreach (var searchRequest in searchRequestsToRemove)
                {
                    m_searchRequests.Remove(searchRequest);
                }

                //cancel all timed out download requests
                var downloadRequestsToRemove = new List<DownloadKeysPendingRequest>();
                foreach (var downloadRequest in m_downloadRequests)
                {
                    if (DateTime.Now - downloadRequest.SentTime > CurrentTimeout)
                    {
                        downloadRequest.Cancel();
                        downloadRequestsToRemove.Add(downloadRequest);
                    }
                }
                foreach (var downloadRequest in downloadRequestsToRemove)
                {
                    m_downloadRequests.Remove(downloadRequest);
                }

                SendNextDownloadRequest();
                SendNextSearchRequest();
            }
        }

        private Dictionary<Guid,PublisherState> PublishersInfo = new Dictionary<Guid,PublisherState>();

        private PublisherState GetPublisherInfo(NodeInfo remoteNode)
        {
            PublisherState info = null;
            if (PublishersInfo.TryGetValue(remoteNode.NodeGUID,out info))
            {
                return info;
            }

            info = new PublisherState(this,remoteNode);

            if (info.Connection == null)
            {
                return null;
            }

            PublishersInfo[info.NodeInfo.NodeGUID] = info;

            return info;
        }


        private Dictionary<string, KeyRequest> m_allRequests = new Dictionary<string, KeyRequest>();

        public bool FindRemotePublishers()
        {
            lock (locker)
            {
                InitializePublishersList();
            }

            if (PublisherCount == 0)
            {
                WaitForPublishers();
            }

            return PublisherCount > 0;
        }

        public ProcessChannelKeyRequest GetKeyAsync(string key)
        {
            KeyRequest request;

            lock(locker)
            {
                InitializePublishersList();
            }

            if (PublisherCount == 0)
            {
                WaitForPublishers();
            }

            lock (locker)
            {
                if (PublisherCount == 0)
                {
                    request = new KeyRequest(key);
                    request.CompleteWithError(new Exception("Can't find dataset over PyxNet"));
                }

                if (m_allRequests.TryGetValue(key, out request))
                {
                    return request;
                }

                request = new KeyRequest(key);
                request.OnRequestedCompleted(OnRequestCompleted);
                m_allRequests[key] = request;

                //check if this is the first request
                if (m_allRequests.Count == 1)
                {
                    StartStreaming();
                }

                var provider = FindLeastLoadedPublisher();

                if (provider != null)
                {
                    provider.Search(request);
                }
                else
                {
                    request.CompleteWithError(new Exception("Failed to connect to dataset over"));
                }
            }

            return request;
        }

        private PublisherState FindLeastLoadedPublisher()
        {
            PublisherState selectedPublisher = null;
            foreach(var publisher in PublishersInfo.Values)
            {
                if (selectedPublisher == null || publisher.Load < selectedPublisher.Load)
                {                
                    selectedPublisher = publisher;
                }
            }
            return selectedPublisher;
        }

        private void OnRequestCompleted(ProcessChannelKeyRequest request)
        {
            lock(locker)
            {
                m_allRequests.Remove(request.Key);
            }
        }

        /// <summary>
        /// This event is watching the cache that is supplying data, and when there is
        /// data missing, attempts to download it from PyxNet.
        /// </summary>
        /// <param name="spEvent">The event details.</param>
        public byte[] GetKey(string key)
        {
            var request = GetKeyAsync(key);

            return request.Value;
        }

        private System.Threading.Timer m_streamingTimer = null;

        private void StartStreaming()
        {
            lock(locker)
            {
                if (m_streamingTimer == null)
                {
                    m_streamingTimer = new Timer(StreamingTick,null,TimeSpan.Zero,TimeSpan.FromSeconds(1));

                    m_stack.RegisterHandler(DataChunk.MessageID, HandleChunk);
                    m_stack.RegisterHandler(DataInfo.MessageID, HandleInfo);
                }

                List<NodeInfo> publishers;
                lock(m_publishers)
                {
                    //fetch the current publisher list
                    publishers = new List<NodeInfo>(Publishers);
                }

                //try to create connection for all publishers
                foreach (var publisher in publishers)
                {
                    GetPublisherInfo(publisher);
                }
            }
        }

        private void StopStreaming()
        {
            lock(locker)
            {
                if (m_streamingTimer!=null)
                {
                    m_streamingTimer.Dispose();
                    m_streamingTimer = null;

                    //clear requests
                    m_allRequests.Clear();

                    //cleare pending messages list and publishers info
                    PublishersInfo.Clear();

                    //unregister from the stack...
                    m_stack.UnregisterHandler(DataChunk.MessageID, HandleChunk);
                    m_stack.UnregisterHandler(DataInfo.MessageID, HandleInfo);
                }
            }
        }

        private void StreamingTick(object state)
        {
            lock(locker)
            {
                InitializePublishersList();

                foreach (var publisher in PublishersInfo.Values)
                {
                    publisher.Tick();
                }

                if (m_allRequests.Count == 0)
                {
                    StopStreaming();
                }
            }
        }

        private void TrySearchAgain(KeyRequest request)
        {
            lock(locker)
            {
                foreach (var publisher in PublishersInfo.Values)
                {
                    if (!request.FoundAt.ContainsKey(publisher.NodeInfo))
                    {
                        publisher.AddPendingSearch(request);
                        return;
                    }
                }
                request.CompleteWithError(new Exception("Couldn't find key over pyxnet"));
            }
        }

        private void HandleChunk(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            var receivedInfo = new DataChunk(args.Message);

            // look and see if this is about our data set.
            if (receivedInfo.DataSetID != m_processDataGuid)
            {
                return;
            }

            if (receivedInfo.ExtraInfo.Identifier == ProcessChannelRequestMessage.MessageID)
            {
                var keysInfo = new ProcessChannelRequestMessage(receivedInfo.ExtraInfo);

                //validate this is the right version
                if (keysInfo.ProcessVersion != m_procVersion)
                {
                    return;
                }

                //validate this is the right channel datacode
                if (keysInfo.DataCode != ChannelIdentifier.DataCode)
                {
                    return;
                }
            }
            else if (receivedInfo.ExtraInfo.Identifier == ProcessChannelMultiKeyRequestMessage.MessageID)
            {
                var keysInfo = new ProcessChannelMultiKeyRequestMessage(receivedInfo.ExtraInfo);

                //validate this is the right version
                if (keysInfo.ProcessVersion != m_procVersion)
                {
                    return;
                }

                //validate this is the right channel datacode
                if (keysInfo.DataCode != ChannelIdentifier.DataCode)
                {
                    return;
                }
            }
            else
            {
                return;
            }

            lock(locker)
            {
                PublisherState publisher;

                if (PublishersInfo.TryGetValue(args.Context.Sender.RemoteNodeInfo.NodeGUID,out publisher))
                {
                    publisher.HandleDownloadResult(receivedInfo);
                }
            }
        }

        private void HandleInfo(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            DataInfo receivedInfo = new DataInfo(args.Message);

            // look and see if this is about our data set.
            if (receivedInfo.DataSetID != m_processDataGuid)
            {
                return;
            }

            //validate this is the right dataset
            if (receivedInfo.ExtraInfo.Identifier != ProcessChannelMultiKeyInfoMessage.MessageID)
            {
                return;
            }

            var keysInfo = new ProcessChannelMultiKeyInfoMessage(receivedInfo.ExtraInfo);

            //validate this is the right version
            if (keysInfo.ProcessVersion != m_procVersion)
            {
                return;
            }

            //validate this is the right channel datacode
            if (keysInfo.DataCode != ChannelIdentifier.DataCode)
            {
                return;
            }

            lock (locker)
            {
                PublisherState publisher;

                if (PublishersInfo.TryGetValue(args.Context.Sender.RemoteNodeInfo.NodeGUID, out publisher))
                {
                    publisher.HandleSearchResult(receivedInfo,keysInfo);
                }
            }
        }

        /// <summary>
        /// Called when coverage data downloaded from pyxnet.
        /// Event contains information about the chunk and the sender that should be audited.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs"/> instance containing the event data.</param>
        void OnDataReceived(object sender, DataDownloader.DownloadDataReceivedEventArgs e)
        {
            //TODO: change the name from coverage to something else?
            UsageReports.CoverageBytesDownloaded(e.Sender.RemoteNodeInfo.FriendlyName,
                                                 e.Sender.RemoteNodeInfo.NodeGUID,
                                                 m_stack.NodeInfo.FriendlyName,
                                                 m_stack.NodeInfo.NodeGUID,
                                                 e.Chunk.DataSetID,
                                                 e.Chunk.ChunkSize
                );

        }

        /// <summary>
        /// This handler is used to make sure that the result being added to the list is one that we want.
        /// We leave the state of the AllowAddition flag alone unless we want to deny addition, so that we
        /// won't override someone else's denial.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleAddingResult(object sender, QueryResultList.AddingElementEventArgs args)
        {
            QueryResult qr = args.PotentialElement;

            if (qr.ResultNode == m_stack.NodeInfo)
            {
                //we don't add the local node as query result for a remote chanell end
                args.AllowAddition = false;
                return;
            }

            if (qr.MatchingDataSetID.Equals(m_processDataGuid))
            {
                // for this to be a valid result, the  must have an ExtraInfo that contains
                // a CoverageRequestMessage and the version must match.
                if (qr.ExtraInfo.StartsWith(ProcessChannelInfoMessage.MessageID))
                {
                    try
                    {
                        ProcessChannelInfoMessage infoRequest = new ProcessChannelInfoMessage(qr.ExtraInfo);

                        if (infoRequest.ProcessVersion == m_procVersion && infoRequest.DataChannels.Contains(ChannelIdentifier.DataCode))
                        {
                            // disallow duplicate node id's
                            for (int index = 0; index < m_queryResults.Count; index++)
                            {
                                if (m_queryResults[index].ResultNode == qr.ResultNode)
                                {
                                    args.AllowAddition = false;
                                    break;
                                }
                            }

                            return;
                        }
                    }
                    catch (Exception exception)
                    {
                        System.Diagnostics.Trace.WriteLine(
                            "The process channel info message could not be constructed from the extra info: " +
                            exception);
                    }
                }
            }

            // if we get to this point in the function then we don't allow addition.
            args.AllowAddition = false;
        }

        /// <summary>
        /// Call this function once to initiate a PyxNet query to find data sources for the coverage.
        /// This will hook up the handler to monitor for more results coming in for additional sources
        /// for the data.
        /// </summary>
        private void InitializePublishersList()
        {
            lock (m_queryResultsLock)
            {
                //lock (m_publishers)
                //{
                //    // the publishers list will be rebuilt from the query results
                //    // list, so its OK just to clear it.
                //    m_publishers.Clear();
                //}

                bool requeryNeeded = false;

                if (!m_queryResults.Started)
                {
                    requeryNeeded = true;
                }
                else if ((m_queryResults.Count == 0))
                {
                    // We should restart the query here by stopping it
                    // before the call to start, but only if it has been 
                    // longer than the normal waiting for results period.
                    if (m_queryStartTime != null)
                    {
                        TimeSpan elapsed = DateTime.Now - m_queryStartTime;
                        if (elapsed.TotalSeconds > s_WaitForPublisherSeconds)
                        {
                            requeryNeeded = true;
                        }
                    }
                }
                else if (m_queryStartTime != null)
                {
                    TimeSpan elapsed = DateTime.Now - m_queryStartTime;
                    if (elapsed.TotalSeconds > s_RequeryTimeWhenResultsFoundInSeconds)
                    {
                        requeryNeeded = true;
                    }
                }

                if (requeryNeeded)
                {
                    if (m_queryResults.Started)
                    {
                        m_queryResults.Stop();
                    }

                    //switch the old and new
                    lock (m_publishers)
                    {
                        m_queryResults.Clear();
                        m_oldPublishers = m_newPublishers;
                        m_newPublishers = new List<NodeInfo>();
                        m_publishersNeedRefersh = true;
                    }

                    m_queryResults.Start();
                    m_queryStartTime = DateTime.Now;
                }
            }
        }

        private void WaitForPublishers()
        {
            // make a query and look for query results
            m_queryResults.WaitForResults(1, s_WaitForPublisherSeconds);
        }

        public bool FoundRemotely()
        {
            InitializePublishersList();
            if (m_firstTimeCallFoundRemotly)
            {
                WaitForPublishers();
                m_firstTimeCallFoundRemotly = false;
            }
            return (PublisherCount > 0);
        }

        /// <summary>
        /// Add a query result to the list of query results that are being used to find providers
        /// of data for this coverage.
        /// </summary>
        /// <param name="qr"></param>
        public void AddQueryResult(QueryResult qr)
        {
            lock (m_queryResults)
            {
                m_queryResults.Add(qr);
            }
        }

        /// <summary>
        /// Initialize the coverage downloader and transfer the minimum amount of information
        /// over PyxNet to be able to support data transfer.  This function must be called and must
        /// return true before this is a usable object.
        /// </summary>
        /// <returns>True if the coverage is live and ready to go, otherwise false.</returns>
        public bool Initialize()
        {
            //TODO: do we want to do something here? 
            // - do we should init publisher list ? or only on demenad...

            return true;
        }

        internal void Detach()
        {
            //stop quires
            m_queryResults.Stop();
        }
    }

    /// <summary>
    /// ProcessChannelDownloaderManager responsible to manage all ProcessChannelDownloader, 
    /// to allow the ability to detach them all when needed
    /// </summary>
    public class ProcessChannelDownloaderManager
    {
        #region Members

        /// <summary>
        /// Weak referencing all Downloaders 
        /// </summary>
        WeakReferenceList<ProcessChannelDownloader> m_downloaders = new WeakReferenceList<ProcessChannelDownloader>();

        #endregion

        #region Methods

        /// <summary>
        /// Add new CoverageDownloader to manager list
        /// </summary>
        /// <param name="downloader"> a CoverageDownloader </param>
        public void Add(ProcessChannelDownloader downloader)
        {
            m_downloaders.Add(downloader);
        }

        /// <summary>
        /// Detach all Downloaders under the Manager supervise 
        /// </summary>
        public void DetachAllDownloaders()
        {
            foreach (ProcessChannelDownloader downloader in m_downloaders)
            {
                downloader.Detach();
            }
        }

        #endregion

        #region Singleton

        private static ProcessChannelDownloaderManager s_manager;

        /// <summary>
        /// Get the Manager.
        /// </summary>
        public static ProcessChannelDownloaderManager Manager
        {
            get
            {
                if (s_manager == null)
                {
                    s_manager = new ProcessChannelDownloaderManager();
                }
                return s_manager;
            }
        }

        #endregion
    }
}