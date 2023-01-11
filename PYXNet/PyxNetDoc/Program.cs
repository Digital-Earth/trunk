using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Utilities.Shell;

namespace PyxNetDoc
{
    class Program
    {
        public static PyxNet.PyxNetStack TheStack { get; set; }

        static void Main(string[] args)
        {
            TheStack = PyxNet.StackSingleton.Stack;

            TheStack.NetworkAddressChanged += TheStack_NetworkAddressChanged;
            TheStack.Connected += TheStack_Connected;

            var shell = new ShellEngine();

            
            var exit = false;
            shell.AddAction("exit|quit", "exit program", () => { exit = true; });
            shell.AddActionsFromType(typeof(Program));            
            
            TheStack.Connect();

            while(!exit)
            {     
                shell.ParseAndExecute();                
            }
        }

        static void TheStack_NetworkAddressChanged(object sender, PyxNet.Stack.NetworkAddressChangedEventArgs e)
        {
            var nodeInfo = TheStack.NodeInfo;

            Console.WriteLine("Network Address Changed");
        }

        static void TheStack_Connected(object sender, PyxNet.Stack.ConnectedEventArgs e)
        {
            Console.WriteLine("Node Connected : " + e.StackConnection.RemoteNodeInfo.FriendlyName);
        }

        [ShellAction(Name = "node", Description = "Show node info")]
        static void ShowNodeInfo()
        {
            ShowNodeInfo(TheStack.NodeInfo);
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
            var pinfo = pinfos[index-1];

            pyxNatControl.RemovePortMapping(pinfo);
        }

        [ShellAction(Name = "s|status", Description = "Stack status")]
        static void StackStatus()
        {
            Console.WriteLine("IsHubConnected: {0}", TheStack.ConnectionManager.IsHubConnected);

            Console.ForegroundColor = ConsoleColor.Green;
            foreach (var node in TheStack.KnownHubList.ConnectedHubs)
            {
                Console.WriteLine("Conntected Hub: {0} - {1}", node.FriendlyName, node.NodeId);
            }
            Console.ResetColor();

            foreach (var node in TheStack.KnownHubList.KnownHubs)
            {
                Console.WriteLine("Known Hub: {0} - {1}", node.FriendlyName, node.NodeId);
            }

            foreach (var node in TheStack.AllKnownNodes.Values)
            {
                Console.WriteLine("Known Node: {0} - {1}", node.FriendlyName, node.NodeId);
            }
        }

        [ShellAction(Name = "find", Description = "Find Node over Pyxnet from node Guid")]
        static void FindNode(Guid nodeGuid,int seconds)
        {
            var nodeId = new PyxNet.NodeId(nodeGuid);
            var nodeInfo = PyxNet.NodeInfo.Find(TheStack, nodeId , TimeSpan.FromSeconds(seconds));

            if (nodeInfo == null)
            {
                Console.WriteLine("can't find node with node ID: {0}", nodeId);
            }
            ShowNodeInfo(nodeInfo);
        }

        [ShellAction(Name = "l|listen", Description = "Listen to messages over the PyxNet Stack")]
        static void Listen()
        {
            TheStack.OnAnyMessage += TheStack_OnAnyMessage;                
            TheStack.OnSendingMessageToConnection += TheStack_OnSendingMessageToConnection;

            Console.WriteLine("Listening to messages [press any key to stop]");
            while (!Console.KeyAvailable)
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(0.2));
            }

            TheStack.OnAnyMessage -= TheStack_OnAnyMessage;
            TheStack.OnSendingMessageToConnection -= TheStack_OnSendingMessageToConnection;
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
                var nodeInfo =property.GetValue(request) as PyxNet.NodeInfo;
                Console.WriteLine("  Connnect request to Node {0} {1}", nodeInfo.FriendlyName, nodeInfo.NodeId);
            }
            
        }

        [ShellAction(Name = "q|query", Description = "Search for a string over PyxNet")]
        static void Query(string queryString, int seconds)
        {
            DateTime startTime = DateTime.Now;
            var querier = new PyxNet.Querier(TheStack, queryString, 1000);

            bool queryExpired = false;

            querier.QueryTimeout = TimeSpan.FromSeconds(seconds);
            querier.QueryExpired += (sender, args) => { queryExpired = true; };
            querier.Result += (sender, args) =>
            {
                Console.WriteLine(" Result Return from {0}: {1}", args.QueryResult.ResultNode.FriendlyName, args.QueryResult.MatchingContents); 
                startTime = DateTime.Now; //make sure we wait few more seconds before returning
            };
            querier.Start();

            while(!queryExpired && !Console.KeyAvailable && (DateTime.Now - startTime).TotalSeconds < seconds+0.1)
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(0.2));
            }

            if (queryExpired)
            {
                Console.WriteLine("Query was expired");
            }
            
            querier.Stop();
        }

        [ShellAction(Name = "c|connect", Description = "Connect to a node over PyxNet")]
        static void ConnectTo(Guid nodeGuid,int seconds)
        {
            Console.WriteLine("finding nodeInfo:" + nodeGuid + "...");

            var nodeId = new PyxNet.NodeId(nodeGuid);
            var nodeInfo = PyxNet.NodeInfo.Find(TheStack, nodeId , TimeSpan.FromSeconds(seconds));

            if (nodeInfo == null)
            {
                Console.WriteLine("can't find node with node ID: {0}", nodeInfo.FriendlyName);
                return;
            }

            Console.WriteLine("try to connect to node " + nodeGuid + "...");
            var connection = TheStack.ConnectionManager.GetConnection(nodeInfo, false, TimeSpan.FromSeconds(seconds));

            if (connection == null)
            {
                Console.WriteLine("can't create a connection to node {0}", nodeInfo.FriendlyName);
                return;
            }

            Console.WriteLine("connection created!");
        }
    }   
}
