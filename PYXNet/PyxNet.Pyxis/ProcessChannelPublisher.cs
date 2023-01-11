using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using PyxNet.DataHandling;
using PyxNet.Service;
using Pyxis.Utilities;


namespace PyxNet.Pyxis
{
    public class ProcessChannelPublisher
    {
        public class PublishedProcessChannelsInfo : Publishing.Publisher.IPublishedItemInfo
        {
            public class PublishedChannelInfo
            {
                #region Members

                public string ChannelDataCode { get; private set; }
                public Certificate Certificate { get; set; }

                public PublishedProcessChannelsInfo PublishedProcess { get; private set; }

                private ProcessChannel m_channel;

                private ProcessChannelRequestMessage m_channelInfo;

                /// <summary>
                /// Helper property to get a ProcessChannelMessage that is initialized to
                /// have all the information about the published process channel
                /// </summary>
                /// <returns></returns>
                public ProcessChannelRequestMessage ChannelInfo
                {
                    get
                    {
                        if (m_channelInfo == null)
                        {

                            m_channelInfo = new ProcessChannelRequestMessage()
                                                {
                                                    DataCode = ChannelDataCode,
                                                    ProcessVersion = PublishedProcess.ProcessVersion,
                                                };
                        }
                        return m_channelInfo;
                    }
                }

                #endregion

                public PublishedChannelInfo(PublishedProcessChannelsInfo process, string code, ProcessChannel processChannel)
                {
                    PublishedProcess = process;
                    ChannelDataCode = code;
                    m_channel = processChannel;
                }

                public ProcessChannel GetDataChannel()
                {
                    return m_channel;
                }
            }

            #region Fields And Properties

            private DataInfo m_dataInfo = new DataInfo();

            public DataInfo DataInfo
            {
                get { return m_dataInfo; }
                set { m_dataInfo = value; }
            }

            /// <summary>
            /// The process we are publishing.
            /// </summary>
            public IProcess_SPtr Process
            {
                get 
                { 
                    //recover process when needed
                    return PipeManager.getProcess(ProcRef); 
                }
            }

            /// <summary>
            /// Storage for the version of the process that we are publishing.
            /// </summary>
            private int m_processVersion;

            /// <summary>
            /// The version of the process that we are publishing.
            /// </summary>
            public int ProcessVersion
            {
                get { return m_processVersion; }
                set
                {
                    m_processVersion = value;
                    foreach (var channel in m_channels.Values)
                    {
                        channel.ChannelInfo.ProcessVersion = m_processVersion;
                    }
                }
            }

            public PYXGeometry_SPtr Geometry { get; private set; }

            private object m_channelsLockObject = new object();
            private Dictionary<string, PublishedChannelInfo> m_channels = new Dictionary<string, PublishedChannelInfo>();

            public IEnumerable<PublishedChannelInfo> Channels
            {
                get { return m_channels.Values; }
            }

            private string m_name;

            private string m_description;

            private string m_lowerCaseDescription;

            private List<string> m_keywords = null;

            /// <summary>
            /// Gets the keywords (a list of words that match this item).
            /// </summary>
            /// <value>The keywords.</value>
            private List<string> Keywords
            {
                get
                {
                    if (m_keywords == null)
                    {
                        // We could lock on this, but it doesn't matter that much.  
                        List<string> keywords = new List<string>();
                        keywords.AddRange(m_lowerCaseDescription.Split(null));
                        keywords.AddRange(m_name.ToLower().Split(null));
                    
                        m_keywords = keywords;
                    }
                    return m_keywords;
                }
            }

            private string m_procGuidString;

            public string Description
            {
                get { return m_description; }
                set
                {
                    m_description = value;
                    m_lowerCaseDescription = m_description.ToLower();
                    m_keywords = null;
                }
            }

            /// <summary>
            /// The pipeline definition of this process.
            /// </summary>
            public string PipelineDefinition { get; set; }

            /// <summary>
            /// Gets this pipeline's proc ref.
            /// </summary>
            /// <value>The proc ref.</value>
            public ProcRef ProcRef
            {
                get
                {
                    return new ProcRef(pyxlib.strToGuid(m_procGuidString), m_processVersion);
                }
            }

