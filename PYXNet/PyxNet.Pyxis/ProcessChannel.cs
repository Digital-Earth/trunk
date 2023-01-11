/******************************************************************************
PyxlibPyxnetChannelProvider.cs

begin      : July 14, 2012
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Threading;
using Pyxis.Utilities;

namespace PyxNet.Pyxis
{
    public class ProcessChannelIdentifier
    {
        public ProcessChannelIdentifier(ProcRef procRef, string dataCode)
        {
            m_procRef = procRef;
            m_dataCode = dataCode;
            UpdatePrivateId();
        }

        private ProcRef m_procRef;
        public ProcRef ProcRef
        {
            get { return m_procRef; }
            private set { m_procRef = value; UpdatePrivateId(); }
        }

        private string m_dataCode;
        public string DataCode
        {
            get { return m_dataCode; }
            private set { m_dataCode = value; UpdatePrivateId(); }
        }

        private string m_id;

        private void UpdatePrivateId()
        {
            m_id = pyxlib.procRefToStr(m_procRef) + ":" + m_dataCode;
        }

        public override bool Equals(object obj)
        {
            if (obj is ProcessChannelIdentifier)
            {
                return m_id == (obj as ProcessChannelIdentifier).m_id;
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return m_id.GetHashCode();
        }

        public override string ToString()
        {
            return "Process Channel : " + pyxlib.procRefToStr(m_procRef) + ", Channel Code=" + m_dataCode;
        }
    }

    public enum ProcessChannelGeyKeyOptions
    {
        FromRemoteNodes,
        FromLocalNode,
    }

    public class ProcessChannelKeyRequest
    {
        public delegate void OnKeyRequestedCompletedDelegate(ProcessChannelKeyRequest request);

        public string Key { get; private set; }
        private byte[] _value;
        public byte[] Value
        {
            get
            {
                Wait();
                return _value;
            }
            private set { _value = value; }
        }

        public bool Completed { get; private set;  }
        public Exception Error { get; private set; }

        public bool HasError
        {
            get { return Error != null; }
        }

        private object m_lockObject = new object();
        private List<OnKeyRequestedCompletedDelegate> m_delegates = new List<OnKeyRequestedCompletedDelegate>();

        public ProcessChannelKeyRequest(string key)
        {
            Key = key;
        }

        public void OnRequestedCompleted(OnKeyRequestedCompletedDelegate aDelegate)
        {
            lock(m_lockObject)
            {
                if (!Completed)
                {
                    m_delegates.Add(aDelegate);
                    return;
                }
            }

            aDelegate(this);
        }

        public void CompleteWithValue(byte[] value)
        {
            lock (m_lockObject)
            {
                if (!Completed)
                {
                    Value = value;
                    Completed = true;
                }
            }

            InvokeOnCompletion();
        }

        public void CompleteWithError(Exception error)
        {
            lock (m_lockObject)
            {
                if (!Completed)
                {
                    Error = error;
                    Completed = true;
                }
            }

            InvokeOnCompletion();
        }

        private void InvokeOnCompletion()
        {
            List<OnKeyRequestedCompletedDelegate> oldDelegates;

            lock (m_lockObject)
            {
                oldDelegates = m_delegates;
                m_delegates = new List<OnKeyRequestedCompletedDelegate>();
            }

            if (oldDelegates != null)
            {
                foreach (var oldDelegate in oldDelegates)
                {
                    oldDelegate(this);
                }
            }
        }

        public void Wait()
        {
            if (! Completed)
            {

                AutoResetEvent taskCompleted = new AutoResetEvent(false);

                OnRequestedCompleted(delegate
                                         {
                                             taskCompleted.Set();
                                         });

                taskCompleted.WaitOne();
            }

            if (Error!=null)
            {
                throw Error;
            }
        }
    }

    public interface IProcessChannelKeyProvider
    {
        byte[] GetKey(string key);
        ProcessChannelKeyRequest GetKeyAsync(string key);
    }

    public class ProcessChannelKeyRequestedEventArgs : EventArgs
    {
        public string Key { get; private set; }

        public ProcessChannelGeyKeyOptions GetKeyOptions { get; private set; }

        public ProcessChannelKeyRequestedEventArgs(string key, ProcessChannelGeyKeyOptions getKeyOptions)
        {
            Key = key;
            GetKeyOptions = getKeyOptions;
        }
    }

    public class ProcessChannelKeyProvidedEventArgs : EventArgs
    {
        public string Key { get; private set; }
        public byte[] Value { get; private set; }

        public ProcessChannelGeyKeyOptions GetKeyOptions { get; private set; }

        public ProcessChannelKeyProvidedEventArgs(string key, byte[] value, ProcessChannelGeyKeyOptions getKeyOptions)
        {
            Key = key;
            Value = value;
            GetKeyOptions = getKeyOptions;
        }
    }

    public class ProcessChannelKeyProvidedFailedEventArgs : EventArgs
    {
        public string Key { get; private set; }
        public Exception Error { get; private set; }

        public ProcessChannelGeyKeyOptions GetKeyOptions { get; private set; }

        public ProcessChannelKeyProvidedFailedEventArgs(string key, Exception error, ProcessChannelGeyKeyOptions getKeyOptions)
        {
            Key = key;
            Error = error;
            GetKeyOptions = getKeyOptions;
        }
    }

    public interface IProcessChannel
    {
        ProcessChannelIdentifier Identifier { get; }

        bool Published { get; }
        bool FoundRemotely();

        event EventHandler<EventArgs> OnPublished;
        event EventHandler<EventArgs> OnFoundRemotely;

        event EventHandler<ProcessChannelKeyRequestedEventArgs> OnKeyRequested;
        event EventHandler<ProcessChannelKeyProvidedEventArgs> OnKeyProvided;
        event EventHandler<ProcessChannelKeyProvidedFailedEventArgs> OnKeyProvidedFailed;

        byte[] GetKey(string key, ProcessChannelGeyKeyOptions options);

        void AttachKeyProvider(IProcessChannelKeyProvider provider, ProcessChannelGeyKeyOptions options);
    }

    public class ProcessChannel : IProcessChannel
    {
        #region Members

        public int UnmanageHandle { get; private set; }

        public ProcessChannelIdentifier Identifier { get; private set; }

        public bool Published
        {
            get { return PublishedChannel != null; }
        }

        public bool HasLocalInput { get; private set; }

        private IProcessChannelKeyProvider m_remoteProvider;
        private IProcessChannelKeyProvider m_localProvider;

        internal ProcessChannelPublisher.PublishedProcessChannelsInfo.PublishedChannelInfo PublishedChannel { get; set; }
        internal ProcessChannelDownloader ChannelDownloader { get; set; }

        #endregion

        #region static functions

        static object m_activeChannelsLockObject = new object();
        static WeakReferenceList<ProcessChannel> m_activeChannels = new WeakReferenceList<ProcessChannel>();  

        public static ProcessChannel Create(ProcRef procRef, string dataCode)
        {
            var id = new ProcessChannelIdentifier(procRef,dataCode);
            lock(m_activeChannelsLockObject)
            {
                var foundChannel = m_activeChannels.Find(delegate(ProcessChannel channel)
                                          {
                                              return channel.Identifier == id;
                                          });

                if (foundChannel != null)
                {
                    return foundChannel;
                }
            }
            return new ProcessChannel(procRef, dataCode);
        }

        #endregion

        #region ctor

        private ProcessChannel(ProcRef procRef, string dataCode)
        {
            Identifier = new ProcessChannelIdentifier(procRef, dataCode);

            UnmanageHandle = -1;

            ChannelDownloader = new ProcessChannelDownloader(PublisherSingleton.Publisher.Stack, Identifier);

            AttachKeyProvider(ChannelDownloader,ProcessChannelGeyKeyOptions.FromRemoteNodes);
        }

        #endregion

        #region internal functions
        internal void SetUnmanageHandle(int handle)
        {
            UnmanageHandle = handle;
        }
        #endregion

        #region Events

        /// <summary> Event handler for when channel become published. </summary>
        public event EventHandler<EventArgs> OnPublished
        {
            add { m_onPublished.Add(value); }
            remove { m_onPublished.Remove(value); }
        }

        private EventHelper<EventArgs> m_onPublished = new EventHelper<EventArgs>();

        /// <summary>
        /// Raises the OnPublsihed event.
        /// </summary>
        public void InvokeOnPublished(object sender)
        {
            m_onPublished.Invoke(sender, new EventArgs());
        }

        /// <summary> Event handler for when remote node found to published the channel. </summary>
        public event EventHandler<EventArgs> OnFoundRemotely
        {
            add { m_onFoundRemotely.Add(value); }
            remove { m_onFoundRemotely.Remove(value); }
        }

        private EventHelper<EventArgs> m_onFoundRemotely = new EventHelper<EventArgs>();

        /// <summary>
        /// Raises the OnFoundRemotely event.
        /// </summary>
        public void InvokeFoundRemotely(object sender)
        {
            m_onFoundRemotely.Invoke(sender, new EventArgs());
        }

        /// <summary> Event handler for when key requested. </summary>
        public event EventHandler<ProcessChannelKeyRequestedEventArgs> OnKeyRequested
        {
            add { m_onKeyRequested.Add(value); }
            remove { m_onKeyRequested.Remove(value); }
        }

        private EventHelper<ProcessChannelKeyRequestedEventArgs> m_onKeyRequested = new EventHelper<ProcessChannelKeyRequestedEventArgs>();


        /// <summary>
        /// Raises the OnKeyRequested event.
        /// </summary>
        protected void InvokeKeyRequested(object sender, string key, ProcessChannelGeyKeyOptions options)
        {
            m_onKeyRequested.Invoke(sender, new ProcessChannelKeyRequestedEventArgs(key, options));
        }

        /// <summary> Event handler for when a key was provided. </summary>
        public event EventHandler<ProcessChannelKeyProvidedEventArgs> OnKeyProvided
        {
            add { m_onKeyProvided.Add(value); }
            remove { m_onKeyProvided.Remove(value); }
        }

        private EventHelper<ProcessChannelKeyProvidedEventArgs> m_onKeyProvided =
            new EventHelper<ProcessChannelKeyProvidedEventArgs>();

        /// <summary>
        /// Raises the OnKeyProvided event.
        /// </summary>
        protected void InvokeKeyProvided(object sender, string key, byte[] value, ProcessChannelGeyKeyOptions options)
        {
            m_onKeyProvided.Invoke(sender, new ProcessChannelKeyProvidedEventArgs(key, value, options));            
        }

        /// <summary> Event handler for when failed to provide a key (timeout/other error). </summary>
        public event EventHandler<ProcessChannelKeyProvidedFailedEventArgs> OnKeyProvidedFailed
        {
            add { m_onKeyProvidedFailed.Add(value); }
            remove { m_onKeyProvidedFailed.Remove(value); }
        }


        private EventHelper<ProcessChannelKeyProvidedFailedEventArgs> m_onKeyProvidedFailed = new EventHelper<ProcessChannelKeyProvidedFailedEventArgs>();

        /// <summary>
        /// Raises the KeyProvidedFailed event.
        /// </summary>
        protected void InvokeKeyProvidedFailed(object sender, string key, Exception error, ProcessChannelGeyKeyOptions options)
        {
            m_onKeyProvidedFailed.Invoke(sender, new ProcessChannelKeyProvidedFailedEventArgs(key, error, options));
        }

        #endregion

        #region State functions

        public bool FoundRemotely()
        {
            if (m_remoteProvider is ProcessChannelDownloader)
            {
                return (m_remoteProvider as ProcessChannelDownloader).FoundRemotely();
            }
            throw new NotImplementedException("Unknown remote provider type");
        }

        #endregion
        #region GetKey functions

        public ProcessChannelKeyRequest GetKeyAsync(string key, ProcessChannelGeyKeyOptions options)
        {
            switch (options)
            {
                case ProcessChannelGeyKeyOptions.FromRemoteNodes:
                    InvokeKeyRequested(this, key, options);
                    try
                    {
                        var request = m_remoteProvider.GetKeyAsync(key);

                        request.OnRequestedCompleted(delegate
                        {
                            if (request.HasError)
                            {
                                InvokeKeyProvidedFailed(this, key, request.Error, options);
                            }
                            else
                            {
                                InvokeKeyProvided(this, key, request.Value, options);
                            }
                        });
                        return request;
                    }
                    catch (Exception error)
                    {
                        InvokeKeyProvidedFailed(this, key, error, options);
                        throw;
                    }

                case ProcessChannelGeyKeyOptions.FromLocalNode:
                    InvokeKeyRequested(this, key, options);
                    try
                    {
                        var request = m_localProvider.GetKeyAsync(key);

                        request.OnRequestedCompleted(delegate
                        {
                            if (request.HasError)
                            {
                                InvokeKeyProvidedFailed(this, key, request.Error, options);
                            }
                            else
                            {
                                InvokeKeyProvided(this, key, request.Value, options);
                            }
                        });
                        return request;
                    }
                    catch (Exception error)
                    {
                        InvokeKeyProvidedFailed(this, key, error, options);
                        throw;
                    }

                default:
                    throw new Exception("unkown options value: " + options);
            }
        }

        public byte[] GetKey(string key, ProcessChannelGeyKeyOptions options)
        {
            switch (options)
            {
                case ProcessChannelGeyKeyOptions.FromRemoteNodes:
                    InvokeKeyRequested(this, key, options);
                    try
                    {
                        var result = m_remoteProvider.GetKey(key);
                        InvokeKeyProvided(this, key, result, options);
                        return result;
                    }
                    catch (Exception error)
                    {
                        InvokeKeyProvidedFailed(this,key,error, options);
                        throw;
                    }

                case ProcessChannelGeyKeyOptions.FromLocalNode:
                    InvokeKeyRequested(this, key, options);
                    try
                    {
                        var result = m_localProvider.GetKey(key);
                        InvokeKeyProvided(this, key, result, options);
                        return result;
                    }
                    catch (Exception error)
                    {
                        InvokeKeyProvidedFailed(this, key, error, options);
                        throw;
                    }

                default:
                    throw new Exception("unkown options value: " + options);
            }
        }

        #endregion

        #region Attaching providers 
        public void AttachKeyProvider(IProcessChannelKeyProvider provider, ProcessChannelGeyKeyOptions options)
        {
            switch (options)
            {
                case ProcessChannelGeyKeyOptions.FromRemoteNodes:
                    m_remoteProvider = provider;
                    break;

                case ProcessChannelGeyKeyOptions.FromLocalNode:
                    m_localProvider = provider;
                    break;

                default:
                    throw new Exception("unknown options value: " + options);
            }
        }
        #endregion

        #region Publish functions

        public void Publish()
        {
            var publisher = PublisherSingleton.Publisher.ProcessChannelPublisher;
            var publishedProcess = publisher.GetPublishedChannel(Identifier.ProcRef);
            if (publishedProcess == null)
            {
                //TODO: we need license server here somehow...

                //publish the pipeline...
                publisher.Publish(Identifier.ProcRef, null);

                //get the channel
                publishedProcess = publisher.GetPublishedChannel(Identifier.ProcRef);
            }

            if (publishedProcess == null)
            {
                throw new Exception("failed to published the process");
            }

            PublishedChannel = publishedProcess.FindChannel(Identifier.DataCode);

            if (PublishedChannel == null)
            {
                PublishedChannel = publishedProcess.AddChannel(this);
                InvokeOnPublished(this);
            }
        }

        public void Unpublish()
        {
            if (PublishedChannel != null)
            {
                PublishedChannel.PublishedProcess.RemoveChannel(Identifier.DataCode);
            }
        }

        #endregion

    }
}