using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;
using Pyxis.Contract.Publishing;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using Pyxis.Storage.FileSystemStorage;
using File = System.IO.File;

namespace StudioLauncher
{
    public class ProductVersionManager
    {
        private readonly string m_executableName;
        private readonly ProductType m_productType;
        private readonly IVersionServer m_remoteVersionServer;
        private readonly IVersionServer m_localVersionServer;
        private readonly string m_pyxisLocalAppDataFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Pyxis");

        public ProductVersionManager(ProductType productType, string executableName, IVersionServer remoteVersionServer, IVersionServer localVersionServer)
        {
            m_productType = productType;
            m_executableName = executableName;
            m_remoteVersionServer = remoteVersionServer;
            m_localVersionServer = localVersionServer;
        }

        /// <summary>
        /// Check if an update is available from the remote server.
        /// </summary>
        /// <returns>true if an update is available, otherwise false.</returns>
        public bool UpdateAvailable()
        {
            try
            {
                Trace.TraceInformation("Checking if update is available.");

                var serverProduct = m_remoteVersionServer.GetLatestVersion(m_productType);
                if (serverProduct == null)
                {
                    Trace.TraceInformation("No server version available.");
                    return false;
                }
                else
                {
                    Trace.TraceInformation("Server version is: {0}", serverProduct.ProductVersion);
                }

                var localProduct = m_localVersionServer.GetLatestVersion(m_productType);
                if (localProduct == null)
                {
                    Trace.TraceInformation("No local product available.");
                    return true;
                }
                else
                {
                    Trace.TraceInformation("Local version is: {0}", localProduct.ProductVersion);
                }

                return localProduct.ProductVersion.CompareTo(serverProduct.ProductVersion) != 0;
            }
            catch (Exception e)
            {
                throw new Exception("Unable to check for updates: " + e.Message, e);
            }
        }

        /// <summary>
        /// Get the path to the product executable on user's computer. Has a side effect of
        /// renaming the folder from "ProductName Version" to "ProductName" if a
        /// "ProductName Version" folder exists that matches the current version.
        /// </summary>
        /// <returns>The path to the latest product executable.</returns>
        public string GetLocalProductExecutablePath()
        {
            var latestLocalVersion = m_localVersionServer.GetLatestVersion(m_productType);
            if (latestLocalVersion != null)
            {
                var freshlyDownloaded = Path.Combine(m_pyxisLocalAppDataFolder, DirectoryName(latestLocalVersion));
                Trace.TraceInformation("Downloaded directory is {0}.", freshlyDownloaded);

                var workingVersion = Path.Combine(m_pyxisLocalAppDataFolder, m_productType.ToString());
                Trace.TraceInformation("Working directory is {0}.", workingVersion);

                var oldVersion = workingVersion + "-old";
                Trace.TraceInformation("Old directory is {0}.", oldVersion);

                if (Directory.Exists(freshlyDownloaded))
                {
                    if (!Directory.Exists(workingVersion) || FileSystemUtilities.TryMoveDirectory(workingVersion, oldVersion, true))
                    {
                        if (FileSystemUtilities.TryMoveDirectory(freshlyDownloaded, workingVersion, true))
                        {
                            Trace.TraceInformation("Successfully moved to the new version.");

                            // delete the previous working version
                            if (Directory.Exists(oldVersion))
                            {
                                FileSystemUtilities.TryDeleteDirectory(oldVersion);
                            }
                        }
                        else if (Directory.Exists(oldVersion) && !FileSystemUtilities.TryMoveDirectory(oldVersion, workingVersion, true))
                        {
                            Trace.TraceInformation("Failed to fall back to the old version.");
                        }
                    }
                    else
                    {
                        Trace.TraceInformation("Unable to move to the new version.");
                    }
                }

                if (Directory.Exists(workingVersion))
                {
                    return Path.Combine(workingVersion, m_executableName);
                }
                else
                {
                    m_localVersionServer.SetLatestVersion(new Product() { ProductType = m_productType });
                }
            }

            return "";
        }

        /// <summary>
        /// Get the local directory name for the product.
        /// </summary>
        /// <param name="product">The product.</param>
        /// <returns>The local directory name.</returns>
        public static string DirectoryName(Product product)
        {
            return product.ProductType.ToString() + " " + product.ProductVersion;
        }

        /// <summary>
        /// Get all available versions from the server.
        /// </summary>
        /// <returns></returns>
        public List<Product> GetAllVersions()
        {
            try
            {
                Trace.TraceInformation("Checking if update is available.");

                var serverProducts = m_remoteVersionServer.GetAllVersions(m_productType);
                if (serverProducts == null)
                {
                    Trace.TraceInformation("Unable to get server versions.");
                }

                return serverProducts;
            }
            catch (Exception e)
            {
                throw new Exception("Unable to get server versions: " + e.Message, e);
            }
        }

        /// <summary>
        /// Try to run the latest local version.
        /// </summary>
        /// <param name="args">Command line arguments to pass to the executable.</param>
        /// <returns>true if the product was run, otherwise false.</returns>
        public bool TryRunTheLocalVersion(string[] args)
        {
            Trace.TraceInformation("Trying to run the local version.");

            var localExe = GetLocalProductExecutablePath();
            return TryRunLocalExe(localExe, args);
        }

