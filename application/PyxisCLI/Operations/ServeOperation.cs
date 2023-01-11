using System;
using System.Linq;
using System.Net;
using System.Reflection;
using ApplicationUtility;
using Microsoft.Owin.Hosting;
using Pyxis.Utilities;
using PyxisCLI.Properties;
using PyxisCLI.Server;
using PyxisCLI.Server.Cluster;
using PyxisCLI.Server.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations
{
    class ServeOperation : IOperationMode
    {
        public string Command
        {
            get { return "serve"; }
        }

        public string Description
        {
            get { return "start Http server to serve data online"; }
        }

        public void Run(string[] args)
        {
            var host = "http://localhost:63036";
            var all = false;
            var authProvider = "";

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("host", (value) => host = value),
                new ArgsParser.Option("all", (value) => all = true),
                new ArgsParser.Option("auth-provider", (value) => authProvider = value));

            if (args.Length < 1)
            {
                Console.WriteLine("usage: pyx {0} [-host=host] [-all] [-auth-provider=provider]", Command);
                return;
            }

            if (all)
            {
                Program.Workspaces.LoadFolder();
            }
            else
            {
                foreach (var workspace in args.Skip(1))
                {
                    Program.Workspaces.GetWorkspace(workspace);
                }    
            }

            if (authProvider.HasContent())
            {
                ClusterConfiguration.ForceAuthenticationProvider(authProvider);
            }

            foreach (var keyValue in Program.Workspaces.Workspaces)
            {
                var workspace = keyValue.Value;
                if (workspace.Owner != null)
                {
                    Console.WriteLine("workspace: {0} [owner:{1}]", keyValue.Key, workspace.Owner.Name);
                }
                else
                {
                    Console.WriteLine("workspace: {0}", keyValue.Key);
                }
            }            

            StartServer(host);           
        }

        public void StartServer(string host)
        {
            string clusterUri = ClusterConfiguration.MasterNode;

            var engineState = Program.Engine.State;

            Console.WriteLine("Engine State : " + engineState);

            //initialize raven client if we have one set
            if (!String.IsNullOrEmpty(Settings.Default.RavenClient))
            {
                Program.RavenClient = new SharpRaven.RavenClient(Settings.Default.RavenClient);
            }

            var retries = 3;

            while (retries > 0)
            {
                //split address: http:*/;https:*/
                var startOptions = new StartOptions();
                retries--;

                foreach (var url in host.Split(';', ',').Select(x => x.Trim()))
                {
                    var fixedUrl = url;

                    if (fixedUrl.EndsWith("+"))
                    {
                        fixedUrl = fixedUrl.TrimEnd('+');
                        var portStart = fixedUrl.LastIndexOf(":");
                        var port = int.Parse(fixedUrl.Substring(portStart + 1));
                        var freePort = PortFinder.GetFreePort(port);
                        fixedUrl = fixedUrl.Substring(0, portStart) + ":" + freePort;

                        Console.WriteLine("Startup url: {0} -> {1}", url, fixedUrl);
                    }
                    else
                    {
                        Console.WriteLine("Startup url: {0}", fixedUrl);
                    }

                    startOptions.Urls.Add(fixedUrl);
                }

                Console.WriteLine("Starting server...");

                try
                {
                    using (WebApp.Start<OwinStartup>(startOptions))
                    {
                        Console.WriteLine("PyxisCLI REST server started");
                        //connect to cluster

                        Program.Cluster = new Cluster(clusterUri, startOptions.Urls);

                        Program.EngineConfig.ResourceReferenceResolvers.Add(resourceReference => Program.Cluster.ResolveGeoSource(resourceReference.Id).GetGeoSource() );

                        //publish endpoint
                        AutomationLog.Endpoint("api", Program.Cluster.LocalHost.First());

                        //publish host in task status
                        AutomationLog.UpdateInfo("host", Program.Cluster.LocalHost.First());

                        Console.ReadLine();
                        AutomationLog.Endpoint("api", "");
                        Console.WriteLine("Stopping PyxisCLI REST server");
                    }
                }
                catch (TargetInvocationException ex)
                {
                    if (ex.InnerException is HttpListenerException)
                    {
                        var httpListenerException = ex.InnerException as HttpListenerException;
                        Console.WriteLine("Failed to start server: {0} (ErrorCode:{1})", httpListenerException.Message, httpListenerException.ErrorCode);

                        //error code for port already in use
                        if (httpListenerException.ErrorCode == 183)
                        {
                            AutomationLog.UpdateInfo("restarting", true);
                            System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(new Random().Next(500, 1500)));
                            continue;
                        }
                    }
                    else
                    {
                        Console.WriteLine("Failed to start server:" + ex.Message);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to start server:" + ex.Message);
                }
                break;
            }
        }
    }
}
