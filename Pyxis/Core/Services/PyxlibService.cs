using System;
using ApplicationUtility;
using Pyxis.Utilities;

namespace Pyxis.Core.Services
{
    /// <summary>
    /// Manages the PYXIS Library service
    /// </summary>
    public class PyxlibService : ServiceBase
    {
        private EngineConfig Config { get; set; }
        private TraceToConsoleCallback TraceCallback { get; set; }
        private ManagedChecksumCalculator ManagedChecksumCalculator { get; set; }

        private bool m_initializationCompleted;

        private SimpleTimer m_traceFlushTimer;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="config">The configuration</param>
        public PyxlibService(EngineConfig config)
        {
            Config = config;
        }

        /// <summary>
        /// Start the service.
        /// </summary>
        protected override void StartService()
        {
            TraceCallback = new TraceToConsoleCallback();

            switch (Config.TraceLevel)
            {
                case TraceLevels.None:
                    TraceCallback.EnabledLevels = 0;
                    break;

                case TraceLevels.Errors:
                    TraceCallback.EnabledLevels = Trace.eLevel.knError;
                    break;

                default:
                case TraceLevels.Info:
                    TraceCallback.EnabledLevels = Trace.eLevel.knError | Trace.eLevel.knTime | Trace.eLevel.knInfo | Trace.eLevel.knUI;
                    break;

                case TraceLevels.Verbose:
                    TraceCallback.EnabledLevels = Trace.eLevel.knError | Trace.eLevel.knTime | Trace.eLevel.knInfo | Trace.eLevel.knDebug | Trace.eLevel.knMemory | Trace.eLevel.knNotify | Trace.eLevel.knUI;
                    break;
            }

            Trace.setTraceCallback(TraceCallback);
            
            var userAppPath = Config.WorkingDirectory;
            string applicationPath = Config.ApplicationDirectory;
            
            if (Environment.CurrentDirectory != applicationPath)
            {
                System.Diagnostics.Trace.WriteLine(Config.EngineName+"::CurrentDirectory was " + Environment.CurrentDirectory);

                //GDAL module require the current directory to be the same as installed application directory to find "plugins\gdal_data" directory.
                Environment.CurrentDirectory = applicationPath;

                System.Diagnostics.Trace.WriteLine(Config.EngineName + "::CurrentDirectory set to " + Environment.CurrentDirectory);
            }

            try
            {
                PYXLibInstance.initialize(Config.EngineName, Config.ClearCache, userAppPath, applicationPath, Config.CacheDirectory ?? "");
            }
            catch (Exception e)
            {
                throw new Exception("Failed to initialize pyxlib with error: " + e.Message, e);
            }

            if (Config.TraceLevel == TraceLevels.Verbose)
            {
                //we need to enable more levels
                Trace.getInstance().setLevels((uint)TraceCallback.EnabledLevels);
            }
            else
            {
                //default levels for log
                Trace.getInstance().setLevels(519);
            }

            ManagedChecksumCalculator = new ManagedChecksumCalculator(ChecksumSingleton.Checksummer);
            ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr(
                ManagedChecksumCalculator));

            ChecksumSingleton.Checksummer.ReadChecksumCache(
                AppServices.getCacheDir(
                ManagedChecksumCalculator.GetDefaultCacheDirectory()) +
                ManagedChecksumCalculator.GetDefaultCacheFilename());

            var provider = new ManagedCSharpFunctionProvider();
            CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr(
                provider));

            var xmlProvider = new ManagedXMLDocumentProvider();
            CSharpXMLDocProvider.setCSharpXMLDocProvider(new CSharpXMLDocProvider_SPtr(xmlProvider));

            var httpProvider = new ManagedHttpRequestProvider();
            HttpRequestProvider.setHttpRequestProvider(new HttpRequestProvider_SPtr(httpProvider));

            ManagedBitmapServer.StartServer();

            FileNotificationManager.getFileNeededNotifier().Event += HandleFileNeeded;

            m_initializationCompleted = true;

            //make sure we flush the log every 1 minute (make it easier to debug)
            m_traceFlushTimer = new SimpleTimer(TimeSpan.FromMinutes(1), (s, e) => { 
                Trace.flush(); 
            });
        }

        /// <summary>
        /// Stop the service.
        /// </summary>
        protected override void StopService()
        {
            if (!m_initializationCompleted)
            {
                //we failed to initialize service, stopping it would cause more errors
                return;
            }

            StopServiceSafe();
        }

        private void StopServiceSafe()
        {
            m_traceFlushTimer.Timer.Stop();
            m_traceFlushTimer = null;

            FileNotificationManager.getFileNeededNotifier().Event -= HandleFileNeeded;

            CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr());
            HttpRequestProvider.setHttpRequestProvider(new HttpRequestProvider_SPtr());
            CSharpXMLDocProvider.setCSharpXMLDocProvider(new CSharpXMLDocProvider_SPtr());
            ManagedBitmapServer.StopServer();
            ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr());
            ChecksumSingleton.Checksummer.WriteChecksumCache();

            //force GC collection to initialize swig *_SPtr object to be disposed.
            GC.Collect();
            //wait for object to be disposed (note - all *_SPtr that been referenced by .Net will still be alive)
            GC.WaitForPendingFinalizers();

            PYXLibInstance.uninitialize();
        }

        /// <summary>
        /// This is raised by the Path process when it is requested by the Coverage Cache to look for an input file 
        /// to a pipeline using the file checksum, if the file does not exist in the path specified in the Path process.
        /// </summary>
        /// <param name="spEvent">The FileEvent.</param>
        static public void HandleFileNeeded(NotifierEvent_SPtr spEvent)
        {
            var ev = FileEvent.dynamic_cast(spEvent.__deref__());
            var path = ev.getPath();
            var url = pyxlib.DynamicPointerCast_IUrl(path);
            var manifestText = url.getManifest();

            var localPath = ChecksumSingleton.Checksummer.getLocallyResolvedFilename(manifestText, ev.getIndex());
            
            if (localPath.Length > 0)
            {
                ev.setLocalPath(localPath);
                ev.setFailed(false);
            }
            else
            {
                ev.setFailed(true);
            }
        }
    }
}
