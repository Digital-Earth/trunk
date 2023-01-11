#define NO_LICENSE_SERVER

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Pyxis.Utilities;

namespace PyxNetHub
{
    internal class Program
    {
        private static bool m_logConnections = true;
        private static bool m_logNodeInfo = true;
        private static bool m_logEverythingElse = false;

        /// <summary>
        /// Writes the line to both the diagnostic trace and the console window.
        /// </summary>
        /// <param name="format">The format.</param>
        /// <param name="args">The args.</param>
        static private void WriteLine(string format, params object[] args)
        {
            string message = String.Format(format, args);

            string prefix = String.Format("[{0:0000}] {1:HH:mm:ss} - ",
                System.Threading.Thread.CurrentThread.ManagedThreadId,
                DateTime.Now);

            Console.WriteLine(prefix + message);
            System.Diagnostics.Trace.WriteLine(prefix + message);
        }

        private class LoggingServiceImp : PyxNet.Logging.LoggingService
        {
            public LoggingServiceImp(PyxNet.Stack s)
                : base(s)
            {
                this.LoggedEventReceived += HandleLoggedEventReceived;
            }

            private void HandleLoggedEventReceived(object sender, PyxNet.Logging.LoggingService.LoggedEventReceivedEventArgs e)
            {
                WriteLine("Logged message {0} recieved from {1}. {2}.", e.LoggedEvent.Category,
                    e.Sender.ToString(), e.LoggedEvent.Description);

                Log myLog = new Log();
                Log.Messages1Row row = myLog.Messages1.NewMessages1Row();
                row.Category = e.LoggedEvent.Category;
                row.Description = e.LoggedEvent.Description;
                row.TimeStamp = e.LoggedEvent.TimeStamp;
                row.Node = e.Sender.Identity.ToString();
                myLog.Messages1.AddMessages1Row(row);
                LogTableAdapters.Messages1TableAdapter adaptor = new PyxNetHub.LogTableAdapters.Messages1TableAdapter();
                int result = adaptor.Update(myLog);

                WriteLine("Updated {0} rows.", result);
            }
        }

        static private LoggingServiceImp s_loggingService = null;

        /// <summary>
        /// Parses a single argument from the command line (or from a DDE connection.)
        /// </summary>
        /// <param name="argument">The argument.</param>
        /// <param name="unParsedArguments">The un parsed arguments (filled in by this function).</param>
        /// <returns>
        /// true iff the app should continue running.
        /// </returns>
        static private bool ParseCommandLine(string argument, ref List<string> unParsedArguments)
        {
            if (argument.Contains("--restart"))
            {
                WriteLine("Restart requested from external application (hopefully from the launcher).  Restarting.");
                return false;
            }
            else if (argument.Contains("--close"))
            {
                WriteLine("Shutdown requested from external application (hopefully from the launcher).  Terminating.");
                return false;
            }
            else if (argument.Contains("--log"))
            {
                WriteLine("Logging enabled.");

                if (s_loggingService == null)
                {
                    s_loggingService = new LoggingServiceImp(PyxNet.StackSingleton.Stack);

                    // Create an empty database, if necessary.
                    string connectionString = PyxNetHub.Properties.Settings.Default.PyxNetHubLogConnectionString;
                    //if (connectionString.Contains("%AppDataLocal%"))
                    //{
                    //    PyxNetHub.Properties.Settings.Default.PyxNetHubLogConnectionString =
                    //        connectionString.Replace("%AppLocalData%",
                    //        System.Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)).Replace("\\\\", "\\");
                    //}
                    if (connectionString.StartsWith("data source=", StringComparison.OrdinalIgnoreCase))
                    {
                        string databaseFile = connectionString.Substring("data source=".Length);
                        if (!System.IO.File.Exists(databaseFile))
                        {
                            try
                            {
                                System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
                                Stream resourceStream = assembly.GetManifestResourceStream(
                                    "PyxNetHub.Resources.PyxNetHubLog.SQLite");

                                if (resourceStream != null)
                                {
                                    using (FileStream emptyDatabaseFile = System.IO.File.Create(databaseFile))
                                    {
                                        byte[] dataBuffer = new byte[4096];
                                        int bytesRead = resourceStream.Read(dataBuffer, 0, dataBuffer.Length);
                                        while (bytesRead > 0)
                                        {
                                            emptyDatabaseFile.Write(dataBuffer, 0, dataBuffer.Length);
                                            bytesRead = resourceStream.Read(dataBuffer, 0, dataBuffer.Length);
                                        }
                                    }
                                }
                            }
                            catch (Exception ex)
                            {
                                WriteLine("Unhandled exception while initializing database: " + ex.Message);
                                throw;
                            }
                        }
                    }
                    // Log directly.
                    s_loggingService.OnLoggedEventReceived(PyxNet.StackSingleton.Stack.NodeInfo.NodeId,
                        new PyxNet.Logging.LoggedEventMessage("Logging Service",
                            string.Format("Logging service started on {0}.", PyxNet.StackSingleton.Stack.NodeInfo.FriendlyName)));
                    // TODO: The following line won't work because we refuse to send a message to ourself.
                    PyxNet.Logging.LoggingClient.DefaultClient.Send("Logging Service", "Legitimate message from service.");
                }
            }
            else if (argument.Contains("--nolog"))
            {
                WriteLine("Logging disabled.");

                if (s_loggingService != null)
                {
                    // TODO: Dispose?
                    s_loggingService = null;
                }
            }
            else
            {
                unParsedArguments.Add(argument);
            }
            return true;
        }

