using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Utilities.Shell;

namespace GeoStreamService
{
    internal static class ShellCommands
    {
        [ShellAction(Name = "node", Description = "Show node info")]
        static void ShowNodeInfo()
        {
            ShowNodeInfo(PyxNet.StackSingleton.Stack.NodeInfo);
        }

        private static void ShowNodeInfo(PyxNet.NodeInfo info)
        {
            Console.WriteLine("Node Info");
            Console.WriteLine("  Name: {0}", info.FriendlyName);
            Console.WriteLine("  ID: {0}", info.NodeId.ToString());
            Console.WriteLine("  Mode: {0}", info.Mode);
            var address = info.Address;
            Console.WriteLine("  Internal Addresses: {0}", String.Join(",", address.InternalIPEndPoints.Select(x => x.ToString())));
            Console.WriteLine("  External Addresses: {0}", String.Join(",", address.ExternalIPEndPoints.Select(x => x.ToString())));
        }

        [ShellAction(Name = "map", Description = "Show UPNP Mapping")]
        static void ShowUpnpMapping()
        {
            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            Console.WriteLine("UPNP Port Mappings:");
            int count = 0;
            int index = 1;
            foreach (var pinfo in pyxNatControl.getPortMappings(ref count))
            {
                Console.WriteLine(" {6}) {0}[{5}] External [{1}:{2}] -> Internal[{3}:{4}]",
                    pinfo.Description,
                    pinfo.ExternalIPAddress,
                    pinfo.ExternalPort,
                    pinfo.InternalHostName,
                    pinfo.InternalPort,
                    pinfo.Protocol,
                    index);
                index++;
            }
        }

        [ShellAction(Name = "delmap", Description = "Remove UPNP mapping")]
        static void RemoveUpnpMapping(int index)
        {
            //Console.Write("Port mapping index>");
            //int index = Int32.Parse(Console.ReadLine());

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            int count = 0;
            var pinfos = pyxNatControl.getPortMappings(ref count);
            var pinfo = pinfos[index - 1];

            pyxNatControl.RemovePortMapping(pinfo);
        }

        [ShellAction(Name = "stack", Description = "Stack status")]
        static void StackStatus()
        {
            var stack = PyxNet.StackSingleton.Stack;
            Console.WriteLine("IsHubConnected: {0}", stack.ConnectionManager.IsHubConnected);

            Console.ForegroundColor = ConsoleColor.Green;
            foreach (var node in stack.KnownHubList.ConnectedHubs)
            {
                Console.WriteLine("Conntected Hub: {0} - {1}", node.FriendlyName, node.NodeId);
            }
            Console.ResetColor();

            foreach (var node in stack.KnownHubList.KnownHubs)
            {
                Console.WriteLine("Known Hub: {0} - {1}", node.FriendlyName, node.NodeId);
            }

            foreach (var node in stack.AllKnownNodes.Values)
            {
                Console.WriteLine("Known Node: {0} - {1}", node.FriendlyName, node.NodeId);
            }
        }

        [ShellAction(Name = "pub|published", Description = "Published item status")]
        static void PublishedStatus()
        {
            var publisher = PyxNet.StackSingleton.Stack.Publisher;

            var publisherType = publisher.GetType();
            var publishedItemProp = publisherType.GetProperty("PublishedItems", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            var publishedItems = (Pyxis.Utilities.DynamicList<PyxNet.Publishing.Publisher.IPublishedItemInfo>)publishedItemProp.GetValue(publisher);

            foreach (var item in publishedItems)
            {
                Console.WriteLine(String.Format("{0} : keyWords({1})", item.GetType().Name, String.Join(", ", item.Keywords)));
            }
        }

        [ShellAction(Name = "qht", Description = "force an update of QueryHashTable")]
        static void ForceQueryHashTableUpdate()
        {
            PyxNet.StackSingleton.Stack.ForceQueryHashTableUpdate();
            Console.WriteLine("QueryHashTable been updated");
        }

        [ShellAction(Name = "find", Description = "Find Node over Pyxnet from node Guid")]
        static void FindNode(Guid nodeGuid, int seconds)
        {
            var nodeId = new PyxNet.NodeId(nodeGuid);
            var nodeInfo = PyxNet.NodeInfo.Find(PyxNet.StackSingleton.Stack, nodeId, TimeSpan.FromSeconds(seconds));

            if (nodeInfo == null)
            {
                Console.WriteLine("can't find node with node ID: {0}", nodeId);
            }
            ShowNodeInfo(nodeInfo);
        }

        [ShellAction(Name = "l|listen", Description = "Listen to messages over the PyxNet Stack")]
        static void Listen()
        {
            var stack = PyxNet.StackSingleton.Stack;
            stack.OnAnyMessage += TheStack_OnAnyMessage;
            stack.OnSendingMessageToConnection += TheStack_OnSendingMessageToConnection;

            Console.WriteLine("Listening to messages [press any key to stop]");
            while (!Console.KeyAvailable)
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(0.2));
            }

