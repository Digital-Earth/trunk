using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// Utility class to help find services.
    /// TODO: Make this class support multiple results, especially when a
    /// service is no longer reachable.
    /// </summary>
    public class ServiceFinder
    {
        private readonly Stack m_stack;

        /// <summary>
        /// Gets the stack.
        /// </summary>
        /// <value>The stack.</value>
        private Stack Stack
        {
            get { return m_stack; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ServiceFinder"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        public ServiceFinder(Stack stack)
        {
            m_stack = stack;
        }

		/// <summary>
		/// Finds the server for the service.
		/// </summary>
		/// <param name="service">The service to find the server for.</param>
		/// <param name="timeout">The time-out.</param>
		/// <returns>The node info of the server.</returns>
        private NodeInfo FindService(ServiceInstance service, TimeSpan timeout)
        {
            try
            {
	            return NodeInfo.Find(m_stack, service.Server, timeout);
			}
            catch (TimeoutException)
            {
            	m_stack.Tracer.WriteLine("Timed out when trying to find {0}", service.Server.ToString());
            }
            return null;
        }

        /// <summary>
        /// Finds the service (queries network).
        /// </summary>
        /// <param name="service">The service ID.</param>
        /// <param name="timeout">The timeout.</param>
        /// <returns>The service instance, or null if not found.</returns>
        // TODO: Make this return multiple services and move validation up to client
        public ServiceInstance FindService(ServiceId service, TimeSpan timeout)
        {
            NodeInfo nodeInfo;
            return FindService(service, timeout, out nodeInfo);
        }

        /// <summary>
        /// Finds the service (queries network).
        /// </summary>
        /// <param name="service">The service ID.</param>
        /// <param name="timeout">The timeout.</param>
        /// <param name="nodeInfo">The node info of the server, or null.</param>
        /// <returns>The service instance, or null if not found.</returns>
        // TODO: Make this return multiple services and move validation up to client
        public ServiceInstance FindService(ServiceId service, TimeSpan timeout, out NodeInfo nodeInfo)
        {
            ServiceInstance result = null;
            NodeInfo node = null;

            foreach (ServiceInstanceFact fact in Stack.CertificateRepository[service])
            {
                nodeInfo = FindService(fact.ServiceInstance, timeout);
                if (nodeInfo != null)
                {
                    return fact.ServiceInstance;
                }
            }

            // TODO: Figure out what we can do to do "any subservice" queries.
            string queryString = service.ToSearchString();

            m_stack.Tracer.DebugWriteLine("Querying for {0}.", queryString);

            Pyxis.Utilities.SynchronizationEvent querying =
                new Pyxis.Utilities.SynchronizationEvent(timeout); 

            Querier querier = new Querier(this.Stack, queryString, 1000);

            querier.Result +=
                delegate(object sender, Querier.ResultEventArgs args)
                {
                    Certificate c = new Certificate(
                        new MessageReader(args.QueryResult.ExtraInfo));
                    if (c.Valid && (c.ServiceInstance != null))
                    {
                        node = FindService(c.ServiceInstance, timeout);
                        if (node != null)
                        {
                            result = c.ServiceInstance;
                            Stack.CertificateRepository.Add(c);

                            querier.Stop();
                            querying.Pulse();
                        }
                    }
                };

            querier.Start();

            querying.Wait();

            nodeInfo = node;
            return result;
        }
    }    
}
