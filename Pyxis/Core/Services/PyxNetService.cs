using ApplicationUtility;
using Pyxis.Publishing;
using PyxNet;
using PyxNet.FileTransfer;
using PyxNet.Pyxis;
using System;

namespace Pyxis.Core.Services
{
    /// <summary>
    /// Methods for starting and stopping the PyxNetService
    /// </summary>
    public class PyxNetService : ServiceBase
    {
        private EngineConfig Config { get; set; }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="config">The EngineConfig</param>
        public PyxNetService(EngineConfig config)
        {
            Config = config;
        }

        /// <summary>
        /// Start the PyxNet service.
        /// </summary>
        protected override void StartService()
        {
            //initialize pyxnet

            //use our custom configuration if we find needed config settings
            if (Config.PyxNetNodeId != Guid.Empty)
            {
                StackSingleton.Configuration = new PyxNetStackCustomConfiguration(Config.PyxNetNodeId);
            }

            StackSingleton.Stack.NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;

            StackSingleton.Stack.NodeInfo.FriendlyName += " - " + Config.EngineName;

            StackSingleton.Stack.Connect();

            // Hook up the coverage downloader system, so that remote processes can provide data.
            CoverageDownloader.InitializeCoverageDownloaderSupport();

            // Hook up process Channel
            PyxlibPyxnetChannelProvider.StartServer();

            FileNotificationManager.getPipelineFilesDownloadNeededNotifier().Event += HandlePipelineFilesNeeded;

            if (Config.User != null)
            {
                AuthenticateAs(Config.User);
            }
            else
            {
                // attach a channel to give the stack context for the license server url
                StackSingleton.Stack.AttachChannel(new Channel(Config.APIUrl));
            }
        }

        /// <summary>
        /// Stop the PyxNet service.
        /// </summary>
        protected override void StopService()
        {
            FileNotificationManager.getPipelineFilesDownloadNeededNotifier().Event -= HandlePipelineFilesNeeded;

            //stop publishing
            PublisherSingleton.Publisher.StopPublishing();

            // stop process channel
            PyxlibPyxnetChannelProvider.StopServer();

            // stop coverage downloader
            CoverageDownloaderManager.Manager.DetachAllDownloaders();
            CoverageDownloader.StopCoverageDownloaderSupport();

            StackSingleton.Stack.Stop();
        }

        /// <summary>
        /// This is raised when a pipeline request to download all supporting files over pyxnet.
        /// For example, when a syled_feature_summary want to download bitmap pipeline for the icons.
        /// </summary>
        /// <param name="spEvent">The FileEvent.</param>
        static public void HandlePipelineFilesNeeded(NotifierEvent_SPtr spEvent)
        {
            PipelineFilesEvent ev =
                PipelineFilesEvent.dynamic_cast(spEvent.__deref__());

            foreach (var manifest in ev.getProcess().ExtractManifests())
            {
                var downloader = new ManifestDownloader(StackSingleton.Stack, manifest, null);

                if (!downloader.Download())
                {
                    ev.setFailed(true);
                    return;
                }
            }

            ev.setFailed(false);
            return;
        }

        internal void AuthenticateAs(User user)
        {
            StackSingleton.Stack.AssignUser(user);
        }
    }
}