            stack.OnAnyMessage -= TheStack_OnAnyMessage;
            stack.OnSendingMessageToConnection -= TheStack_OnSendingMessageToConnection;
        }

        static void TheStack_OnSendingMessageToConnection(PyxNet.Stack stack, PyxNet.StackConnection connection, PyxNet.Message message)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine(" [SEND] {0} : MessageID: {1} Length: {2}",
                    connection.FriendlyName,
                    message.Identifier,
                    message.Length);
            Console.ResetColor();
        }

        static void TheStack_OnAnyMessage(object sender, PyxNet.Stack.AnyMessageEventArgs args)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(" [RECV] {0} : MessageID: {1} Length: {2}",
                    args.Connection.FriendlyName,
                    args.Message.Identifier,
                    args.Message.Length);
            Console.ResetColor();

            if (args.Message.Identifier == "Conn")
            {
                var request = new PyxNet.StackConnector(new PyxNet.MessageReader(args.Message));

                var property = typeof(PyxNet.StackConnector).GetProperty("ToNode", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
                var nodeInfo = property.GetValue(request) as PyxNet.NodeInfo;
                Console.WriteLine("  Connnect request to Node {0} {1}", nodeInfo.FriendlyName, nodeInfo.NodeId);
            }

        }

        [ShellAction(Name = "q|query", Description = "Search for a string over PyxNet")]
        static void Query(string queryString, int seconds)
        {
            DateTime startTime = DateTime.Now;
            var querier = new PyxNet.Querier(PyxNet.StackSingleton.Stack, queryString, 1000);

            bool queryExpired = false;

            querier.QueryTimeout = TimeSpan.FromSeconds(seconds);
            querier.QueryExpired += (sender, args) => { queryExpired = true; };
            querier.Result += (sender, args) =>
            {
                Console.WriteLine(" Result Return from {0}: {1}", args.QueryResult.ResultNode.FriendlyName, args.QueryResult.MatchingContents);
                startTime = DateTime.Now; //make sure we wait few more seconds before returning
            };
            querier.Start();

            while (!queryExpired && !Console.KeyAvailable && (DateTime.Now - startTime).TotalSeconds < seconds + 0.1)
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(0.2));
            }

            if (queryExpired)
            {
                Console.WriteLine("Query was expired");
            }

            querier.Stop();
        }

        [ShellAction(Name = "cs|cache-stats", Description = "Show the cache dir statics")]
        static void CacheStats()
        {
            double mb = 1024 * 1024;
            foreach (var pipeline in PipelineRepository.Instance.GetAllPublishedPipelines())
            {
                var cacheUsage = new CacheUsage(pipeline.ProcRef);
                var inputFiles = cacheUsage.CalculateInputFilesDiskUsage() / mb;
                var cacheSize = cacheUsage.CalculateCacheDiskSpaceUsage() / mb;
                Console.WriteLine(String.Format("{0} - \"{1}\":\n   Total: {2:0.0} MB = InputFiles: {3:0.0} MB + PyxCache: {4:0.0} MB", 
                                  pipeline.ProcRef, pipeline.Name, inputFiles+cacheSize, inputFiles, cacheSize));
            }
        }

        [ShellAction(Name = "cwd", Description = "Current Working Directory")]
        static void Cwd()
        {
            Console.WriteLine(Environment.CurrentDirectory);
        }
    }
}