            private ProcessChannelInfoMessage m_processChannelInfo;
            /// <summary>
            /// Helper property to get a ProcessChannelMessage that is initialized to
            /// have all the information about the published process channel
            /// </summary>
            /// <returns></returns>
            public ProcessChannelInfoMessage ProcessChannelsInfo
            {
                get
                {
                    if (m_processChannelInfo == null)
                    {

                        m_processChannelInfo = new ProcessChannelInfoMessage()
                                                   {
                                                       ProcessVersion = ProcessVersion,
                                                       Geometry = Geometry,
                                                       PipelineDefinition = PipelineDefinition,
                                                   };

                        lock (m_channelsLockObject)
                        {
                            m_processChannelInfo.DataChannels = new List<string>(m_channels.Keys);
                        }
                    }
                    return m_processChannelInfo;
                }
            }

            private PyxNet.Service.Certificate m_certificate;

            public PyxNet.Service.Certificate Certificate
            {
                get { return m_certificate; }
                set { m_certificate = value; }
            }

            #endregion

            /// <summary>
            /// Constructor.
            /// </summary>
            /// <param name="spProc">A handle to the process.</param>
            public PublishedProcessChannelsInfo(IProcess_SPtr spProc)
            {
                if (null == spProc || spProc.isNull())
                {
                    throw new ArgumentNullException("spProc");
                }
                
                ProcessVersion = spProc.getProcVersion();
                m_procGuidString = pyxlib.guidToStr(spProc.getProcID());
                m_name = spProc.getProcName();
                Description = spProc.getProcDescription();

                PipelineDefinition = PipeManager.writePipelineToNewString(spProc);

                //extract geometry from process output...
                IFeature_SPtr feature = pyxlib.QueryInterface_IFeature(spProc.getOutput());

                if (feature != null && feature.isNotNull())
                {
                    Geometry = feature.getGeometry();
                }

                DataInfo dataInfo = new DataInfo();
                // set the data id guid to be the guid of the process...
                dataInfo.DataSetID.Guid = new Guid(pyxlib.guidToStr(spProc.getProcID()));
                dataInfo.DataChunkSize = 65535;
                dataInfo.DataLength = 0;
                dataInfo.AllAvailable = true;
                dataInfo.UseEncryption = false;
                dataInfo.UseSigning = false;
                dataInfo.UsesHashCodes = false;

                DataInfo = dataInfo;
            }

            #region Equality

            public override bool Equals(object obj)
            {
                return Equals(obj as PublishedProcessChannelsInfo);
            }

            public bool Equals(PublishedProcessChannelsInfo info)
            {
                if (null == info)
                {
                    return false;
                }

                if (info.DataInfo.DataSetID == DataInfo.DataSetID &&
                    info.ProcessVersion == ProcessVersion)
                {
                    return true;
                }

                return false;
            }

            public override int GetHashCode()
            {
                throw new MissingMethodException("Method not implemented.");
            }

            #endregion

            #region Find/Add/Remove Channels

            public PublishedChannelInfo FindChannel(string code)
            {
                lock (m_channelsLockObject)
                {
                    PublishedChannelInfo channel;

                    if (m_channels.TryGetValue(code, out channel))
                    {
                        return channel;
                    }
                    return null;
                }
            }

            public PublishedChannelInfo AddChannel(ProcessChannel channel)
            {
                var code = channel.Identifier.DataCode;
                lock (m_channelsLockObject)
                {
                    if (!m_channels.ContainsKey(code))
                    {
                        m_channels[code] = new PublishedChannelInfo(this, code, channel);
                        ProcessChannelsInfo.DataChannels = new List<string>(m_channels.Keys);
                    }
                    return m_channels[code];
                }
            }

            public void RemoveChannel(string code)
            {
                lock (m_channelsLockObject)
                {
                    if (m_channels.ContainsKey(code))
                    {
                        m_channels.Remove(code);
                        ProcessChannelsInfo.DataChannels = new List<string>(m_channels.Keys);
                    }
                }
            }

            #endregion


