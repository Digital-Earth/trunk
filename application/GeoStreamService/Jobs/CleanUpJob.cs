/******************************************************************************
CleanUpJob.cs

begin		: October 24, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using ApplicationUtility;
using Pyxis.Services.PipelineLibrary.Repositories;
using System;
using System.Collections.Concurrent;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for cleaning up files that are no longer needed
    /// </summary>
    internal class CleanUpJob : Job
    {
        public CleanUpJob()
            : base()
        {
            Status = new ObservableOperationStatus();
            Status.Description = "Clean up GWSS";
        }

        public static bool operator !=(CleanUpJob a, CleanUpJob b)
        {
            return !(a == b);
        }

        public static bool operator ==(CleanUpJob a, CleanUpJob b)
        {
            if (object.ReferenceEquals(a, null))
            {
                return object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            return obj != null && obj is CleanUpJob;
        }

        public override int GetHashCode()
        {
            return 0;
        }

        protected override void DoExecute()
        {
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Starting the clean up!");

            var freedTemp = DeleteTempFolder();
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Disk Space Freed in Local\\Temp Folder : " + FileUtility.SizeToString(freedTemp));

            var freedCache = DeleteCache();
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Disk Space Freed in Process Cache : " + FileUtility.SizeToString(freedCache));

            var freedFiles = DeleteSupportingFiles();
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Disk Space Freed in Downloaded Files : " + FileUtility.SizeToString(freedFiles));

            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Total Freed Disk Space : " + FileUtility.SizeToString(freedFiles + freedCache + freedTemp));
        }

        private long DeleteCache()
        {
            var neededFolders = new ConcurrentDictionary<string, byte>();
            foreach (var item in PipelineRepository.Instance.GetAllPipelines())
            {
                var processHead = PipeManager.getProcess(item.ProcRef);
                if (processHead.isNotNull())
                {
                    foreach (var process in processHead.WalkPipelines())
                    {
                        var processCacheDir = PipeUtils.getProcessIdentityCacheDirectory(process);
                        if (!String.IsNullOrEmpty(processCacheDir))
                        {
                            //removing the \Data from the path
                            processCacheDir = processCacheDir.Substring(0, processCacheDir.LastIndexOf("\\Data"));
                            neededFolders.TryAdd(processCacheDir, 0);
                        }
                    }
                }
            }

            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Total Needed Cache: " + FileUtility.SizeToString(neededFolders.Keys.Sum(x => FileUtility.TryGetDirectorySize(x))));

            long totalDeletedBytes = 0;

            Parallel.ForEach(Directory.EnumerateDirectories(AppServices.getCacheDir("ProcessCache")), checkSumfolderName =>
            {
                int itemsInfolder = 0;
                foreach (var item in Directory.EnumerateDirectories(checkSumfolderName))
                {
                    itemsInfolder++;
                    var folderName = Path.Combine(checkSumfolderName, item);
                    byte value;
                    if (!neededFolders.TryGetValue(folderName, out value))
                    {
                        totalDeletedBytes += FileUtility.DeleteAccessibleItems(folderName);
                        itemsInfolder--;
                    }
                }
                if (itemsInfolder == 0)
                {
                    FileUtility.DeleteAccessibleItems(checkSumfolderName);
                }
            });
            return totalDeletedBytes;
        }

        private long DeleteSupportingFiles()
        {
            //Finding necessary files
            var neededFiles = new ConcurrentDictionary<string, long>();
            foreach (var item in PipelineRepository.Instance.GetAllPipelines())
            {
                var processHead = PipeManager.getProcess(item.ProcRef);
                if (processHead.isNotNull())
                {
                    foreach (var file in processHead.SupportingFiles())
                    {
                        long size = 0;
                        FileUtility.TryGetFileSize(file.FullName, out size);
                        neededFiles.TryAdd(file.FullName, size);
                    }
                }
            }

            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Total Needed Files: " + FileUtility.SizeToString(neededFiles.Sum(x => x.Value)));

            var downloadDirectory = PyxNet.FileTransfer.ManifestDownloader.CacheDirectory;
            if (!Directory.Exists(downloadDirectory))
            {
                return 0;
            }
            //Removing unnecessary files
            long totalDeletedFiles = 0;
            Parallel.ForEach(Directory.EnumerateFiles(downloadDirectory, "*", SearchOption.AllDirectories), fileName =>
            {
                long size = 0;
                if (!neededFiles.TryGetValue(fileName, out size))
                {
                    FileUtility.TryGetFileSize(fileName, out size);
                    if (FileUtility.TryDeleteFile(fileName))
                    {
                        totalDeletedFiles += size;
                        GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Deleted: " + fileName + " file size : " + FileUtility.SizeToString(size) + " B");
                    }
                    else
                    {
                        GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Can not delete file " + fileName + " ");
                    }
                }
            });
            //Removing Empty Directories, removing subdirectories first(bottom up)
            foreach (var folder in Directory.EnumerateDirectories(downloadDirectory, "*", SearchOption.AllDirectories).OrderByDescending(x => x.Length))
            {
                if (Directory.GetFileSystemEntries(folder).Length == 0)
                {
                    FileUtility.DeleteAccessibleItems(folder);
                }
            }
            return totalDeletedFiles;
        }

        private long DeleteTempFolder()
        {
            var tempSize = FileUtility.TryGetDirectorySize(Path.GetTempPath()); // in Bytes
            var freeDiskSpace = Pyxis.Publishing.Protocol.ContractObligations.ServerStatus.CurrentStatus().AvailableDiskSpaceMB * 1048576; // in Bytes

            // Our Heuristic to clean the temp folder
            if (tempSize > freeDiskSpace)
            {
                return FileUtility.DeleteAccessibleItems(Path.GetTempPath());
            }
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.CleanUp, "Skipping Temp, folder is too small" + FileUtility.SizeToString(tempSize));
            return 0;
        }
    }
}