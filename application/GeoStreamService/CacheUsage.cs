using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using ApplicationUtility;
using System.Threading.Tasks;

namespace GeoStreamService
{
    class CacheUsage
    {
        public ProcRef ProcRef { get; private set; }

        public CacheUsage(ProcRef procRef)
        {
            ProcRef = procRef;
        }

        public long CalculateInputFilesDiskUsage()
        {
            return PipeManager.getProcess(ProcRef).SupportingFiles().Sum(x => x.Length);
        }

        public long CalculateCacheDiskSpaceUsage()
        {
            var cacheDirectories = new HashSet<string>();
            foreach (var process in PipeManager.getProcess(ProcRef).WalkPipelines())
            {
                var processIDCacheDir = PipeUtils.getProcessIdentityCacheDirectory(process);
                if (!String.IsNullOrEmpty(processIDCacheDir))
                {
                    cacheDirectories.Add(processIDCacheDir);
                }
            };

            return cacheDirectories.Sum(x => CaclulateDirectorySize(x));
        }

        private long CaclulateDirectorySize(string rootDir)
        {
            return Directory.GetFiles(rootDir, "*.*", SearchOption.AllDirectories)
                .Sum(file => new FileInfo(file).Length);
        }
    }
}