            /// <summary>
            /// Check to see if a query string matches the published coverage in any way.
            /// </summary>
            /// <param name="testAgainst">The string to test.</param>
            /// <returns>True if the string matches.</returns>
            public bool Matches(string testAgainst)
            {
                string lowerCaseTestAgainst = testAgainst.ToLower();
                string[] wordsToMatch = lowerCaseTestAgainst.Split(null);

                bool matched = true;
                foreach (string word in wordsToMatch)
                {
                    if (!Keywords.Contains(word))
                    {
                        matched = false;
                        break;
                    }
                }
                if (matched)
                {
                    return true;
                }

                // check to see if the Guid matches in string format.
                return m_procGuidString.Equals(testAgainst);
            }

            #region IPublishedItemInfo Members

            IEnumerable<string> Publishing.Publisher.IPublishedItemInfo.Keywords
            {
                get
                {
                    // add in the guid of the process.
                    yield return m_procGuidString;

                    // add in all the parts of the description
                    foreach (string keyword in this.Keywords)
                    {
                        yield return keyword;
                    }

                    // TODO: scan for meta data that could be used to search on
                    // and add it to the local query hash table.
                }
            }

            public QueryResult Matches(Query query, Stack stack)
            {
                if (Matches(query.Contents))
                {
                    // we found a match
                    NodeInfo connectedNodeInfo = null;
                    foreach (NodeInfo nodeInfo in stack.KnownHubList.ConnectedHubs)
                    {
                        connectedNodeInfo = nodeInfo;
                        break;
                    }
                    QueryResult queryResult =
                        new QueryResult(query.Guid, query.OriginNode, stack.NodeInfo, connectedNodeInfo);
                    queryResult.MatchingContents = m_name;
                    queryResult.MatchingDescription = Description;
                    queryResult.MatchingDataSetID = DataInfo.DataSetID;

                    // add in extra info here about version number add available channels
                    lock (m_channelsLockObject)
                    {
                        queryResult.ExtraInfo = ProcessChannelsInfo.ToMessage();
                    }
                    return queryResult;
                }

                return null;
            }

            #endregion
        }

        #region Fields and Properties

        // A TraceTool that keeps a log of where we got all our pieces from.
        private NumberedTraceTool<CoveragePublisher> m_tracer
            = new NumberedTraceTool<CoveragePublisher>(TraceTool.GlobalTraceLogEnabled);



        /// <summary>
        /// This list of coverages that we are publishing.
        /// </summary>
        private Dictionary<string, PublishedProcessChannelsInfo> m_publishedPipelines = new Dictionary<string, PublishedProcessChannelsInfo>();
        private object m_publishedPipelinesLockObject = new object();

        /// <summary>
        /// The stack that we are using to publish/monitor.
        /// </summary>
        private readonly Stack m_stack;

        #endregion

        #region RequestsCache

        private class CachedRequestItem
        {
            public PublishedProcessChannelsInfo.PublishedChannelInfo Channel { get; set; }
            public string Key { get; set; }
            public byte[] Value { get; set; }
        }

        private object m_requestsCacheLockObject = new object();
        private List<CachedRequestItem> m_requestsCache = new List<CachedRequestItem>();

        private CachedRequestItem FetchCachedRequest(PublishedProcessChannelsInfo.PublishedChannelInfo channel, string key)
        {
            lock(m_requestsCacheLockObject)
            {
                foreach(var item in m_requestsCache)
                {
                    if (item.Channel == channel && item.Key == key)
                    {
                        m_requestsCache.Remove(item);
                        m_requestsCache.Insert(0,item);

                        return item;
                    }
                }
            }

            return null;
        }

        private void AddCachedRequest(PublishedProcessChannelsInfo.PublishedChannelInfo channel, string key, byte[] value)
        {
            lock (m_requestsCacheLockObject)
            {
                m_requestsCache.Insert(0, new CachedRequestItem()
                                              {
                                                  Channel = channel,
                                                  Key = key,
                                                  Value = value
                                              });

                if (m_requestsCache.Count > 10)
                {
                    m_requestsCache.RemoveRange(10, m_requestsCache.Count - 10);
                }
            }
        }

        #endregion

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack that you want to publish on.</param>
        public ProcessChannelPublisher(Stack stack)
        {
            m_stack = stack;

            // register the messages that we want to monitor.
            m_stack.RegisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.RegisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);
        }

