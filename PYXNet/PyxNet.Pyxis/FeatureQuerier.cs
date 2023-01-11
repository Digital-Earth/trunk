/******************************************************************************
FeatureQuerier.cs

begin      : 26/08/2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using Pyxis.Utilities;
using PyxNet;
using System.Collections.Generic;

namespace PyxNet.Pyxis
{
    public class FeatureQuerier : IDisposable
    {
        #region Fields and Properties

        /// <summary>
        /// Dictionary mapping search string to a querier responsible for getting publishers.
        /// </summary>
        private readonly Dictionary<string, PyxNet.Querier> m_queriers =
            new Dictionary<string, PyxNet.Querier>();

        /// <summary>
        /// Providers that are serving a result for the query.
        /// </summary>
        private readonly
            Dictionary<PyxNet.DataHandling.DataGuid, Dictionary<int, List<PyxNet.QueryResult>>>
            m_providers =
            new Dictionary<PyxNet.DataHandling.DataGuid, Dictionary<int, List<PyxNet.QueryResult>>>();

        /// <summary>
        /// The search tokens for the query.
        /// </summary>
        private readonly List<String> m_searchTokens = new List<String>();

        /// <summary>
        /// Return true if a query is started.
        /// </summary>
        public bool Started
        {
            get
            {
                lock (m_searchTokens)
                {
                    return m_searchTokens.Count > 0;
                }
            }
        }

        #endregion

        /// <summary>
        /// Construct the object.
        /// </summary>
        public FeatureQuerier()
        {
        }

        #region Dispose

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    Stop();
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~FeatureQuerier()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        /// <summary>
        /// Start the query.
        /// </summary>
        /// <param name="searchTokens">The search strings to query for.</param>
        public void Start(PyxNet.Stack stack, IList<String> searchTokens)
        {
            // Stop existing queries.
            Stop();

            lock (m_providers)
            {
                m_providers.Clear();
            }

            // Start the searches.
            foreach (String searchToken in searchTokens)
            {
                bool containsKey;
                lock (m_queriers)
                {
                    containsKey = m_queriers.ContainsKey(searchToken);
                }

                // Ignores duplicates.
                if (!containsKey)
                {
                    // Add it to the list.
                    lock (m_searchTokens)
                    {
                        m_searchTokens.Add(searchToken);
                    }

                    // Create a querier.
                    PyxNet.Querier querier = 
                        new PyxNet.Querier(stack, searchToken, 1000);

                    // Add it to the dictionary.
                    lock (m_queriers)
                    {
                        m_queriers.Add(searchToken, querier);
                    }

                    // Start it.
                    querier.Result += HandleResult;
                    querier.Start();
                }
            }

            System.Diagnostics.Debug.Assert(Started);
        }

        /// <summary>
        /// Stop the query.
        /// </summary>
        public void Stop()
        {
            lock (m_searchTokens)
            {
                m_searchTokens.Clear();
            }

            lock (m_queriers)
            {
                // Stop the searches and clear the dictionary.
                foreach (PyxNet.Querier querier in m_queriers.Values)
                {
                    querier.Stop();
                    querier.Result -= HandleResult;
                }
                m_queriers.Clear();
            }
        }

        /// <summary>
        /// Iterate through providers that are serving a result for the query,
        /// and perform an action on each in a thread-safe manner.
        /// </summary>
        public void ForEach(Action<KeyValuePair<PyxNet.DataHandling.DataGuid,
            Dictionary<int, List<PyxNet.QueryResult>>>> action)
        {
            lock (m_providers)
            {
                foreach (KeyValuePair<PyxNet.DataHandling.DataGuid,
                    Dictionary<int, List<PyxNet.QueryResult>>> pair in m_providers)
                {
                    action(pair);
                }
            }
        }

        #region Result Event

        /// <summary>
        /// Class which wraps a QueryResult object, and will be passed as the second argument to a ResultHandler.
        /// </summary>
        public class ResultEventArgs : Querier.ResultEventArgs
        {
            private readonly ProcRef m_procRef;

            public ProcRef ProcRef
            {
                get { return m_procRef; }
            }

            public ResultEventArgs(QueryResult result, ProcRef procRef) : base(result)
            {
                m_procRef = procRef;
            }
        }

        /// <summary>
        /// Event which is fired when a result comes in.
        /// </summary>
        public event EventHandler<ResultEventArgs> ResultsReceived
        {
            add
            {
                m_ResultsReceived.Add(value);
            }
            remove
            {
                m_ResultsReceived.Remove(value);
            }
        }
        private EventHelper<ResultEventArgs> m_ResultsReceived = new EventHelper<ResultEventArgs>();

        /// <summary>
        /// Method to safely raise event Result.
        /// </summary>
        protected void OnResultRaise(QueryResult queryResult, ProcRef procRef)
        {
            m_ResultsReceived.Invoke(this, new ResultEventArgs(queryResult, procRef));
        }

        #endregion Result Event

        // This handler is used to watch for query results coming in from PyxNet.
        private void HandleResult(object sender, PyxNet.Querier.ResultEventArgs args)
        {
            // Get the query result.
            PyxNet.QueryResult queryResult = args.QueryResult;

            // Make sure it contains all search terms.
            // TODO: Add boolean logic on the PyxNet node side.
            if (!queryResult.DoesQueryMatch(m_searchTokens))
            {
                return;
            }

            // Make sure it is a provider of a coverage process.
            if (null == queryResult.ExtraInfo ||
                queryResult.ExtraInfo.Identifier != PyxNet.Pyxis.FeatureDefinitionMessage.MessageID)
            {
                return;
            }

            PyxNet.Pyxis.FeatureDefinitionMessage extraInfo =
                new PyxNet.Pyxis.FeatureDefinitionMessage(queryResult.ExtraInfo);
            PyxNet.DataHandling.DataGuid id = queryResult.MatchingDataSetID;
            int version = extraInfo.ProcRef.getProcVersion();

            // Add it to our dictionary of providers.
            lock (m_providers)
            {
                // Try to get dictionary mapping process version to providers for this process ID.
                Dictionary<int, List<PyxNet.QueryResult>> providersForProcess;
                if (m_providers.TryGetValue(id, out providersForProcess))
                {
                    System.Diagnostics.Debug.Assert(null != providersForProcess,
                        "There should never be null values in the providers dictionary.");

                    List<PyxNet.QueryResult> providersForProcessVersion;
                    if (providersForProcess.TryGetValue(version, out providersForProcessVersion))
                    {
                        System.Diagnostics.Debug.Assert(null != providersForProcessVersion,
                            "There should never be null values in the providers dictionary.");

                        // Make sure this provider isn't already in the list.
                        PyxNet.QueryResult existingProvider = providersForProcessVersion.Find(
                            delegate(PyxNet.QueryResult element)
                            {
                                return element.ResultNode.Equals(queryResult.ResultNode);
                            });
                        if (null == existingProvider)
                        {
                            providersForProcessVersion.Add(queryResult);
                        }
                    }
                    else
                    {
                        // Create list of providers, and add the query result.
                        providersForProcessVersion = new List<PyxNet.QueryResult>();
                        providersForProcessVersion.Add(queryResult);

                        // Add this to the dictionary.
                        providersForProcess.Add(version, providersForProcessVersion);
                    }
                }
                else
                {
                    // Create list of providers, and add the query result.
                    List<PyxNet.QueryResult> providersForProcessVersion = new List<PyxNet.QueryResult>();
                    providersForProcessVersion.Add(queryResult);

                    // Create dictionary mapping process version to providers, and add the provider list.
                    providersForProcess = new Dictionary<int, List<PyxNet.QueryResult>>();
                    providersForProcess.Add(version, providersForProcessVersion);

                    // Add this to the dictionary.
                    m_providers.Add(id, providersForProcess);
                }
            }

            OnResultRaise(queryResult, extraInfo.ProcRef);
        }
    }
}