        /// <summary>
        /// Create a PyxNet Stack and run it in Hub mode until someone presses Alt+X.
        /// </summary>
        /// <param name="args">Command Line Args</param>
        public static int Main(string[] args)
        {
            try
            {
                //System.Diagnostics.Trace.Listeners.Add(new System.Diagnostics.TextWriterTraceListener(Console.Out));
                //System.Diagnostics.Debug.Listeners.Add(new System.Diagnostics.TextWriterTraceListener(Console.Out));

                using (Pyxis.Utilities.DDEManager ddeManager = new Pyxis.Utilities.DDEManager())
                {
                    if (ddeManager.Initialize(args) == false)
                    {
                        // Another hub is already running.
                        return (int)0x04;
                    }

                    bool shutdownRequested = false;

                    // Connect up for messages from other copies of the app.
                    ddeManager.DDECommandReceived +=
                        delegate(object sender,
                        Pyxis.Utilities.DDECommandReceivedEventArgs e)
                        {
                            List<string> unparsedDDEArgs = new List<string>();
                            if (ParseCommandLine(e.Command, ref unparsedDDEArgs) == false)
                            {
                                shutdownRequested = true;
                            }

                            // We ignore any extra args on the DDE command line.
                        };

                    // set this to listen on the hub port.
                    PyxNet.StackSingleton.InitialPort = PyxNet.PyxNetStack.HubPort;

                    // Get the stack going.
                    using (PyxNet.PyxNetStack theStack = PyxNet.StackSingleton.Stack)
                    {
                        // Ensure that each external address uses the hub port,
                        // and throw an exception otherwise.
                        System.Action<System.Net.IPEndPoint> checkPort =
                            delegate(System.Net.IPEndPoint ipEndPoint)
                            {
                                int port = ipEndPoint.Port;
                                if (port != PyxNet.PyxNetStack.HubPort)
                                {
                                    throw new ApplicationException(String.Format(
                                        "The hub is using port {0} instead of {1}, and will not accept any PyxNet client connections.",
                                        port, PyxNet.PyxNetStack.HubPort));
                                }
                            };

                        foreach (System.Net.IPEndPoint ipEndPoint in theStack.NodeInfo.Address.ExternalIPEndPoints)
                        {
                            checkPort(ipEndPoint);
                        }
                        foreach (System.Net.IPEndPoint ipEndPoint in theStack.NodeInfo.Address.InternalIPEndPoints)
                        {
                            checkPort(ipEndPoint);
                        }

                        // It's a hub.
                        theStack.NodeInfo.Mode = PyxNet.NodeInfo.OperatingMode.Hub;

                        // Set the friendly name of the stack.
                        String stackName = String.Format("Hub {0}", theStack.NodeInfo.FriendlyName);
                        theStack.NodeInfo.FriendlyName = stackName;

                        // Logging.
                        theStack.Tracer.Enabled = true;
                        theStack.OnAnyMessage += theStack_OnAnyMessage;

                        // Parse the arguments.
                        List<string> unparsedArgs = new List<string>();
                        foreach (string command in args)
                        {
                            if (ParseCommandLine(command, ref unparsedArgs) == false)
                            {
                                shutdownRequested = true;
                            }
                        }

                        // The next unparsed argument is the publish directory.
                        string publishDirectory = null;
                        if (unparsedArgs.Count > 0)
                        {
                            publishDirectory = unparsedArgs[0];
                            unparsedArgs.RemoveAt(0);
                            WriteLine("Publishing from directory {0}", publishDirectory);
                        }
                        else
                        {
                            WriteLine("Not publishing anything.  (Publication directory is expected as the first command-line parameter.");
                        }

                        //--
                        //-- by default, a deadman timer will be setup
                        //-- to restart the hub periodically.
                        //--
                        if (PyxNetHub.Properties.Settings.Default.EnableNoConnectionRestartTimer)
                        {
                            WriteLine("Enabling \"no connection\" restart timer.");
                            // If we go too long (60 minutes) without any reconnection, we fire a restart.
                            Pyxis.Utilities.DeadManTimer connectionTimeout = new Pyxis.Utilities.DeadManTimer(
                                TimeSpan.FromMinutes(60),
                                delegate(object o, System.Timers.ElapsedEventArgs a)
                                {
                                    WriteLine("We have not seen a new connection in over sixty minutes.  Restarting.");
                                    shutdownRequested = true;
                                });

                            theStack.Connected +=
                                delegate(object sender, PyxNet.Stack.ConnectedEventArgs e)
                                {
                                    connectionTimeout.KeepAlive();
                                };
                        }
                        else
                        {
                            WriteLine("WARNING: \"no connection\" restart timer is DISABLED.");
                        }

                        WriteLine("Running PyxNet hub at adddress " + theStack.NodeInfo.Address.ToString());
                        WriteLine("NodeID is " + theStack.NodeInfo.NodeGUID.ToString());

                        Console.WriteLine("Press Alt+X to stop.");

                        // Each remaining unparsed argument is a host to connect to.
                        // Try to connect to each.
                        // If we fail to connect, we simply eat any exceptions since
                        // we always want to start the hub.
                        foreach (string hostName in unparsedArgs)
                        {
                            try
                            {
                                WriteLine("Connecting to named host at {0}", hostName);
                                // This could be our own address, so we eat the "connecting to self" exception.
                                ConnectToNamedHost(hostName);
                            }
                            catch (Exception ex)
                            {
                                WriteLine(string.Format("Failed to connect to {0}.  {1}.", hostName, ex.Message));
                            }
                        }

                        // Try to publish the directory from the command line.
                        try
                        {
                            DirectoryInfo publishDir = new DirectoryInfo(publishDirectory);
                            if (publishDir.Exists)
                            {
                                PublishDirectory(theStack.FilePublisher, publishDir);
                            }
                        }
                        catch
                        {
                        }

                        // Expedite the sending of the QHT so that published items are available right away.
                        theStack.ForceQueryHashTableUpdate();

                        CertifyAllManifests();

                        WriteLine(theStack.FilePublisher.PublishedFiles.Count() + " files published.");

                        for (; ; )
                        {
                            while (!Console.KeyAvailable)
                            {
                                if (shutdownRequested)
                                {
                                    return 0x20;
                                }
                                System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
                            }
                            ConsoleKeyInfo theKey = Console.ReadKey();
                            if ((theKey.KeyChar.ToString().ToUpper() == "X") &&
                                ((theKey.Modifiers & ConsoleModifiers.Alt) != 0))
                            {
                                break;
                            }
                            switch (theKey.KeyChar.ToString().ToUpper())
                            {
                                case "A":
                                    m_logEverythingElse = !m_logEverythingElse;
                                    break;

                                case "C":
                                    m_logConnections = !m_logConnections;
                                    break;

                                case "D":
                                    Console.WriteLine("Connection logging is " + m_logConnections.ToString());
                                    Console.WriteLine("Node Info logging is " + m_logNodeInfo.ToString());
                                    Console.WriteLine("All other message logging is " + m_logEverythingElse.ToString());
                                    break;

                                case "L":
                                    DumpLogs();
                                    break;

                                case "N":
                                    m_logNodeInfo = !m_logNodeInfo;
                                    break;

                                case "S":
                                    WriteLine("");
                                    WriteLine("***** Hub Status *****");
                                    WriteLine("");
                                    WriteLine(theStack.ToStringVerbose());
                                    WriteLine("Publishing {0} files.",
                                        theStack.FilePublisher.PublishedFiles.Count());
                                    foreach (var f in theStack.FilePublisher.PublishedFiles)
                                    {
                                        if (IsValidManifestFile(f))
                                        {
                                            WriteLine("Manifest file: {0}", f.FullName);
                                        }
                                    }
                                    WriteLine("***** End of Hub Status *****");
                                    break;

                                case "H":
                                    Console.WriteLine("Alt+X - exit program.");
                                    Console.WriteLine("A     - toggle All other message logging.");
                                    Console.WriteLine("C     - toggle Connection logging.");
                                    Console.WriteLine("D     - display currect logging settings.");
                                    Console.WriteLine("H     - display this help message.");
                                    Console.WriteLine("L     - dump log to a file.");
                                    Console.WriteLine("N     - toggle Node Info logging.");
                                    Console.WriteLine("S     - show stack status.");
                                    break;
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                WriteLine("Unhandled exception in hub: {0}", ex.ToString());
            }
            return 0;
        }

        private static Dictionary<FileInfo, System.Threading.Thread> s_manifestThreads =
            new Dictionary<FileInfo, System.Threading.Thread>();

        /// <summary>
        /// Certifies all manifests.  This is called at startup, and every time the
        /// published directory changes.
        /// </summary>
        private static void CertifyAllManifests()
        {
            PyxNet.Stack theStack = PyxNet.StackSingleton.Stack;

            // Find all of the manifests that we're publishing, and get certificates for them.
            foreach (FileInfo file in theStack.FilePublisher.PublishedFiles)
            {
                if (IsValidManifestFile(file))
                {
                    lock (s_manifestThreads)
                    {
                        // Throw away dead threads....
                        if (s_manifestThreads.ContainsKey(file) &&
                            !s_manifestThreads[file].IsAlive)
                        {
                            s_manifestThreads.Remove(file);
                        }

                        // ... and add new threads, if necessary.
                        if (!s_manifestThreads.ContainsKey(file))
                        {
                            System.Threading.Thread thread =
                                new System.Threading.Thread(CertifyManifest);
                            thread.IsBackground = true;
                            thread.Start(file);
                            s_manifestThreads.Add(file, thread);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Determines whether the specified file is a valid manifest file.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <returns>
        /// 	<c>true</c> if the specified file is a valid manifest file; otherwise, <c>false</c>.
        /// </returns>
        private static bool IsValidManifestFile(FileInfo file)
        {
            if (file.Extension.ToLower() == ".manifest")
            {
                try
                {
                    Pyxis.Utilities.Manifest manifest = Pyxis.Utilities.Manifest.ReadFromFile(file.FullName);

                    // Verify that each file in the manifest is present.
                    string temporaryDirectory = PyxNet.FileTransfer.ManifestDownloader.CacheDirectory;
                    string outputDirectory = file.DirectoryName;

                    PyxNet.FileTransfer.DownloadContext downloadContext =
                        new PyxNet.FileTransfer.DownloadContext(outputDirectory, temporaryDirectory);

                    PyxNet.FileTransfer.DownloadContext.DownloadedManifest tester =
                        new PyxNet.FileTransfer.DownloadContext.DownloadedManifest(
                        downloadContext, manifest);

                    if (tester.Valid)
                    {
                        return true;
                    }
                    else
                    {
                        //--
                        //-- traverse manifest looking for missing files,
                        //-- and files that fail the CRC check
                        //--
                        List<string> missingFiles = new List<string>();
                        List<string> badCrcs = new List<string>();
                        List<string> generalFailures = new List<string>();

                        foreach (var entry in tester.Entries)
                        {
                            if (!entry.Valid)
                            {
                                if (string.IsNullOrEmpty(entry.ManifestEntry.FileStamp))
                                {
                                    generalFailures.Add(entry.ManifestEntry.RelativePath);
                                }
                                else if (!downloadContext.ExistsInFinal(entry.ManifestEntry))
                                {
                                    missingFiles.Add(entry.ManifestEntry.RelativePath);
                                }
                                else
                                {
                                    // We have no checksum yet.  Forcibly build it and test it.
                                    string filename = downloadContext.ConstructFinalPath(entry.ManifestEntry);

                                    string checksum =
                                            Pyxis.Utilities.ChecksumSingleton.Checksummer.CalculateFileCheckSumNoCache(
                                            filename);
                                    if (!entry.ManifestEntry.FileStamp.Equals(checksum))
                                    {
                                        badCrcs.Add(entry.ManifestEntry.RelativePath);
                                    }
                                }
                            }
                        }

                        if ((missingFiles.Count > 0) || (badCrcs.Count > 0) || (generalFailures.Count > 0))
                        {
                            StringBuilder message = new StringBuilder();
                            message.AppendFormat("Ignoring: {0} because its contents are damaged:", file.FullName);
                            message.AppendLine();

                            if (missingFiles.Count > 0)
                            {
                                message.AppendFormat("*** {0} file(s) missing: {1}.",
                                    missingFiles.Count, string.Join(", ", missingFiles.ToArray()));
                                message.AppendLine();
                            }

                            if (badCrcs.Count > 0)
                            {
                                message.AppendFormat("*** {0} files have failed CRC check: {1}.",
                                    badCrcs.Count, string.Join(", ", badCrcs.ToArray()));
                                message.AppendLine();
                            }

                            if (generalFailures.Count > 0)
                            {
                                message.AppendFormat("*** {0} files have failed verification: {1}.",
                                    badCrcs.Count, string.Join(", ", generalFailures.ToArray()));
                                message.AppendLine();
                            }

                            WriteLine("{0}", message.ToString());

                            return false;
                        }
                        else
                        {
                            // Each file checked out.  All is good.
                            return true;
                        }
                    }
                }
                catch (Exception)
                {
                    WriteLine("{0} is not a valid manifest.  Ignoring.", file.FullName);
                }
            }
            return false;
        }

        /// <summary>
        /// Connects to the named host.
        /// </summary>
        /// <param name="hostName">Name of the host.</param>
        private static void ConnectToNamedHost(string hostName)
        {
            if (null == hostName)
            {
                throw new ArgumentNullException("hostName");
            }

            PyxNet.StackSingleton.Stack.Connect(hostName);

            if (PyxNet.StackSingleton.Stack.Port != PyxNet.PyxNetStack.HubPort)
            {
                WriteLine("The hub port {0} is already in use.  Using port {1}.",
                    PyxNet.PyxNetStack.HubPort, PyxNet.StackSingleton.Stack.Port);
            }
        }

        private static Pyxis.Utilities.DeadManTimer s_fileUpdated = null;
        private static object s_fileUpdatedLock = new object();

        /// <summary>
        /// Signal that a file has been updated.
        /// </summary>
        private static void FileUpdated()
        {
            lock (s_fileUpdatedLock)
            {
                if (s_fileUpdated == null)
                {
                    s_fileUpdated = new Pyxis.Utilities.DeadManTimer(
                        TimeSpan.FromSeconds(10), FileUpdateFinished);
                }
                s_fileUpdated.KeepAlive();
            }
        }

        /// <summary>
        /// Files the update finished.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="args">The <see cref="System.Timers.ElapsedEventArgs"/> instance containing the event data.</param>
        private static void FileUpdateFinished(object sender, System.Timers.ElapsedEventArgs args)
        {
            lock (s_fileUpdatedLock)
            {
                s_fileUpdated = null;
            }

            CertifyAllManifests();
        }

        private static void ConnectTo(DelayedFileSystemWatcher watcher)
        {
            watcher.Changed += delegate(object sender, DelayedFileSystemEventArgs e)
            {
                FileUpdated();

                // Only get verbose about file chnages.  Directories change
                // whenever you look at them under NT.
                var existingFiles = e.Events.Where(x => File.Exists(x.FullPath)).ToList();
                if (existingFiles.Count > 0)
                {
                    WriteLine(string.Format("Observed a change in {0}.", existingFiles.Select(x => x.FullPath).Aggregate((x, y) => x + ", " + y)));
                }
            };
        }

        private static void PublishDirectory(PyxNet.Publishing.Files.FilePublisher publisher, DirectoryInfo directory)
        {
            try
            {
                WriteLine("Publishing files from : " + directory.FullName);
                if (directory.Exists)
                {
                    ConnectTo(publisher.PublishDirectory(directory));
                }
            }
            catch
            {
            }

            foreach (DirectoryInfo subDirectory in directory.GetDirectories())
            {
                PublishDirectory(publisher, subDirectory);
            }
        }

        private static void DumpLogs()
        {
            WriteLine("Trace logs dumped to file: " + Pyxis.Utilities.TraceTool.SaveToTempFile());
        }

        /// <summary>
        /// Gets the name (and address) of the node to display.
        /// </summary>
        /// <param name="nodeInfo">The node info.</param>
        /// <returns></returns>
        static private string GetNodeDisplayName(PyxNet.NodeInfo nodeInfo)
        {
            StringBuilder result = new StringBuilder();
            result.Append(nodeInfo.FriendlyName);
            System.Net.IPEndPoint endPoint = null;
            foreach (System.Net.IPEndPoint internalEndPoint in nodeInfo.Address.InternalIPEndPoints)
            {
                endPoint = internalEndPoint;
            }
            foreach (System.Net.IPEndPoint externalEndPoint in nodeInfo.Address.ExternalIPEndPoints)
            {
                endPoint = externalEndPoint;
            }
            if (endPoint != null)
            {
                result.AppendFormat(" at {0}", endPoint.ToString());
            }
            return result.ToString();
        }

        private static void theStack_OnAnyMessage(object sender, PyxNet.Stack.AnyMessageEventArgs args)
        {
            switch (args.Message.Identifier)
            {
                case PyxNet.StackConnectionRequest.MessageID:
                    if (m_logConnections)
                    {
                        try
                        {
                            PyxNet.StackConnectionRequest theRequest = new PyxNet.StackConnectionRequest(args.Message);
                            if (theRequest.IsToNodeGUID(args.Stack.NodeInfo.NodeGUID))
                            {
                                WriteLine("Connection from {0}.", GetNodeDisplayName(theRequest.FromNodeInfo));
                            }
                            else
                            {
                                WriteLine("Connection for another node from {0}.",
                                    GetNodeDisplayName(theRequest.FromNodeInfo));
                            }
                        }
                        catch
                        {
                            WriteLine("{0} sent a badly formed connection request message : {1}",
                                args.Connection.ToString(),
                                args.Message.ToString());
                        }
                    }
                    break;

                case PyxNet.StackConnection.LocalNodeInfoMessageID:
                    if (m_logNodeInfo)
                    {
                        try
                        {
                            PyxNet.NodeInfo nodeInfo = new PyxNet.NodeInfo(args.Message);
                            WriteLine("Node info from {0}.",
                                GetNodeDisplayName(nodeInfo));
                        }
                        catch (Exception e)
                        {
                            WriteLine("Exception handling local node info message: {0}", e.Message);
                        }
                    }
                    break;

                default:
                    if (m_logEverythingElse)
                    {
                        WriteLine("{0} message from {1}.",
                            args.Message.Identifier,
                            args.Connection.ToString());
                    }
                    break;
            }
        }

        private static object threadLock = new object();

        private static void CertifyManifest(object arg)
        {
            while (true)
            {
                // When should we retry?
                DateTime timeToRetry = DateTime.Now + TimeSpan.FromMinutes(2);

                try
                {
                    lock (threadLock)
                    {
                        FileInfo file = arg as FileInfo;

                        WriteLine("CertifyManifest: file={0}", file.FullName);

                        //--
                        //-- certificate timer needs to release every so often so
                        //-- that the certificate query request can be re-issued
                        //-- if there is no response from the query.
                        //-- possible cause is license server comining on line.
                        //--
                        Pyxis.Utilities.SynchronizationEvent certificateTimer =
                            new Pyxis.Utilities.SynchronizationEvent(TimeSpan.FromSeconds(30));

                        PyxNet.Service.CertificateRequester requester = null;

                        if (IsValidManifestFile(file))
                        {
                            // TODO: Use the correct directory here...
                            Pyxis.Utilities.ManifestEntry manifestEntry =
                                new Pyxis.Utilities.ManifestEntry(file, ".");
                            PyxNet.Service.ResourceInstanceFact manifestFact =
                                new PyxNet.Service.ResourceInstanceFact(manifestEntry);

                            // Search for a certificate.
                            PyxNet.Service.ResourceInstanceFact certifiedFact =
                                PyxNet.Service.ResourceInstanceFact.FindCertifiedFact(
                                    PyxNet.StackSingleton.Stack,
                                    manifestFact);

                            if (certifiedFact != null)
                            {
                                WriteLine("Using existing certificate for {0}. ",
                                    manifestEntry.FileName);
                                timeToRetry = certifiedFact.Certificate.ExpireTime + TimeSpan.FromSeconds(30);
                            }
                            else
                            {
                                WriteLine("Requesting a new certificate for {0}. ",
                                    manifestEntry.FileName);

                                // If we didn't find one, then request one.
                                requester = new PyxNet.Service.CertificateRequester(
                                    PyxNet.StackSingleton.Stack, manifestFact);

                                requester.DisplayUri +=
                                    delegate(object sender, PyxNet.DisplayUriEventArgs e)
                                    {
                                        WriteLine("Certificate request generated a URL, SHOULD NOT HAVE HAPPENED.");

                                        // TODO: We need to have an "admin" operation
                                        // on the license server, so that we don't have to have a user sitting in front of
                                        // the PyxNetHub machine.
#if false
                                        System.Diagnostics.Process.Start(e.Uri.ToString());
#endif
                                    };

                                requester.CertificateReceived +=
                                    delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                                    {
                                        //--
                                        //-- certificate received, release the timer, and calculate
                                        //-- a retry time based on certificate expiry time.
                                        //--
                                        requester = null;
                                        timeToRetry = c.Certificate.ExpireTime + TimeSpan.FromSeconds(30);
                                        WriteLine("Received a certificate for {0}. ",
                                            manifestEntry.FileName);
                                        certificateTimer.Pulse();
                                    };

                                // TODO: PermissionGranted event should be the one to tell us that we're done
                                requester.PermissionGranted +=
                                    delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                                    {
                                        // TODO: Fill this in
                                    };

#if NO_LICENSE_SERVER // TODO: Remove forgery
                                new Pyxis.Utilities.SimpleTimer(TimeSpan.FromSeconds(1),
                                    delegate(object timeout_sender, System.Timers.ElapsedEventArgs timeout_e)
                                    {
                                        if (requester != null)
                                        {
                                            // The certificate hasn't been returned, so forge one.
                                            WriteLine("Forging a certificate for {0}, license server code turned off.",
                                                manifestEntry.FileName);
                                            PyxNet.Service.ServiceInstance localService =
                                                PyxNet.Service.ServiceInstance.Create(
                                                    PyxNet.Service.CertificateServer.CertificateAuthorityServiceId,
                                                    PyxNet.StackSingleton.Stack.NodeInfo.NodeId);

                                            PyxNet.Service.Certificate forgedCertificate = new PyxNet.Service.Certificate(
                                                localService, DateTime.Now + TimeSpan.FromDays(1));
                                            forgedCertificate.Add(manifestFact);
                                            forgedCertificate.IssuedTime = file.CreationTime;
                                            forgedCertificate.SignCertificate(PyxNet.StackSingleton.Stack.PrivateKey);

                                            requester.OnCertificateReceived(requester, forgedCertificate);
                                        }
                                    });
#endif

#if !NO_LICENSE_SERVER
                                requester.Start(TimeSpan.FromSeconds(30));
#endif
                            }
                        }
                        else
                        {
                            // This manifest is no longer valid.  It might have
                            // been deleted, or mangled, or (in the future) it
                            // might refer to a file that we don't have.
                            return;
                        }

                        //--
                        //-- requester at work, wait for certificate to arrive.
                        //-- certificate timer will release with timeout.
                        //--
                        if (requester != null)
                        {
                            certificateTimer.Wait();
                        }
                    }
                }
                catch (ApplicationException ex)
                {
                    WriteLine(String.Format("Error certifying manifest: {0}", ex.Message));
                }
                catch (Exception e)
                {
                    WriteLine(String.Format("Internal failure while certifying manifest: {0}", e.Message));
                }

                //--
                //-- put thread into waiting state until the received certificate expires.
                //-- or two miniutes from now if we did not get a certificate.
                //--
                TimeSpan timeRemaining = timeToRetry - DateTime.Now;
                if (timeRemaining.TotalMilliseconds > 0)
                {
                    new Pyxis.Utilities.SynchronizationEvent(timeRemaining).Wait();
                }
            }
        }
    }
}