        /// <summary>
        /// Stop publishing all coverages, and stop responding to messages.
        /// </summary>
        public void StopPublishing()
        {
            // unregister the messages that we want to monitor.
            m_stack.UnregisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.UnregisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);

            lock (m_publishedPipelinesLockObject)
            {
                m_publishedPipelines.Clear();
            }
        }

        #region DisplayUri Event

        /// <summary> Event handler for DisplayUri. </summary>
        public event EventHandler<DisplayUriEventArgs> DisplayUri
        {
            add
            {
                m_DisplayUri.Add(value);
            }
            remove
            {
                m_DisplayUri.Remove(value);
            }
        }

        private EventHelper<DisplayUriEventArgs> m_DisplayUri = new EventHelper<DisplayUriEventArgs>();

        /// <summary>
        /// Raises the DisplayUri event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theUri"></param>
        public void OnDisplayUri(object sender, Uri theUri)
        {
            m_DisplayUri.Invoke( sender, new DisplayUriEventArgs(theUri));
        }
        #endregion DisplayUri Event

        #region Published Event

        private EventHelper<PublishedEventArgs> m_Published =
            new EventHelper<PublishedEventArgs>();

        /// <summary> Event handler for Published. </summary>
        public event EventHandler<PublishedEventArgs> Published
        {
            add
            {
                m_Published.Add(value);
            }
            remove
            {
                m_Published.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Published event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theProcess"></param>
        public void OnPublished(object sender, IProcess_SPtr theProcess)
        {
            m_Published.Invoke(sender, new PublishedEventArgs(theProcess));
        }

        /// <summary>
        /// Raises the Published event, using the default sender (this).
        /// </summary>
        /// <param name="theProcess"></param>
        public void OnPublished(IProcess_SPtr theProcess)
        {
            OnPublished(this, theProcess);
        }

        #endregion Published Event

        #region Message Handlers

        /// <summary>
        /// Handle DataChunkRequest messages.  Look for any data that we are publishing that
        /// matches the chunk request and return the chunk of data if we have it.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleChunkRequest(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            m_tracer.DebugWriteLine("Handling Chunk request from {0}", args.Context.Sender);

            try
            {
                DataChunkRequest request = new DataChunkRequest(args.Message);

                if (request.ExtraInfo.Identifier == ProcessChannelRequestMessage.MessageID)
                {
                    HandleSingleKeyUploadRequest((Stack)sender, request, args);
                }
                else if (request.ExtraInfo.Identifier == ProcessChannelMultiKeyRequestMessage.MessageID)
                {
                    HandleMultiKeyUploadRequest((Stack)sender, request, args);
                }
            }
            catch (Exception err)
            {
                System.Diagnostics.Trace.WriteLine("Failed to handle chunk request with error " + err.ToString());
                m_tracer.WriteLine("Failed to handle chunk request with error " + err.ToString());
            }
        }

        private bool IsValidRequest(DataChunkRequest request,ProcRef procRef)
        {
#if ENFORCE_CERTIFICATE
            if (request.Certificate == null)
            {                
                return false;
            }
            
            if (!request.Certificate.Valid)
            {
                return false;
            }
            
            if (!m_stack.CertificateValidator.IsCertificateValid(request.Certificate))
            {
                return false;
            }

            var fact = request.Certificate.FindFirstFact<GeoSourcePermissionFact>();
            if (fact == null)
            {
                return false;
            }

            var factProcRef = pyxlib.strToProcRef(fact.ProcRef);

            if (procRef != factProcRef)
            {
                return false;
            }
#endif
            return true;
        }

        private void HandleMultiKeyUploadRequest(Stack stack, DataChunkRequest request, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            if (!request.ExtraInfo.StartsWith(ProcessChannelMultiKeyRequestMessage.MessageID))
            {
                // it must have an ExtraInfo that is a ProcessChannelRequestMessage to make sense of this request.
                return;
            }

            var dataRequest = new ProcessChannelMultiKeyRequestMessage(request.ExtraInfo);

            var procRef = new ProcRef(pyxlib.strToGuid(request.DataSetID.Guid.ToString()), dataRequest.ProcessVersion);

            if (!IsValidRequest(request, procRef))
            {
                return;
            }

            string key = GetPipelineKey(procRef);

            PublishedProcessChannelsInfo publishedPipeline = null;

            lock (m_publishedPipelinesLockObject)
            {
                if (!m_publishedPipelines.TryGetValue(key, out publishedPipeline))
                {
                    //we don't publish the given pipeline.
                    return;
                }
            }

            var publishedChannel = publishedPipeline.FindChannel(dataRequest.DataCode);

            if (publishedChannel == null)
            {
                //we don't publish the given channel
                return;
            }

            using(var resultStream = new MemoryStream())
            {
                var foundKeys = new ProcessChannelMultiKeyRequestMessage()
                                    {
                                        DataCode = dataRequest.DataCode,
                                        ProcessVersion = dataRequest.ProcessVersion,
                                        RequestedKeys = new List<string>()
                                    };

                foreach (var aKey in dataRequest.RequestedKeys)
                {
                    var dataChunk = GetDataChunkForRequest(publishedChannel, aKey, request);

                    if (dataChunk != null)
                    {
                        foundKeys.RequestedKeys.Add(aKey);
                        resultStream.Write(dataChunk.Data.Data,0,dataChunk.ChunkSize);
                    }
                }

                //memory stream buffer length is sometimes larger then resultStream.Length. therefore, we copy only what we need.
                var data = new byte[resultStream.Length];
                resultStream.Seek(0, SeekOrigin.Begin);
                resultStream.Read(data, 0, (int)resultStream.Length);

                var finalDataChunk = new DataChunk(data, 0);

                finalDataChunk.DataSetID = request.DataSetID;
                finalDataChunk.ExtraInfo = foundKeys.ToMessage();

                args.Context.Sender.SendMessage(finalDataChunk.ToMessage());

                GlobalPerformanceCounters.Counters.CoverageBytesUploaded.IncrementBy(finalDataChunk.ChunkSize);

                UsageReports.CoverageBytesUploaded(stack.NodeInfo.FriendlyName,
                                                   stack.NodeInfo.NodeGUID,
                                                   args.Context.Sender.RemoteNodeInfo.FriendlyName,
                                                   args.Context.Sender.RemoteNodeInfo.NodeGUID,
                                                   finalDataChunk.DataSetID,
                                                   finalDataChunk.ChunkSize
                    );
            }
        }

        private void HandleSingleKeyUploadRequest(Stack stack, DataChunkRequest request, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            if (!request.ExtraInfo.StartsWith(ProcessChannelRequestMessage.MessageID))
            {
                // it must have an ExtraInfo that is a ProcessChannelRequestMessage to make sense of this request.
                return;
            }

            var dataRequest = new ProcessChannelRequestMessage(request.ExtraInfo);

            var procRef = new ProcRef(pyxlib.strToGuid(request.DataSetID.Guid.ToString()), dataRequest.ProcessVersion);

            if (!IsValidRequest(request, procRef))
            {
                return;
            }

            string key = GetPipelineKey(procRef);

            PublishedProcessChannelsInfo publishedPipeline = null;

            lock (m_publishedPipelinesLockObject)
            {
                if (!m_publishedPipelines.TryGetValue(key, out publishedPipeline))
                {
                    //we don't publish the given pipeline.
                    return;
                }
            }

            var publishedChannel = publishedPipeline.FindChannel(dataRequest.DataCode);

            if (publishedChannel == null)
            {
                //we don't publish the given channel
                return;
            }

            var dataChunk = GetDataChunkForRequest(publishedChannel, dataRequest.RequestedKey, request);

            if (dataChunk != null)
            {
                dataChunk.DataSetID = request.DataSetID;
                dataChunk.ExtraInfo = request.ExtraInfo;

                args.Context.Sender.SendMessage(dataChunk.ToMessage());
                GlobalPerformanceCounters.Counters.CoverageBytesUploaded.IncrementBy(dataChunk.ChunkSize);

                UsageReports.CoverageBytesUploaded(stack.NodeInfo.FriendlyName,
                                                   stack.NodeInfo.NodeGUID,
                                                   args.Context.Sender.RemoteNodeInfo.FriendlyName,
                                                   args.Context.Sender.RemoteNodeInfo.NodeGUID,
                                                   dataChunk.DataSetID,
                                                   dataChunk.ChunkSize
                    );
            }
        }


        private byte[] GetDataForRequest(PublishedProcessChannelsInfo.PublishedChannelInfo publishedChannel, string key)
        {
            byte[] value = null;

            CachedRequestItem cached = FetchCachedRequest(publishedChannel, key);

            if (cached != null && cached.Key == key)
            {
                value = cached.Value;
            }
            else
            {
                try
                {
                    value = publishedChannel.GetDataChannel().GetKey(key, ProcessChannelGeyKeyOptions.FromLocalNode);
                }
                catch (Exception)
                {
                    value = null;
                }

                AddCachedRequest(publishedChannel, key, value);
            }

            return value;
        }

        private DataChunk GetDataChunkForRequest(PublishedProcessChannelsInfo.PublishedChannelInfo publishedChannel, string key, DataChunkRequest request)
        {
            var value = GetDataForRequest(publishedChannel, key);

            if (value == null)
            {
                return null;
            }
            int count = Math.Min(request.ChunkSize, value.Length - request.Offset);

            if (count > 0)
            {
                var buffer = new byte[count];
                Buffer.BlockCopy(value, request.Offset, buffer, 0, count);
                return new DataChunk(buffer, request.Offset);
            }

            return null;
        }

        /// <summary>
        /// Handle InfoRequest messages.  Look for any data we are publishing and
        /// return the info that is pertinent to that data.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleInfoRequest(object sender,
                                      PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            m_tracer.DebugWriteLine("Handling Info request from {0}", args.Context.Sender);

            DataInfoRequest request = new DataInfoRequest(args.Message);

            if (request.ExtraInfo.Identifier == ProcessChannelRequestMessage.MessageID)
            {
                HandleSingleKeyRequest(request,args);
            }
            else if (request.ExtraInfo.Identifier == ProcessChannelMultiKeyRequestMessage.MessageID)
            {
                HandleMultiKeyRequest(request, args);
            }
        }

        private void HandleMultiKeyRequest(DataInfoRequest request, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            var dataRequest = new ProcessChannelMultiKeyRequestMessage(request.ExtraInfo);

            string key =
                GetPipelineKey(new ProcRef(pyxlib.strToGuid(request.DataSetID.Guid.ToString()), dataRequest.ProcessVersion));

            PublishedProcessChannelsInfo publishedPipeline = null;

            lock (m_publishedPipelinesLockObject)
            {
                if (!m_publishedPipelines.TryGetValue(key, out publishedPipeline))
                {
                    //we don't publish the given pipeline.
                    return;
                }
            }

            var publishedChannel = publishedPipeline.FindChannel(dataRequest.DataCode);

            if (publishedChannel == null)
            {
                //we don't publish the given channel
                return;
            }

            var resultMessage = new ProcessChannelMultiKeyInfoMessage()
                                    {
                                        DataCode = dataRequest.DataCode,
                                        ProcessVersion = dataRequest.ProcessVersion,
                                        Keys = new List<ProcessChannelMultiKeyInfoMessage.KeyInfo>()
                                    };

            var totalSize = 0;

            foreach (var requstedKey in dataRequest.RequestedKeys)
            {
                var value = GetDataForRequest(publishedChannel, requstedKey);

                if (value != null)
                {
                    resultMessage.Keys.Add(new ProcessChannelMultiKeyInfoMessage.KeyInfo(requstedKey, value.Length));                                               
                }
                else
                {
                    resultMessage.Keys.Add(new ProcessChannelMultiKeyInfoMessage.KeyInfo(requstedKey));
                }
            }

            var returnInfo = new DataInfo(publishedPipeline.DataInfo)
                                 {
                                     ExtraInfo = resultMessage.ToMessage(), 
                                     DataLength = totalSize
                                 };
            m_tracer.DebugWriteLine("Responding to Info request from {0}", args.Context.Sender);
            args.Context.Sender.SendMessage(returnInfo.ToMessage());
        }

        private void HandleSingleKeyRequest(DataInfoRequest request, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            var dataRequest = new ProcessChannelRequestMessage(request.ExtraInfo);

            string key =
                GetPipelineKey(new ProcRef(pyxlib.strToGuid(request.DataSetID.Guid.ToString()),dataRequest.ProcessVersion));

            PublishedProcessChannelsInfo publishedPipeline = null;

            lock (m_publishedPipelinesLockObject)
            {
                if (!m_publishedPipelines.TryGetValue(key, out publishedPipeline))
                {
                    //we don't publish the given pipeline.
                    return;
                }
            }

            var publishedChannel = publishedPipeline.FindChannel(dataRequest.DataCode);

            if (publishedChannel == null)
            {
                //we don't publish the given channel
                return;
            }

            var value = GetDataForRequest(publishedChannel, dataRequest.RequestedKey);

            if (value != null)
            {
                var returnInfo = new DataInfo(publishedPipeline.DataInfo)
                                     {
                                         ExtraInfo = request.ExtraInfo, 
                                         DataLength = value.Length
                                     };

                m_tracer.DebugWriteLine("Responding to Info request from {0}", args.Context.Sender);
                args.Context.Sender.SendMessage(returnInfo.ToMessage());
            }
            else
            {
                DataNoInfo returnInfo = new DataNoInfo(publishedPipeline.DataInfo.DataSetID)
                                            {
                                                ExtraInfo = request.ExtraInfo
                                            };

                args.Context.Sender.SendMessage(returnInfo.ToMessage());
            }
        }

        #endregion

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="processHandle">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(ProcRef processHandle, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == processHandle)
            {
                return new IProcess_SPtr();
            }

            // Get the process through the process resolver.
            IProcess_SPtr spProcess = PipeManager.getProcess(processHandle);

            return Publish(spProcess, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="spProc">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(IProcess_SPtr spProc, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == spProc)
            {
                return new IProcess_SPtr();
            }

            PublishedProcessChannelsInfo pub;
            try
            {
                pub = new PublishedProcessChannelsInfo(spProc);
            }
            catch (ArgumentException)
            {
                // It's not a coverage process.
                return new IProcess_SPtr();
            }

            return Publish(pub, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="pub">The info about the coverage to publish.</param>
        /// <param name="licenseServer">The licence server to use to negotiate a licence.
        /// Can be null if no negotiation is wanted.</param>
        /// <returns>The process smart pointer, which points to null if unsuccessful.</returns>
        private IProcess_SPtr Publish(
            PublishedProcessChannelsInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            var key = GetPipelineKey(pub.ProcRef);
            // Return if it's already published.
            lock (m_publishedPipelinesLockObject)
            {
                if (m_publishedPipelines.ContainsKey(key))
                {
                    return m_publishedPipelines[key].Process;
                }
            }

            // If we made it here, we are trying publishing a new coverage (to us).
            if (GetExistingPublicationCertificate(pub) ||
                (RequestPublicationCertificate(pub, licenseServer)))
            {
                bool needToPublishItem = false;


                lock (m_publishedPipelinesLockObject)
                {
                    if (!m_publishedPipelines.ContainsKey(key))
                    {
                        m_publishedPipelines[key] = pub;
                        needToPublishItem = true;
                    }
                }

                // we found a local certificate to publish, so we are OK to publish.
                if (needToPublishItem)
                {
                    m_stack.Publisher.PublishItem(pub);
                    OnPublished(pub.Process);
                }

                // TODO: verify the return value here.
                return pub.Process;
            }

            return new IProcess_SPtr();
        }

        /// <summary>
        /// This function will look for the publication certificate, first in the local
        /// stack's repository, then search for it on PyxNet.  
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool GetExistingPublicationCertificate(
            PublishedProcessChannelsInfo pub)
        {
#if LICENSE_SERVER
            Stack stack = m_stack;

            // Build the fact that we want to find.
            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.DataInfo.DataSetID.Guid);
            PyxNet.Service.ResourceDefinitionFact newFact =
                new PyxNet.Service.ResourceDefinitionFact(
                    pub.CoverageInfo.PipelineDefinition);
            newFact.ResourceId = coverageResourceId;

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ResourceDefinitionFact resourceDefinitionFact =
                finder.Find() as PyxNet.Service.ResourceDefinitionFact;

            // Return true if the certificate is not null.
            if (resourceDefinitionFact != null)
            {
                pub.Certificate = GetExistingResourcePermisson(resourceDefinitionFact.ResourceId);
            }
            return pub.Certificate != null;
#else
            return true;
#endif
        }

        private PyxNet.Service.Certificate GetExistingResourcePermisson(
            PyxNet.Service.ResourceId resourceId)
        {
            Stack stack = m_stack;

            // TODO: Change to PipelinePermissionFact
            // Build the fact that we want to find.
            PyxNet.Service.ResourcePermissionFact newFact =
                new PyxNet.Service.ResourcePermissionFact(resourceId,
                                                          stack.NodeInfo.NodeId);

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ICertifiableFact resourcePermissionFact =
                finder.Find();

            // Return it.
            if (resourcePermissionFact != null)
            {
                return resourcePermissionFact.Certificate;
            }
            return null;
        }

        /// <summary>
        /// This function will request the publication certificate from a server.
        /// It is a blocking call.
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool RequestPublicationCertificate(
            PublishedProcessChannelsInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            // TODO: Deal with equivalency.

            Stack stack = m_stack;

            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.DataInfo.DataSetID.Guid);

            PyxNet.Service.ResourceDefinitionFact newDefinitionFact =
                new PyxNet.Service.ResourceDefinitionFact(pub.PipelineDefinition);
            newDefinitionFact.ResourceId = coverageResourceId;

            // TODO: Switch to PipelinePermissionFact
            PyxNet.Service.ResourcePermissionFact newPermissionFact =
                new PyxNet.Service.ResourcePermissionFact(coverageResourceId,
                                                          stack.NodeInfo.NodeId);

            // Search for a certificate locally.
            foreach (PyxNet.Service.ResourceDefinitionFact fact in
                m_stack.CertificateRepository.GetMatchingFacts(
                    newDefinitionFact.UniqueKeyword, newDefinitionFact.GetType()))
            {
                System.Diagnostics.Debug.Assert(fact != null);
                pub.Certificate = GetExistingResourcePermisson(fact.ResourceId);
                return true;
            }

            SynchronizationEvent permissionGrantedTimer = 
                new SynchronizationEvent();

            // If here, we don't have a certificate.  Request one.
            PyxNet.Service.CertificateRequester requester =
                new PyxNet.Service.CertificateRequester(stack,
                                                        newDefinitionFact, newPermissionFact);
            requester.DisplayUri +=
                delegate(object sender, PyxNet.DisplayUriEventArgs a)
                    {
                        OnDisplayUri(this, a.Uri);
                    };
            requester.CertificateReceived +=
                delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                    {
                        // Get the certificate.
                        PyxNet.Service.Certificate certificate = c.Certificate;
                        System.Diagnostics.Debug.Assert(certificate != null);

                        // Add it to the stack's certificate repository.
                        stack.CertificateRepository.Add(certificate);

                        // Set it in the published coverage info.
                        pub.Certificate = certificate;
                    };
            requester.PermissionGranted +=
                delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                    {
                        permissionGrantedTimer.Pulse();
                    };
            requester.Start(licenseServer, TimeSpan.FromSeconds(30));

            // Wait until we get one.
            permissionGrantedTimer.Wait();

            return true;
        }

        internal bool IsPublished(IProcess_SPtr procref)
        {
            var key = GetPipelineKey(procref);
            lock (m_publishedPipelinesLockObject)
            {
                return m_publishedPipelines.ContainsKey(key);
            }
        }

        internal PublishedProcessChannelsInfo GetPublishedChannel(ProcRef procref)
        {
            var key = GetPipelineKey(procref);
            lock (m_publishedPipelinesLockObject)
            {
                if (m_publishedPipelines.ContainsKey(key))
                    return m_publishedPipelines[key];
            }

            return null;
        }

        private string GetPipelineKey(ProcRef procRef)
        {
            return procRef.getProcID() + "[" + procRef.getProcVersion() + "]";
        }

        private string GetPipelineKey(IProcess_SPtr spProc)
        {
            return spProc.getProcID() + "[" + spProc.getProcVersion() + "]";
        }
    }
}