using ApplicationUtility;
using GeoWebCore.Properties;
using Microsoft.Owin.Hosting;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.UI.Layers;
using Pyxis.Utilities;
using Pyxis.Publishing;
using System;
using System.Linq;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;
using System.Net;
using GeoWebCore.Controllers;
using GeoWebCore.Services;
using GeoWebCore.Services.Cluster;
using GeoWebCore.Services.Storage;

namespace GeoWebCore
{
    internal class Program
    {
        public static GeoWebCoreRunInformation RunInformation = new GeoWebCoreRunInformation();

        public static Engine Engine;

        public static SharpRaven.RavenClient RavenClient;

        private static void Main(string[] args)
        {
            var assemblyName = Assembly.GetEntryAssembly().GetName();

            //enable crash reporter only if we run outside the debugger. we let exception pass to the debuger if there is one attached.
            var enableCrashReporter = !Debugger.IsAttached;

            using (var crashReporter = new CrashReporter(
                assemblyName.Name,
                assemblyName.Version.ToString(),
                Settings.Default.CrashDumpUploadUri,
                enableCrashReporter))
            {
                crashReporter.DisableWindowsErrorReportingDialog();

                // modify WorldViewDefault to allow authenticated requests
                var config = EngineConfig.WorldViewDefault;
                var apiKey = new ApiKey(Settings.Default.GWCUserKey, Settings.Default.GWCUserAccount);
                var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI).Authenticate(apiKey);
                config.User = channel.AsUser();

                // see if we can keep this one for local testing
                config.UsePyxnet = false;

                bool doImport = false;
                bool doProcess = false;
                bool doDownload = false;
                bool doGalleryStatus = false;
                GeoWebCoreRunInformation.RunMode runMode = GeoWebCoreRunInformation.RunMode.Server;
                var root = Settings.Default.UserFilesSharedStorage;

                int numberOfSegments = 0;
                int segmentId = 0;

                string clusterUri = Environment.GetEnvironmentVariable("GGS_CLUSTER_MASTER");

                Console.WriteLine("Command Line Args: {0}", String.Join(" ", args));

                //set default envrionment as dev.
                RunInformation.Environment = GeoWebCoreRunInformation.RunEnvironment.Dev;

                args = ArgsParser.Parse(args,
                    //set up the node is
                    new ArgsParser.Option("node", (guid) =>
                    {
                        config.PyxNetNodeId = Guid.Parse(guid);
                        Console.WriteLine("Node ID = " + config.PyxNetNodeId);
                    }),

                    //set up the log level
                    new ArgsParser.Option("verbose", (x) => config.TraceLevel = TraceLevels.Info),

                    //set up the log level
                    new ArgsParser.Option("env",
                        (x) =>
                            RunInformation.Environment =
                                (GeoWebCoreRunInformation.RunEnvironment)
                                Enum.Parse(typeof(GeoWebCoreRunInformation.RunEnvironment), x, true)),

                    //set the raw files director is 
                    new ArgsParser.Option("fd|filesDir", (x) => root = x),

                    //set the raw files director is 
                    new ArgsParser.Option("cd|cacheDir", (x) => config.CacheDirectory = x),

                    //set the cluster url
                    new ArgsParser.Option("cluster", (x) => clusterUri = x ),

                    //defined if to perform import or just discovery
                    new ArgsParser.Option("i|import", (x) => doImport = true),

                    //defined if to perform import or just discovery
                    new ArgsParser.Option("p|process", (x) => doProcess = true),

                    //defined in to download source file for a give geo-source
                    new ArgsParser.Option("d|download", (x) => doDownload = true),

                    //defined in to download source file for a give geo-source
                    new ArgsParser.Option("gs|galleryStatus", (x) => doGalleryStatus = true),

                    new ArgsParser.Option("segment", (x) => segmentId = int.Parse(x)),
                    new ArgsParser.Option("segments", (x) => numberOfSegments = int.Parse(x)),

                    //run validate checksum
                    new ArgsParser.Option("vc|validateChecksums",
                        (x) => runMode = GeoWebCoreRunInformation.RunMode.ValidateChecksum)
                );

                AppServices.setConfiguration(AppServicesConfiguration.localStorageFormat,
                    AppServicesConfiguration.localStorageFormat_files);
                AppServices.setConfiguration(AppServicesConfiguration.importMemoryLimit, "200");

                //initialize raven client if we have one set
                if (!String.IsNullOrEmpty(Settings.Default.RavenClient))
                {
                    RavenClient = new SharpRaven.RavenClient(Settings.Default.RavenClient);
                }

                Engine = Engine.Create(config);
                Engine.Start();
                Engine.EnableAllImports();
                GeoSourceInitializer.Initialize(Engine, config);
                Engine.BeforeStopping(GeoSourceInitializer.Deinitialize);

                // common root for all user files

                if (String.IsNullOrEmpty(root))
                {
                    root = AppServices.getCacheDir("UserFiles");
                }

                RunInformation.GalleryFilesRoot = root;

                //Register Pyxis.UI resources to load default icons
                ManagedBitmapServer.Instance.RegisterResourcesInAssembly(Assembly.GetAssembly(typeof(GlobeLayer)));

                //get http listen address (or addresses)
                var addresses = Settings.Default.DefaultHttpAddresses;

                if (args.Length > 0)
                {
                    addresses = args[0];
                }

                if (doGalleryStatus)
                {
                    runMode = GeoWebCoreRunInformation.RunMode.GalleryStatus;
                }
                else if (args.Any(x => x.StartsWith("pyxis://")))
                {
                    runMode = doDownload
                        ? GeoWebCoreRunInformation.RunMode.GalleryDownload
                        : GeoWebCoreRunInformation.RunMode.GalleryImport;
                }
                else if (doImport)
                {
                    runMode = GeoWebCoreRunInformation.RunMode.LocalDiscovery;
                }
                else if (doProcess)
                {
                    runMode = GeoWebCoreRunInformation.RunMode.LocalImport;
                }

                RunInformation.Mode = runMode;

                Console.WriteLine("User files root: {0}", RunInformation.GalleryFilesRoot);
                Console.WriteLine("License server url: {0}", config.APIUrl);
                UserAuthorizer.LicenseServerUrl = config.APIUrl;

                Console.WriteLine("Run mode: {0}", RunInformation.Mode);
                Console.WriteLine("Run env: {0}", RunInformation.Environment);

                if (numberOfSegments > 0)
                {
                    Console.WriteLine("Run Segment: Segment #{0} of {1}", segmentId, numberOfSegments);
                    UserUrlsStorage.NumberOfSegments = numberOfSegments;
                    UserUrlsStorage.Segment = segmentId;

                    GalleryUrlsController.InitPublicGazetteer();
                }

                const string protocolPrefix = "pyxis://";

                switch (RunInformation.Mode)
                {
                    case GeoWebCoreRunInformation.RunMode.ValidateChecksum:
                        CLI.ValidateChecksum.Run();
                        break;

                    case GeoWebCoreRunInformation.RunMode.LocalDiscovery:
                        foreach (var arg in args)
                        {
                            CLI.ImportGeoSource.Discover(arg);
                        }
                        break;

                    case GeoWebCoreRunInformation.RunMode.LocalImport:
                        foreach (var arg in args)
                        {
                            CLI.ImportGeoSource.DiscoverAndProcess(arg);
                        }
                        break;

                    case GeoWebCoreRunInformation.RunMode.GalleryImport:
                        foreach (var arg in args.Where(x => x.StartsWith(protocolPrefix)))
                        {
                            CLI.ImportGeoSource.ImportFromGallery(Guid.Parse(arg.Substring(protocolPrefix.Length)));
                        }
                        break;

                    case GeoWebCoreRunInformation.RunMode.GalleryDownload:
                        foreach (var arg in args.Where(x => x.StartsWith(protocolPrefix)))
                        {
                            CLI.ImportGeoSource.DownloadFromGallery(Guid.Parse(arg.Substring(protocolPrefix.Length)), root);
                        }
                        break;

                    case GeoWebCoreRunInformation.RunMode.GalleryStatus:
                        var geoSources = new List<Guid>();
                        

                        foreach (var arg in args.Where(x => x.StartsWith(protocolPrefix)))
                        {
                            geoSources.Add(Guid.Parse(arg.Substring(protocolPrefix.Length)));
                        }

                        if (geoSources.Count == 0)
                        {
                            CLI.GalleryStatus.RunStatusOnTheLastDays();
                        }
                        else
                        {
                            CLI.GalleryStatus.RunStatusOnGeoSources(geoSources);
                        }

                        break;

                    case GeoWebCoreRunInformation.RunMode.Server:
                        {
                            var retries = 3;

                            while (retries > 0)
                            {
                                //split address: http:*/;https:*/
                                var startOptions = new StartOptions();
                                retries--;

                                foreach (var url in addresses.Split(';', ',').Select(x => x.Trim()))
                                {
                                    var fixedUrl = url;

                                    if (fixedUrl.EndsWith("+"))
                                    {
                                        fixedUrl = fixedUrl.TrimEnd('+');
                                        var portStart = fixedUrl.LastIndexOf(":");
                                        var port = int.Parse(fixedUrl.Substring(portStart + 1));
                                        var freePort = Utilities.PortFinder.GetFreePort(port);
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
                                        Console.WriteLine("GeoWebCore OWIN server started");
                                        //connect to cluster
                                        RunInformation.Cluster = new Cluster(clusterUri, startOptions.Urls);
                                        //publish endpoint
                                        AutomationLog.Endpoint("api", RunInformation.Cluster.LocalHost.First());
                                        Console.ReadLine();
                                        AutomationLog.Endpoint("api", "");
                                        Console.WriteLine("Closing GeoWebCore OWIN server");
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
                                            AutomationLog.UpdateInfo("restarting",true);
                                            System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(new Random().Next(500,1500)));
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
                        break;

                    default:
                        Console.WriteLine("Unknown run mode: " + runMode);
                        break;
                }
                Engine.Stop();
            }
        }
    }
}