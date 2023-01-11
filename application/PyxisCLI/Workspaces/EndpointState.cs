using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Workspaces;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Workspaces
{
    internal class EndpointState
    {
        public Endpoint Endpoint { get; private set; }
        
        public UrlDiscoveryReport DiscoveryReport { get; private set; }
        public List<DataSet> DataSets { get; private set; }

        private Task<UrlDiscoveryReport> m_discoveryTask;

        public EndpointState(Endpoint endpoint)
        {
            Endpoint = endpoint;

            var data = LocalPersistance.AttachData(Endpoint);
            DiscoveryReport = data.Get<UrlDiscoveryReport>("status");
        }

        public Task<UrlDiscoveryReport> Discover(bool forceNewDiscovery = false)
        {
            if (m_discoveryTask != null && m_discoveryTask.IsCompleted)
            {
                if (forceNewDiscovery)
                {
                    m_discoveryTask = null;
                }
            }

            if (m_discoveryTask == null)
            {
                m_discoveryTask = Task<UrlDiscoveryReport>.Factory.StartNew(() =>
                {
                    var data = LocalPersistance.AttachData(Endpoint);
                    var discoveryReport = data.Get<UrlDiscoveryReport>("status");

                    var expectedCount = 0;
                    if (discoveryReport != null)
                    {
                        expectedCount = discoveryReport.DataSetCount;
                        discoveryReport.Status = "discovering";
                    }
                    else
                    {
                        discoveryReport = new UrlDiscoveryReport()
                        {
                            Uri = Endpoint.Uri,
                            Host = new Uri(Endpoint.Uri).Host,
                            Status = "discovering",
                            DataSetCount = 0,
                            VerifiedDataSetCount = 0,
                            BrokenDataSetCount = 0,
                            UnknownDataSetCount = 0,
                            LastDiscovered = DateTime.UtcNow,
                        };
                    }

                    DiscoveryReport = discoveryReport;

                    var discoverySummary = LocalGazetteer.DiscoverRecursively(Program.Engine, Endpoint.Uri,
                        expectedCount);

                    discoveryReport = new UrlDiscoveryReport()
                    {
                        Uri = Endpoint.Uri,
                        Host = new Uri(Endpoint.Uri).Host,
                        Status = "discovered",
                        ServiceIdentifier = discoverySummary.ServiceIdentifier,
                        DataSetCount = discoverySummary.DataSets.Count,
                        VerifiedDataSetCount = 0,
                        BrokenDataSetCount = 0,
                        UnknownDataSetCount = 0,
                        LastDiscovered = DateTime.UtcNow
                    };

                    data.Set("status", discoveryReport);
                    data.Set("datasets", discoverySummary.DataSets);

                    DiscoveryReport = discoveryReport;
                    DataSets = discoverySummary.DataSets;

                    return DiscoveryReport;
                });
            }

            return m_discoveryTask;
        }

        public List<DataSet> Search(string query)
        {
            if (DataSets == null)
            {
                var data = LocalPersistance.AttachData(Endpoint);
                DataSets = data.Get<List<DataSet>>("datasets");   
            }

            return DataSets.Where(dataSet => dataSet.Metadata.Name.IndexOf(query,StringComparison.InvariantCultureIgnoreCase) != -1).ToList();
        }
    }
}