        /// <summary>
        /// Try to run the specified product/version from the user's computer.
        /// </summary>
        /// <param name="product">the product/version to run.</param>
        /// <param name="args">Command line arguments to pass to the executable.</param>
        /// <returns>true if the product was run, otherwise false</returns>
        public bool TryRunTheLocalVersion(Product product, string[] args)
        {
            Trace.TraceInformation("Trying to run specific version: " + product.ProductVersion);

            var localFolder = Path.Combine(m_pyxisLocalAppDataFolder, DirectoryName(product));
            var localExe =  Path.Combine(localFolder, m_executableName);
            return TryRunLocalExe(localExe, args);
        }

        /// <summary>
        /// Try to run the specified executable from the user's computer.
        /// </summary>
        /// <param name="localExe">Path to the local executable.</param>
        /// <param name="args">Command line arguments to pass to the executable.</param>
        /// <returns>true if the executable was run, otherwise false</returns>
        public bool TryRunLocalExe(string localExe, string[] args)
        {
            Trace.TraceInformation("Trying to run: " + localExe);

            if (localExe != "" && File.Exists(localExe))
            {
                try
                {
                    Process.Start(localExe, string.Join(" ", args));
                }
                catch (Exception e)
                {
                    Trace.TraceError("Unable to run: " + localExe);
                    Trace.TraceError("Exception: " + e.Message);
                    Trace.TraceError("Stack trace: " + e.StackTrace);
                    return false;
                }

                Trace.TraceInformation("Running: " + localExe);
                return true;
            }
            else
            {
                Trace.TraceError("Unable to run: " + localExe);
                Trace.TraceError("File does not exist.");
                return false;
            }
        }

        /// <summary>
        /// Download and optionally run the latest update.
        /// </summary>
        /// <param name="runInBackground">If false, displays a progress dialog and runs the application after downloading.</param>
        /// <param name="args">Command line arguments to pass to the executable.</param>
        public void DownloadUpdate(bool runInBackground, string[] args)
        {
            var latestVersion = m_remoteVersionServer.GetLatestVersion(m_productType);
            if (latestVersion == null)
            {
                Trace.TraceInformation("No server version available.");
                return;
            }

            Trace.TraceInformation("Getting latest version from server: {0}", latestVersion.ProductVersion);

            DownloadVersion(latestVersion, runInBackground);

            m_localVersionServer.SetLatestVersion(latestVersion);

            if (!runInBackground)
            {
                if (!TryRunTheLocalVersion(args))
                {
                    throw new Exception("Unable to run latest version of product.");
                }
            }
        }

        /// <summary>
        /// Download and run the specified product/version. Display a progress dialog.
        /// </summary>
        /// <param name="product">The product/version to download and run.</param>
        /// <param name="args">Command line arguments to pass to the executable.</param>
        public void DownloadAndRunVersion(Product product, string[] args)
        {
            DownloadVersion(product, false);
            Trace.TraceInformation("Getting version from server: {0}", product.ProductVersion);

            if (!TryRunTheLocalVersion(product, args))
            {
                throw new Exception(String.Format("Unable to run version {0} of product.", product.ProductVersion));
            }
        }

        /// <summary>
        /// Download the specified product/version. Optionally display a progress dialog.
        /// If latest is true, upon successful download, this version becomes the current
        /// local version of the product.
        /// </summary>
        /// <param name="product">The product/version to download.</param>
        /// <param name="runInBackground">If true, do not display a progress dialog</param>
        public void DownloadVersion(Product product, bool runInBackground)
        {
            Trace.TraceInformation("Getting version from server: {0}", product.ProductVersion);

            if (product.TransferType == TransferType.BlobClientV1)
            {
                IBlobProvider blobProvider = new CachedBlobProvider(product.Url, Path.Combine(m_pyxisLocalAppDataFolder, "BlobCache"));
                var client = new FileSystemStorage(blobProvider);
                var expectedName = Path.Combine(m_pyxisLocalAppDataFolder, DirectoryName(product));
                var tracker = client.DownloadDirectoryAsync(product.Key, expectedName);
                DownloadProgressUI progressUI = null;
                if (!runInBackground)
                {
                    progressUI = new DownloadProgressUI();
                    tracker.ProgressMade += progressTracker => progressUI.Progress = progressTracker.Percent;
                }
                var continuation = tracker.Task.ContinueWith(
                    t =>
                    {
                        if (t.Result)
                        {
                            if (!runInBackground && (progressUI != null))
                            {
                                if (progressUI.InvokeRequired)
                                {
                                    progressUI.Invoke(new Action(progressUI.Close));
                                }
                                else
                                {
                                    progressUI.Close();
                                }
                            }
                        }
                        else
                        {
                            throw new Exception("Unable to download update.");
                        }
                    });

                if (!runInBackground)
                {
                    Application.Run(progressUI);
                }
                continuation.Wait();
            }
            else
            {
                throw new Exception(String.Format("Unsupported transfer type: {0}.", product.TransferType));
            }
        }
    }
}