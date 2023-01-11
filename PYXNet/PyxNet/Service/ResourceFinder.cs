using System;
using System.Collections.Generic;
using System.Text;

using PyxNet.Service;

namespace PyxNet.Service
{
    /// <summary>
    /// Utility class to find a resource by name.
    /// </summary>
    public class ResourceFinder
    {
        private readonly PyxNet.Stack m_stack;

        private readonly string m_resourceName;

        private readonly Pyxis.Utilities.DynamicList<ResourceInstanceFact> m_results =
            new Pyxis.Utilities.DynamicList<ResourceInstanceFact>();

        public IList<ResourceInstanceFact> Results
        {
            get { return m_results; }
        }

        #region FactFound Event

        /// <summary> EventArgs for a FactFound event. </summary>    
        public class FactFoundEventArgs : EventArgs
        {
            private ResourceInstanceFact m_ResourceInstanceFact;

            /// <summary>The ResourceInstanceFact.</summary>
            public ResourceInstanceFact ResourceInstanceFact
            {
                get { return m_ResourceInstanceFact; }
                set { m_ResourceInstanceFact = value; }
            }

            internal FactFoundEventArgs(ResourceInstanceFact theResourceInstanceFact)
            {
                m_ResourceInstanceFact = theResourceInstanceFact;
            }
        }

