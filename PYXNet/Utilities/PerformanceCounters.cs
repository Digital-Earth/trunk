/******************************************************************************
PerformanceCounters.cs

begin      : September 09, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace PyxNet.Utilities
{
    /// <summary>
    /// A single performance counter.
    /// This class is used to insulate the client code from the fact that 
    /// the user running the app might not have the rights they need to create
    /// performance counters, and thus the counters might not exist.
    /// </summary>
    public class PyxPerformanceCounter
    {
        private readonly PerformanceCounter m_counter;

        private long m_value;

        public PyxPerformanceCounter()
        {
            m_counter = null;
            m_value = 0;
        }

        public PyxPerformanceCounter(PerformanceCounter counter)
        {
            m_counter = counter;
            RawValue = 0;
        }

        /// <summary>
        /// Increment the performance counter.
        /// </summary>
        public void Increment()
        {
            ++m_value;
            if (m_counter != null)
            {
                m_counter.Increment();
            }
        }

        /// <summary>
        /// Increment the performance counter by an amount.
        /// </summary>
        /// <param name="value">The value to increment by.</param>
        public void IncrementBy(long value)
        {
            m_value += value;
            if (m_counter != null)
            {
                m_counter.IncrementBy(value);
            }
        }

        /// <summary>
        /// Decrement the performance counter.
        /// </summary>
        public void Decrement()
        {
            --m_value;
            if (m_counter != null)
            {
                m_counter.Decrement();
            }
        }

        /// <summary>
        /// The raw value of the performance counter.
        /// </summary>
        public long RawValue
        {
            get
            {
                if (m_counter != null)
                {
                    return m_counter.RawValue;
                }
                return m_value;
            }
            set
            {
                m_value = value;
                if (m_counter != null)
                {
                    m_counter.RawValue = value;
                }
            }
        }
    }

    /// <summary>
    /// Wraps up performance counters for the global application.  
    /// </summary>
    public class GlobalPerformanceCounters
    {
        private bool m_performanceCountersInitialized = false;
        private readonly object m_performanceCountersInitializedLock = new object();
        private const string s_performanceCounterCategoryName = "PyxNet";

        private const string s_name1 = "Certificates Validated";
        private const string s_help1 = "The number of certificates that PyxNet has validated.";
        private const string s_name2 = "Coverage Data Uploaded";
        private const string s_help2 = "The number of bytes of coverage data that have been sent to PyxNet.";
        private const string s_name3 = "Data Downloaded";
        private const string s_help3 = "The number of bytes of data that have been received from PyxNet.";
        private const string s_name4 = "Coverage Data Tiles Downloaded";
        private const string s_help4 = "The number of tiles of coverage data that have been received from PyxNet.";
        private const string s_name5 = "File Data Uploaded";
        private const string s_help5 = "The number of bytes of file data that have been sent to PyxNet.";
        private const string s_name6 = "Milliseconds for an XML deserialize.";
        private const string s_help6 = "The number of milliseconds to deserialize an XML message.";

        private PyxPerformanceCounter m_certificatesValidated = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the certificates validated performance counter.
        /// </summary>
        public PyxPerformanceCounter CertificatesValidated
        {
            get
            {
                InitializePerformanceCounters();
                return m_certificatesValidated;
            }
        }

        private PyxPerformanceCounter m_coverageBytesUploaded = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the coverage bytes uploaded performance counter.
        /// </summary>
        public PyxPerformanceCounter CoverageBytesUploaded
        {
            get
            {
                InitializePerformanceCounters();
                return m_coverageBytesUploaded;
            }
        }

        private PyxPerformanceCounter m_dataBytesDownloaded = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the data bytes downloaded performance counter.
        /// </summary>
        public PyxPerformanceCounter DataBytesDownloaded
        {
            get
            {
                InitializePerformanceCounters();
                return m_dataBytesDownloaded;
            }
        }

        private PyxPerformanceCounter m_coverageTilesDownloaded = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the coverage tiles downloaded performance counter.
        /// </summary>
        public PyxPerformanceCounter CoverageTilesDownloaded
        {
            get
            {
                InitializePerformanceCounters();
                return m_coverageTilesDownloaded;
            }
        }

        private PyxPerformanceCounter m_fileBytesUploaded = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the file bytes uploaded performance counter.
        /// </summary>
        public PyxPerformanceCounter FileBytesUploaded
        {
            get
            {
                InitializePerformanceCounters();
                return m_fileBytesUploaded;
            }
        }

        private PyxPerformanceCounter m_millisecondsDeserializingXML = new PyxPerformanceCounter();

        /// <summary>
        /// Gets the milliseconds deserializing XML performance counter.
        /// </summary>
        public PyxPerformanceCounter MillisecondsDeserializingXML
        {
            get
            {
                InitializePerformanceCounters();
                return m_millisecondsDeserializingXML;
            }
        }

        /// <summary>
        /// Helper function to safely initialize.
        /// </summary>
        private void InitializePerformanceCounters()
        {
            if (!m_performanceCountersInitialized) lock (m_performanceCountersInitializedLock)
            {
                if (!m_performanceCountersInitialized)
                {
                    m_performanceCountersInitialized = true;
                    try
                    {
                        if (PerformanceCounterCategory.Exists(s_performanceCounterCategoryName))
                        {
                            try
                            {
                                // This delete is optional.
                                PerformanceCounterCategory.Delete(s_performanceCounterCategoryName);
                            }
                            catch (Exception exception)
                            {
                                Trace.WriteLine(
                                    "Ignoring exception when deleting performance counter category: {0}",
                                    exception.ToString());
                            }
                        }

                        CounterCreationDataCollection CCDC = new CounterCreationDataCollection();
                        CCDC.Add(new CounterCreationData(s_name1, s_help1, PerformanceCounterType.NumberOfItems32));
                        CCDC.Add(new CounterCreationData(s_name2, s_help2, PerformanceCounterType.NumberOfItems64));
                        CCDC.Add(new CounterCreationData(s_name3, s_help3, PerformanceCounterType.NumberOfItems64));
                        CCDC.Add(new CounterCreationData(s_name4, s_help4, PerformanceCounterType.NumberOfItems32));
                        CCDC.Add(new CounterCreationData(s_name5, s_help5, PerformanceCounterType.NumberOfItems64));
                        CCDC.Add(new CounterCreationData(s_name6, s_help6, PerformanceCounterType.NumberOfItems32));

                        // Create the category.
                        PerformanceCounterCategory.Create(s_performanceCounterCategoryName,
                            "Monitors the performance of PYXIS WorldView's PyxNet P2P Subsystem.",
                            PerformanceCounterCategoryType.SingleInstance,
                            CCDC);

                        // Create the performance counter, initialized to 0.
                        // Create the performance counters
                        PerformanceCounter c1 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name1, false);
                        m_certificatesValidated = new PyxPerformanceCounter(c1);
                        PerformanceCounter c2 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name2, false);
                        m_coverageBytesUploaded = new PyxPerformanceCounter(c2);
                        PerformanceCounter c3 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name3, false);
                        m_dataBytesDownloaded = new PyxPerformanceCounter(c3);
                        PerformanceCounter c4 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name4, false);
                        m_coverageTilesDownloaded = new PyxPerformanceCounter(c4);
                        PerformanceCounter c5 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name5, false);
                        m_fileBytesUploaded = new PyxPerformanceCounter(c5);
                        PerformanceCounter c6 =
                            new PerformanceCounter(s_performanceCounterCategoryName, s_name6, false);
                        m_millisecondsDeserializingXML = new PyxPerformanceCounter(c6);
                    }
                    catch (System.Security.SecurityException)
                    {
                        System.Diagnostics.Trace.WriteLine("Could not create global performance counters (SecurityException).");
                    }
                    catch (System.ComponentModel.Win32Exception)
                    {
                        System.Diagnostics.Trace.WriteLine("Could not create global performance counters (Win32Exception).");
                    }
                    catch (System.Exception e)
                    {
                        System.Diagnostics.Trace.WriteLine("Could not create global performance counters (Exception).  " + 
                            e.Message);
                    }
                }
            }
        }

        /// <summary>
        /// The performance counters, used to monitor the global application.
        /// </summary>
        static public GlobalPerformanceCounters Counters = new GlobalPerformanceCounters();
    }

    /// <summary>
    /// Wraps up all of the stacks performance counters.
    /// </summary>
    public class PyxStackPerformanceCounters
    {
        private bool m_performanceCountersInitialized = false;
        private readonly object m_performanceCountersInitializedLock = new object();
        private const string s_performanceCounterCategoryName = "PyxNet: Communication Stacks";

        private const string s_name1 = "Permanent Connections";
        private const string s_help1 = "The number of PyxNet nodes permanently connected.";
        private const string s_name2 = "Temporary Connections";
        private const string s_help2 = "The number of PyxNet nodes temporarily connected.";
        private const string s_name3 = "Volatile Connections";
        private const string s_help3 = "The number of PyxNet nodes waiting to complete a connection.";
        private const string s_name4 = "Queries Received";
        private const string s_help4 = "The number of PyxNet queries that have been received.";
        private const string s_name5 = "Queries Matched";
        private const string s_help5 = "The number of PyxNet queries that matched something on this stack.";
        private const string s_name6 = "Query Results Returned";
        private const string s_help6 = "The number of PyxNet queries results that have been sent.";

        private readonly string m_instanceName;

        public PyxStackPerformanceCounters(string InstanceName)
        {
            m_instanceName = InstanceName;
        }

        #region Counters
        private PyxPerformanceCounter m_permanentConnections = new PyxPerformanceCounter();

        public PyxPerformanceCounter PermanentConnections
        {
            get
            {
                InitializePerformanceCounters();
                return m_permanentConnections;
            }
        }

        private PyxPerformanceCounter m_temporaryConnections = new PyxPerformanceCounter();

        public PyxPerformanceCounter TemporaryConnections
        {
            get
            {
                InitializePerformanceCounters();
                return m_temporaryConnections;
            }
        }

        private PyxPerformanceCounter m_volatileConnections = new PyxPerformanceCounter();

        public PyxPerformanceCounter VolatileConnections
        {
            get
            {
                InitializePerformanceCounters();
                return m_volatileConnections;
            }
        }

        private PyxPerformanceCounter m_queriesReceived = new PyxPerformanceCounter();

        public PyxPerformanceCounter QueriesReceived
        {
            get
            {
                InitializePerformanceCounters();
                return m_queriesReceived;
            }
        }

        private PyxPerformanceCounter m_queriesMatched = new PyxPerformanceCounter();

        public PyxPerformanceCounter QueriesMatched
        {
            get
            {
                InitializePerformanceCounters();
                return m_queriesMatched;
            }
        }

        private PyxPerformanceCounter m_queryResultsSent = new PyxPerformanceCounter();

        public PyxPerformanceCounter QueryResultsSent
        {
            get
            {
                InitializePerformanceCounters();
                return m_queryResultsSent;
            }
        }
        #endregion

        /// <summary>
        /// Helper function to safely initialize.
        /// </summary>
        private void InitializePerformanceCounters()
        {
            if (!m_performanceCountersInitialized)
            {
                lock (m_performanceCountersInitializedLock)
                {
                    if (!m_performanceCountersInitialized)
                    {
                        m_performanceCountersInitialized = true;
                        try
                        {
                            for (int tryCount = 1; tryCount <= 2; tryCount++)
                            {
                                // Although this code seems like it is in the reverse order, the best way to tell if
                                // we need to creat the PerformanceCounterCategory is to try and  and create all of
                                // our PerformanceCounters, and then if that fails, delete the old 
                                // PerformanceCounterCategory (if needed) and create the new one, and then come back to
                                // the top of the loop to create the PerformanceCounters.
                                try
                                {
                                    // Create the performance counters
                                    PerformanceCounter c1 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name1, m_instanceName, false);
                                    m_permanentConnections = new PyxPerformanceCounter(c1);
                                    PerformanceCounter c2 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name2, m_instanceName, false);
                                    m_temporaryConnections = new PyxPerformanceCounter(c2);
                                    PerformanceCounter c3 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name3, m_instanceName, false);
                                    m_volatileConnections = new PyxPerformanceCounter(c3);
                                    PerformanceCounter c4 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name4, m_instanceName, false);
                                    m_queriesReceived = new PyxPerformanceCounter(c4);
                                    PerformanceCounter c5 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name5, m_instanceName, false);
                                    m_queriesMatched = new PyxPerformanceCounter(c5);
                                    PerformanceCounter c6 =
                                        new PerformanceCounter(s_performanceCounterCategoryName, s_name6, m_instanceName, false);
                                    m_queryResultsSent = new PyxPerformanceCounter(c6);

                                    // If we get here with no exceptions then everything is initialized.
                                    return;
                                }
                                catch
                                {
                                }

                                if (tryCount == 1)
                                {
                                    if (PerformanceCounterCategory.Exists(s_performanceCounterCategoryName))
                                    {
                                        PerformanceCounterCategory.Delete(s_performanceCounterCategoryName);
                                    }

                                    CounterCreationDataCollection CCDC = new CounterCreationDataCollection();
                                    CCDC.Add(new CounterCreationData(s_name1, s_help1, PerformanceCounterType.NumberOfItems32));
                                    CCDC.Add(new CounterCreationData(s_name2, s_help2, PerformanceCounterType.NumberOfItems32));
                                    CCDC.Add(new CounterCreationData(s_name3, s_help3, PerformanceCounterType.NumberOfItems32));
                                    CCDC.Add(new CounterCreationData(s_name4, s_help4, PerformanceCounterType.NumberOfItems32));
                                    CCDC.Add(new CounterCreationData(s_name5, s_help5, PerformanceCounterType.NumberOfItems32));
                                    CCDC.Add(new CounterCreationData(s_name6, s_help6, PerformanceCounterType.NumberOfItems32));

                                    // Create the category.
                                    PerformanceCounterCategory.Create(s_performanceCounterCategoryName,
                                        "Monitors the performance of PYXIS WorldView's PyxNet P2P Comunication Stack.",
                                        PerformanceCounterCategoryType.MultiInstance,
                                        CCDC);
                                }
                            }

                        }
                        catch (System.Security.SecurityException)
                        {
                            System.Diagnostics.Trace.WriteLine(
                                String.Format("Could not create {0} performance counters (SecurityException).", m_instanceName));
                        }
                        catch (System.ComponentModel.Win32Exception)
                        {
                            System.Diagnostics.Trace.WriteLine(
                                String.Format("Could not create {0} performance counters (Win32Exception).", m_instanceName));
                        }
                    }
                }
            }
        }
    }
}
