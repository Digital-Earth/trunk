/******************************************************************************
PipelineExtentions.cs

begin      : 25/10/2013 2:53:03 PM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Pyxis.Utilities;

namespace ApplicationUtility
{
    /// <summary>
    /// This class contains extentions methods for IProcess_SPtr
    /// </summary>
    public static class PipelineExtentions
    {
        /// <summary>
        /// Iterates over the files needed for the pipeline ending with process and are locally resolved
        /// </summary>
        /// <param name="process"></param>
        /// <returns>An IEnumerable&lt;System.IO.FileInfo&gt; over all the supporting files of the pipeline ending with process</returns>
        public static IEnumerable<FileInfo> SupportingFiles(this IProcess_SPtr process)
        {
            var supportingFiles = new List<string>();
            foreach (var childProc in WalkPipelines(process).Where(x => x.getSpec().providesOutputType(IPath.iid)).Distinct())
            {
                childProc.SupportingFilesSingle(supportingFiles);
            }
            return supportingFiles.Distinct().Select(x => new System.IO.FileInfo(x));
        }

        /// <summary>
        /// Enumerates the files needed by and are locally resolved in the processes collection (not necessarily a single pipeline)
        /// </summary>
        /// <param name="processes"></param>
        /// <returns>An IEnumerable&lt;System.IO.FileInfo&gt; over all the supporting files of the processes</returns>
        public static IEnumerable<FileInfo> SupportingFiles(this IEnumerable<IProcess_SPtr> processes)
        {
            var supportingFiles = new List<string>();
            foreach (var process in processes.Where(x => x.getSpec().providesOutputType(IPath.iid)).Distinct())
            {
                process.SupportingFilesSingle(supportingFiles);
            }
            return supportingFiles.Distinct().Select(x => new System.IO.FileInfo(x));
        }

        private static void SupportingFilesSingle(this IProcess_SPtr process, List<string> supportingFiles)
        {
            process.initProc(true);
            if (process.getInitState() == IProcess.eInitStatus.knInitialized && process.getSpec().providesOutputType(IPath.iid))
            {

                var path = pyxlib.QueryInterface_IPath(process);
                for (int index = 0; index < path.getLength(); ++index)
                {
                    string filename = path.getLocallyResolvedPath(index);
                    if (System.IO.File.Exists(filename))
                    {
                        supportingFiles.Add(filename);
                    }
                }
            }
            else
            {
                System.Console.WriteLine(process.getInitError().getError());
            }
        }

        /// <summary>
        /// Iterates over the files referenced by the process
        /// </summary>
        /// <param name="process"></param>
        /// <returns></returns>
        public static IEnumerable<Manifest> ExtractManifests(this IProcess_SPtr process)
        {
            foreach (var child in process.WalkPipelines())
            {
                foreach (var manifest in child.ExtractSingleManifest())
                {
                    yield return manifest;
                }
            }
        }

        public static IEnumerable<Manifest> ExtractManifests(this IEnumerable<IProcess_SPtr> processes)
        {
            foreach (var process in processes)
            {
                foreach (var manifest in process.ExtractSingleManifest())
                {
                    yield return manifest;
                }
            }
        }

        private static IEnumerable<Manifest> ExtractSingleManifest(this IProcess_SPtr process)
        {
            IUrl_SPtr url = pyxlib.QueryInterface_IUrl(process);
            if ((url != null) && (url.isNotNull()))
            {
                Manifest manifest = null;
                try
                {
                    string manifestText = url.getManifest();
                    manifest = XmlTool.FromXml<Manifest>(manifestText);
                }
                catch
                {
                }

                if (manifest != null)
                {
                    yield return manifest;
                }
            }
        }

        public static IEnumerable<IProcess_SPtr> GetChildProcesses(this IProcess_SPtr process)
        {
            foreach (var parameter in process.getParameters())
            {
                for (var valueIndex = 0; valueIndex < parameter.getValueCount(); ++valueIndex)
                {
                    yield return parameter.getValue(valueIndex);
                }
            }
        }

        /// <summary>
        /// Recursively iterates over the processes that this process use as input
        /// </summary>
        /// <param name="process"></param>
        /// <returns></returns>
        public static IEnumerable<IProcess_SPtr> WalkPipelines(this IProcess_SPtr process)
        {
            if ((process != null) && (process.isNotNull()))
            {
                yield return process;

                foreach(var childProcess in WalkChildrenPipelines(process, WalkPipelines))
                {
                    yield return childProcess;
                }
            }
        }

        /// <summary>
        /// Recursively iterates over the processes that this process use as input
        /// Stops when a child process is a GeoPacketSource
        /// </summary>
        /// <param name="process"></param>
        /// <returns></returns>
        public static IEnumerable<IProcess_SPtr> WalkPipelinesSkipGeoPacketSources(this IProcess_SPtr process)
        {
            if ((process != null) && (process.isNotNull()))
            {
                yield return process;

                if (!process.getSpec().providesOutputType(IGeoPacketSource.iid))
                {
                    foreach(var childProcess in WalkChildrenPipelines(process, WalkPipelinesSkipGeoPacketSources))
                    {
                        yield return childProcess;
                    }
                }
            }
        }

        /// <summary>
        /// Recursively iterates over the processes that this process use as input, excluding the process itself
        /// Stops when a child process is a GeoPacketSource
        /// </summary>
        /// <param name="process"></param>
        /// <returns></returns>
        public static IEnumerable<IProcess_SPtr> WalkPipelinesSkipGeoPacketSourcesAfterParent(this IProcess_SPtr process)
        {
            if ((process != null) && (process.isNotNull()))
            {
                foreach (var childProcess in WalkChildrenPipelines(process, WalkPipelinesSkipGeoPacketSources))
                {
                    yield return childProcess;
                }
            }
        }

        private static IEnumerable<IProcess_SPtr> WalkPipelinesExcludeGeoPacketSources(this IProcess_SPtr process)
        {
            if ((process != null) && (process.isNotNull()) && !process.getSpec().providesOutputType(IGeoPacketSource.iid))
            {
                yield return process;
                foreach (var childProcess in WalkChildrenPipelines(process, WalkPipelinesExcludeGeoPacketSources))
                {
                    yield return childProcess;
                }
            }
        }

        /// <summary>
        /// Recursively iterates over the processes that this process use as input, unconditionally including the process itself
        /// Stops when a child process is a GeoPacketSource
        /// </summary>
        /// <param name="process"></param>
        /// <returns></returns>
        public static IEnumerable<IProcess_SPtr> WalkPipelinesExcludeGeoPacketSourcesAfterParent(this IProcess_SPtr process)
        {
            if ((process != null) && (process.isNotNull()))
            {
                yield return process;
                foreach (var childProcess in WalkChildrenPipelines(process, WalkPipelinesExcludeGeoPacketSources))
                {
                    yield return childProcess;
                }
            }
        }

        private static IEnumerable<IProcess_SPtr> WalkChildrenPipelines(this IProcess_SPtr process, Func<IProcess_SPtr, IEnumerable<IProcess_SPtr>> TraversalMethod)
        {
            foreach (var child in process.GetChildProcesses().SelectMany(x => TraversalMethod(x)))
            {
                yield return child;
            }

            // check if process has embedded resources
            var embeddedResources = pyxlib.QueryInterface_IEmbeddedResourceHolder(process);
            if ((embeddedResources != null) && (embeddedResources.isNotNull()))
            {
                //if does, extract all manifests from each of it's resources
                for (int i = 0; i < embeddedResources.getEmbeddedResourceCount(); i++)
                {
                    foreach (var childProcess in TraversalMethod(embeddedResources.getEmbeddedResource(i)))
                    {
                        yield return childProcess;
                    }
                }
            }
        }

        public static IEnumerable<IProcess_SPtr> Distinct(this IEnumerable<IProcess_SPtr> processes)
        {
            var seenKeys = new HashSet<ProcRef>();
            foreach (var process in processes)
            {
                if (seenKeys.Add(new ProcRef(process)))
                {
                    yield return process;
                }
            }
        }

        public static IEnumerable<IProcess_SPtr> GeoPacketSources(this IProcess_SPtr process)
        {
            return WalkPipelines(process).GeoPacketSources();
        }

        public static IEnumerable<IProcess_SPtr> ImmediateGeoPacketSources(this IProcess_SPtr process)
        {
            return WalkPipelinesSkipGeoPacketSources(process).GeoPacketSources();
        }

        public static IEnumerable<IProcess_SPtr> GeoPacketSources(this IEnumerable<IProcess_SPtr> processes)
        {
            return processes.Where(x => x.getSpec().providesOutputType(IGeoPacketSource.iid)).Distinct().ToList();
        }
    }
}