        /// <summary> Event handler for FactFound. </summary>
        public event EventHandler<FactFoundEventArgs> FactFound
        {
            add
            {
                m_FactFound.Add(value);
            }
            remove
            {
                m_FactFound.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<FactFoundEventArgs> m_FactFound = new Pyxis.Utilities.EventHelper<FactFoundEventArgs>();

        /// <summary>
        /// Raises the FactFound event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theResourceInstanceFact"></param>
        private void OnFactFound(ResourceInstanceFact theResourceInstanceFact)
        {
            // Note that this next line is unusual in an OnBlah function.
            if (m_results.Add(theResourceInstanceFact, false))
            {
                m_FactFound.Invoke(this, new FactFoundEventArgs(theResourceInstanceFact));
            }
        }

        #endregion FactFound Event

        #region RemoteQueryStopped Event

        /// <summary> EventArgs for a RemoteQueryStopped event. </summary>    
        public class RemoteQueryStoppedEventArgs : EventArgs
        {
        }

        /// <summary> Event handler for RemoteQueryStopped. </summary>
        public event EventHandler<RemoteQueryStoppedEventArgs> RemoteQueryStopped
        {
            add
            {
                m_RemoteQueryStopped.Add(value);
            }
            remove
            {
                m_RemoteQueryStopped.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<RemoteQueryStoppedEventArgs> m_RemoteQueryStopped = new Pyxis.Utilities.EventHelper<RemoteQueryStoppedEventArgs>();

        /// <summary>
        /// Raises the RemoteQueryStopped event.
        /// </summary>
        private void OnRemoteQueryStopped()
        {
            m_RemoteQueryStopped.Invoke( this, new RemoteQueryStoppedEventArgs());
        }

        #endregion RemoteQueryStopped Event

        public ResourceFinder(PyxNet.Stack stack, string resourceName)
        {
            m_stack = stack;
            m_resourceName = resourceName;
        }

        /// <summary>
        /// Examines the certificate fact, and adds it to the results if it is valid.
        /// </summary>
        /// <param name="resourceInstanceFact">The new resource instance fact.</param>
        private void ExamineCertificateFact(ResourceInstanceFact resourceInstanceFact)
        {
            if (IsNewMatchingFact(resourceInstanceFact))
            {
                OnFactFound(resourceInstanceFact);
            }
        }

        /// <summary>
        /// Examines the resource instance fact, and returns true if it is valid and not already found.
        /// </summary>
        /// <param name="resourceInstanceFact">The new resource instance fact.</param>
        /// <returns>True if it is a new fact we are looking for.</returns>
        private bool IsNewMatchingFact(ResourceInstanceFact resourceInstanceFact)
        {
            // If the resource instance fact's file name is a match, it's a match.
            if ((resourceInstanceFact != null) &&
                (resourceInstanceFact.ManifestEntry.FileName == m_resourceName))
            {
                // If the resource instance fact isn't in the results list, it's a new one.
                ResourceInstanceFact found = m_results.Find(
                    delegate(ResourceInstanceFact result)
                    {
                        return result.Id == resourceInstanceFact.Id;
                    });
                return null == found;
            }
            return false;
        }

        /// <summary>
        /// Examines the certificate, and adds it to the results if it is valid.
        /// </summary>
        /// <param name="newCertificate">The new certificate.</param>
        private void ExamineCertificate(Certificate newCertificate)
        {
            foreach (ICertifiableFact fact in newCertificate.Facts)
            {
                ExamineCertificateFact(fact as ResourceInstanceFact);
            }
        }

        /// <summary>
        /// Finds any matching resources from the local repository.
        /// </summary>
        /// <returns>All facts found.</returns>
        public IList<ResourceInstanceFact> FindLocal()
        {
            // Check to see what's in the system repository...
            foreach (ResourceInstanceFact resourceInstanceFact in
                m_stack.CertificateRepository.GetResourceInstanceFacts(IsNewMatchingFact))
            {
                OnFactFound(resourceInstanceFact);
            }

            return m_results;
        }

        private Pyxis.Utilities.DeadManTimer m_queryHeartbeat;
        private volatile Querier m_querier;

        /// <summary>
        /// This locks m_querier and m_queryHeartBeat.
        /// </summary>
        private readonly Object m_lockQuerier = new Object();

        /// <summary>
        /// Starts the remote query.
        /// </summary>
        /// <returns>False if already started.</returns>
        public bool StartRemoteQuery()
        {
            if (m_querier == null)
            {
                // Search for the resource by name.
                // TODO: Use ResourceInstanceFact's Unique Keyword to generate this string.
                String queryString = String.Format("RESN:{0}", m_resourceName);

                // Set up the querier.
                Querier querier = new PyxNet.Querier(m_stack, queryString, 1000);
                querier.Result +=
                    delegate(object sender, Querier.ResultEventArgs args)
                    {
                        System.Threading.ThreadPool.QueueUserWorkItem(
                            delegate(Object unused)
                            {
                                try
                                {
                                    m_stack.Tracer.DebugWriteLine("QueryResult.ExtraInfo.Length = {0}", 
                                        args.QueryResult.ExtraInfo.Length);

                                    if (args.QueryResult.ExtraInfo.Length > 0)
                                    {
                                        Certificate certificate = new Certificate(new MessageReader(
                                            args.QueryResult.ExtraInfo));
                                        if (certificate.Valid)
                                        {
                                            ExamineCertificate(certificate);
                                            m_stack.CertificateRepository.Add(certificate);
                                        }
                                        else
                                        {
                                            m_stack.Tracer.WriteLine("Ignoring an expired certificate for query '{0}'.",
                                                this.m_querier.Query.Contents);
                                        }

                                        Pyxis.Utilities.DeadManTimer queryHeartbeat = m_queryHeartbeat;
                                        if (queryHeartbeat != null)
                                        {
                                            queryHeartbeat.KeepAlive();
                                        }
                                    }
                                    else
                                    {
                                        m_stack.Tracer.WriteLine("QueryResult missing certificate.");
                                    }
                                }
                                catch (Exception ex)
                                {
                                    m_stack.Tracer.WriteLine("Internal failure. {0}", ex.Message);
                                }
                            });
                    };

                lock (m_lockQuerier)
                {
                    if (m_querier == null)
                    {
                        // Run the querier.
                        m_querier = querier;
                        m_querier.Start();
                        return true;
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Starts the remote query.
        /// </summary>
        /// <returns>False if already started.</returns>
        /// <param name="timeOut">The time out.</param>
        public bool StartRemoteQuery(TimeSpan timeOut)
        {
            lock (m_lockQuerier)
            {
                if (null != m_queryHeartbeat)
                {
                    m_queryHeartbeat.Stop();
                }
                m_queryHeartbeat = new Pyxis.Utilities.DeadManTimer(
                    timeOut,
                    delegate(object sender, System.Timers.ElapsedEventArgs e)
                    {
                        StopRemoteQuery();
                    });

	            return StartRemoteQuery();
	         }
        }

        /// <summary>
        /// Waits for results.
        /// </summary>
        /// <param name="minResults">The minimum number of results.</param>
        /// <param name="timeout">The max time to wait.</param>
        /// <returns>True iff there were enough results found.</returns>
        public bool WaitForResults(int minResults, TimeSpan timeout)
        {
            Pyxis.Utilities.SynchronizationEvent waiting = new Pyxis.Utilities.SynchronizationEvent();

            EventHandler<RemoteQueryStoppedEventArgs> stoppedHandler = delegate(object caller, ResourceFinder.RemoteQueryStoppedEventArgs args)
            {
                waiting.Pulse();
            };
            this.RemoteQueryStopped += stoppedHandler;

            EventHandler < FactFoundEventArgs > factFoundHandler =
            delegate(object caller, ResourceFinder.FactFoundEventArgs args)
            {
                if (m_results.Count >= minResults)
                {
                    waiting.Pulse();
                }
            };
            this.FactFound += factFoundHandler;

            StartRemoteQuery(timeout);

            if (m_results.Count < minResults)
            {
                waiting.Wait();
            }

            this.RemoteQueryStopped -= stoppedHandler;
            this.FactFound -= factFoundHandler;
            return (m_results.Count > -minResults);
        }

        /// <summary>
        /// Stops the remote query.
        /// </summary>
        /// <returns>False if already stopped.</returns>
        public bool StopRemoteQuery()
        {
            lock (m_lockQuerier)
            {
                if (m_queryHeartbeat != null)
                {
                    m_queryHeartbeat.Stop();
                    m_queryHeartbeat = null;
                }

                if (m_querier != null)
                {
                    m_querier.Stop();
                    m_querier = null;
                    OnRemoteQueryStopped();
                    return true;
                }
            }

            return false;
        }
    }

    // TODO: Write unit tests!
}
