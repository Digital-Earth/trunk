using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Core;

namespace Pyxis.IO.DataDiscovery
{
    internal class DiscoveryTask
    {
        private class DiscoveryContext : IDiscoveryContext
        {
            private readonly DiscoveryTask m_task;

            public DiscoveryContext(DiscoveryTask discoveryTask)
            {
                m_task = discoveryTask;
            }

            public DiscoveryRequest Request
            {
                get { return m_task.m_request; }
            }

            public Task<IDiscoveryNetworkResult> SendRequestAsync(IDiscoveryNetworkRequest request)
            {
                return m_task.DoRequestAsync(request);
            }
        }

        private readonly DiscoveryRequest m_request;
        private readonly IDiscoveryContext m_context;
        private readonly DiscoveryResult m_result;
        private readonly object m_lock = new object();
        private readonly Dictionary<IDiscoveryNetworkRequest, Task<IDiscoveryNetworkResult>> m_networkRequests = new Dictionary<IDiscoveryNetworkRequest, Task<IDiscoveryNetworkResult>>();

        private Engine m_engine;

        private static readonly List<IDiscoveryService> s_discoveryServices = new List<IDiscoveryService>();
        private readonly List<IDiscoveryService> m_discoveryServices;

        public static void RegisterDiscoverService(IDiscoveryService service)
        {
            s_discoveryServices.Add(service);
        }

        public static IDiscoveryService FindDiscoveryServiceForUri(string uri)
        {
            return s_discoveryServices.FirstOrDefault(service => service.IsUriSupported(uri));
        }

        public DiscoveryTask(Engine engine, string uri, IPermit permit = null, List<IDiscoveryService> discoveryServices = null)
            : this(engine, new DiscoveryRequest() { Uri = uri, Permit = permit }, discoveryServices)
        {
            
        }

        public DiscoveryTask(Engine engine, DiscoveryRequest request, List<IDiscoveryService> discoveryServices = null )
        {
            m_engine = engine;
            m_request = request;

            m_result = new DiscoveryResult()
            {
                Uri = request.Uri,
                Permit = request.Permit,
            };

            m_context = CreateContext();

            m_discoveryServices = new List<IDiscoveryService>( discoveryServices ?? s_discoveryServices );
        }

        public async Task<DiscoveryResult> GetResult()
        {
            var discoveryResults = await Task.WhenAll(m_discoveryServices.Select(x => x.DiscoverAsync(m_context)));
            foreach (var result in discoveryResults)
            {
                if (result == null) continue;

                //update results if found
                if (result.DataSet != null && m_result.DataSet == null)
                {
                    m_result.DataSet = result.DataSet;
                    m_result.Metadata = result.Metadata;
                    m_result.ServiceIdentifier = result.ServiceIdentifier;
                    m_result.LastUpdated = result.LastUpdated;
                }
                else if (result.DataSet != null && m_result.DataSet != null)
                {
                    m_result.AddDataSet(result.DataSet);
                }
                else if (result.Metadata != null && m_result.DataSet == null)
                {
                    m_result.Metadata = result.Metadata;
                    m_result.ServiceIdentifier = result.ServiceIdentifier;
                    m_result.LastUpdated = result.LastUpdated;
                }
                else if (result.ServiceIdentifier.HasContent() && !m_request.ServiceIdentifier.HasContent())
                {
                    m_result.ServiceIdentifier = result.ServiceIdentifier;
                    m_result.LastUpdated = result.LastUpdated;
                }

                //merge additional datasets 
                if (result.AdditionalDataSets != null)
                {
                    foreach (var dataset in result.AdditionalDataSets)
                    {
                        m_result.AddDataSet(dataset);
                    }
                }

                //merge leads
                if (result.Leads != null)
                {
                    foreach (var lead in result.Leads)
                    {
                        AddLead(lead);
                    }
                }

                //merge roots
                if (result.AdditionalRoots != null)
                {
                    foreach (var root in result.AdditionalRoots)
                    {
                        AddAdditionalRoot(root);
                    }
                }
            }
            return m_result;
        }

        private IDiscoveryContext CreateContext()
        {
            return new DiscoveryContext(this);
        }

        private void AddLead(DiscoveryRequest request)
        {
            lock (m_lock)
            {
                if (m_result.Leads == null)
                {
                    m_result.Leads = new List<DiscoveryRequest>();
                }

                if (!m_result.Leads.Contains(request))
                {
                    m_result.Leads.Add(request);
                }
            }
        }

        private void AddAdditionalRoot(DiscoveryRequest request)
        {
            lock (m_lock)
            {
                if (m_result.AdditionalRoots == null)
                {
                    m_result.AdditionalRoots = new List<DiscoveryRequest>();
                }

                if (!m_result.AdditionalRoots.Contains(request))
                {
                    m_result.AdditionalRoots.Add(request);
                }
            }
        }
        

        private Task<IDiscoveryNetworkResult> DoRequestAsync(IDiscoveryNetworkRequest request)
        {
            lock (m_lock)
            {
                Task<IDiscoveryNetworkResult> result;

                if (m_networkRequests.TryGetValue(request, out result))
                {
                    return result;
                }

                return m_networkRequests[request] = request.SendAsync();
            }
        }
    }
}
