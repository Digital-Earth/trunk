/******************************************************************************
PyxlibPyxnetChannelProvider.cs

begin      : July 14, 2012
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using ApplicationUtility;
using PyxNet.Publishing;

namespace PyxNet.Pyxis
{

    public class PyxlibPyxnetChannelProvider : PYXNETChannelProvider, IDirectorReferenceCounter
    {
        #region Members

        private object m_marshaledChannelsLockObject = new object();
        private Dictionary<int, ProcessChannel> m_marshaledChannelsByHandle = new Dictionary<int, ProcessChannel>();
        private Dictionary<ProcessChannelIdentifier, ProcessChannel> m_marshaledChannelsByIdentifier = new Dictionary<ProcessChannelIdentifier, ProcessChannel>();
        private Random handleGenerator = new Random();

        #endregion

        #region class UnmanagedProvider

        private class UnmanagedProvider : IProcessChannelKeyProvider
        {
            private PYXNETChannelKeyProvider_SPtr KeyProvider { get; set; }
            private PyxlibPyxnetChannelProvider Provider { get; set; }

            public UnmanagedProvider(PyxlibPyxnetChannelProvider provider, PYXNETChannelKeyProvider_SPtr  keyProvider)
            {
                Provider = provider;
                KeyProvider = keyProvider;
            }

            public byte[] GetKey(string key)
            {
                var valueBase64 = KeyProvider.getKey(key);
                if (valueBase64 == "[NULL]")
                    return null;

                return Convert.FromBase64String(valueBase64);
            }

            public ProcessChannelKeyRequest GetKeyAsync(string key)
            {
                ProcessChannelKeyRequest request = new ProcessChannelKeyRequest(key);

                System.Threading.ThreadPool.QueueUserWorkItem(
                    delegate
                        {
                            try
                            {
                                request.CompleteWithValue(Convert.FromBase64String(KeyProvider.getKey(key)));
                            }
                            catch (Exception error)
                            {
                                request.CompleteWithError(error);
                            }
                        });

                return request;
            }
        }

        #endregion

        #region PYXNETChannelProvider API

        protected override int createChannel(ProcRef processProcRef, string code)
        {
            //NOTE: create procRef on Managed side, so we could keep it for future use.
            //the processProcRef.swigOwnMem = false. so the object would be deleted because its on the stack of c++.
            //therefore, we copy it and make sure it will live on Managed size (swigOwnMem = true)
            var safeProcRef = new ProcRef(processProcRef.getProcID(), processProcRef.getProcVersion());

            ProcessChannelIdentifier id = new ProcessChannelIdentifier(safeProcRef, code);

            lock (m_marshaledChannelsLockObject)
            {
                if (m_marshaledChannelsByIdentifier.ContainsKey(id))
                {
                    return m_marshaledChannelsByIdentifier[id].UnmanageHandle;
                }

                ProcessChannel channel = ProcessChannel.Create(safeProcRef, code);

                //create new handle for this channel
                int handle = handleGenerator.Next();
                while (m_marshaledChannelsByHandle.ContainsKey(handle))
                {
                    handle = handleGenerator.Next();
                }
                channel.SetUnmanageHandle(handle);


                m_marshaledChannelsByIdentifier[id] = channel;
                m_marshaledChannelsByHandle[channel.UnmanageHandle] = channel;

                return channel.UnmanageHandle;
            }
        }

        protected override void removeChannel(int channelId)
        {
           lock (m_marshaledChannelsLockObject)
           {
               if (m_marshaledChannelsByHandle.ContainsKey(channelId))
               {
                   var channel = m_marshaledChannelsByHandle[channelId];

                   if (channel.Published)
                   {
                       channel.Unpublish();
                   }

                   m_marshaledChannelsByHandle.Remove(channelId);
                   m_marshaledChannelsByIdentifier.Remove(channel.Identifier);
               }
           }
        }

        private ProcessChannel GetChannel(int channelId)
        {
            ProcessChannel channel = null;

            lock (m_marshaledChannelsLockObject)
            {
                if (!m_marshaledChannelsByHandle.TryGetValue(channelId, out channel))
                {
                    return null;
                }
            }
            return channel;

        }


        private object m_pendingRequestsLock = new object();
        private Dictionary<int, List<string>> m_pendingRequests = new Dictionary<int, List<string>>();
        private List<Thread> m_workingThreads = new List<Thread>(); 

        private void PerformRequests()
        {
            while(true)
            {
                int channelId = 0;
                string key = "";
                lock (m_pendingRequestsLock)
                {
                    if (m_pendingRequests.Count == 0)
                    {
                        m_workingThreads.Remove(System.Threading.Thread.CurrentThread);
                        return;
                    }
                    foreach(var requests in m_pendingRequests)
                    {
                        channelId = requests.Key;
                        key = requests.Value[0];
                        requests.Value.RemoveAt(0);
                        if (requests.Value.Count==0)
                        {
                            m_pendingRequests.Remove(requests.Key);
                        }
                        break;
                    }
                }


                try
                {
                    var channel = GetChannel(channelId);

                    if (channel == null)
                    {
                        this.keyProvidedFailed(channelId, key);
                    }
                    else
                    {
                        var result = channel.GetKey(key, ProcessChannelGeyKeyOptions.FromRemoteNodes);
                        this.keyProvided(channelId, key, Convert.ToBase64String(result));
                    }
                }
                catch
                {
                    this.keyProvidedFailed(channelId, key);
                }
            }
        }

        protected override void requestKey(int channelId, string key)
        {
            var channel = GetChannel(channelId);

            if (channel != null && channel.FoundRemotely())
            {
                var request = channel.GetKeyAsync(key, ProcessChannelGeyKeyOptions.FromRemoteNodes);

                request.OnRequestedCompleted(
                    delegate
                        {
                            if (request.HasError)
                            {
                                this.keyProvidedFailed(channelId, key);
                            }
                            else
                            {
                                this.keyProvided(channelId, key, Convert.ToBase64String(request.Value));
                            }
                        });
            }
            else
            {
                this.keyProvidedFailed(channelId,key);
            }
        }

        protected override void cancelRequestKey(int channelId, string key)
        {
            //Do nothing right now...
        }

        protected override void addLocalProvider(int channelId, PYXNETChannelKeyProvider_SPtr provider)
        {
            var channel = GetChannel(channelId);

            if (channel != null)
            {
                channel.AttachKeyProvider(new UnmanagedProvider(this,new PYXNETChannelKeyProvider_SPtr(provider)),ProcessChannelGeyKeyOptions.FromLocalNode );
            }
        }

        protected override void publishChannel(int channelId)
        {
            var channel = GetChannel(channelId);

            if (channel != null)
            {
                channel.Publish();
            }
        }

        protected override void unpublishChannel(int channelId)
        {
            var channel = GetChannel(channelId);

            if (channel != null)
            {
                channel.Unpublish();
            }
        }

        #endregion

        #region Static memebers

        public static PyxlibPyxnetChannelProvider Instance
        {
            get
            {
                if (m_instance == null)
                {
                    m_instance = new PyxlibPyxnetChannelProvider();
                }

                return m_instance;
            }
        }

        public static void StartServer()
        {
            PYXNETChannelProvider.setInstance(new PYXNETChannelProvider_SPtr(Instance));
        }

        public static void StopServer()
        {
            if (m_instance != null)
            {
                PYXNETChannelProvider.setInstance(new PYXNETChannelProvider_SPtr());
                m_instance = null;
            }
        }

        protected static PyxlibPyxnetChannelProvider m_instance = null;

        #endregion

        #region PYXObject Lifetime Management

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.release(this);
        }

        #endregion

        /// <summary>
        /// Protected Default constructor - force it to create only one instance
        /// </summary>
        private PyxlibPyxnetChannelProvider()
        {
        }

        #endregion PYXObject Lifetime Management
    }
}
