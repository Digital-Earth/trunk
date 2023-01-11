using System;
using System.Collections.Generic;
using System.Drawing;
using System.Threading.Tasks;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using Pyxis.Utilities;
using PyxisCLI.Server.Cluster;

namespace PyxisCLI.Server.Services
{
    internal static class GeoSourceInitializer
    {
        private static readonly object s_lock = new object();

        private const int ProcessCacheSize = 100;

        private static readonly LimitedSizeDictionary<string, IProcess_SPtr> s_processes =
            new LimitedSizeDictionary<string, IProcess_SPtr>(ProcessCacheSize);


        private static readonly Dictionary<Guid, GeoSource> s_geoSources = new Dictionary<Guid, GeoSource>();

        private static readonly LimitedSizeDictionary<Guid, IGeoSourceState> s_geoSourceStateCache = new LimitedSizeDictionary<Guid, IGeoSourceState>(ProcessCacheSize);

        public static IGeoSourceState GetGeoSourceState(Guid id)
        {
            lock (s_lock)
            {
                IGeoSourceState state;
                if (s_geoSourceStateCache.TryGetValue(id, out state))
                {
                    return state;
                }

                state = new GeoSourceState(id);
                s_geoSourceStateCache[id] = state;
                return state;
            }
        }

        private static string s_localGeoSourceCacheFolder;

        private static string GetProcessKey(Guid guid)
        {
            return guid.ToString();
        }

        private static string GetProcessKey(Guid guid, Style style)
        {
            return JsonConvert.SerializeObject(new {GeoSource = guid, Style = style});
        }

        static GeoSourceInitializer()
        {
            Program.Engine.BeforeStopping(Deinitialize);
        }

        /// <summary>
        /// Initialize a GeoSource object into a IProcess_SPtr.
        /// </summary>
        /// <param name="geoSource">GeoSource to initialize.</param>
        /// <returns>IProcess_SPtr to perform analysis on.</returns>
        public static IProcess_SPtr Initialize(GeoSource geoSource)
        {
            var key = GetProcessKey(geoSource.Id);

            lock (s_lock)
            {
                if (s_processes.ContainsKey(key))
                {
                    if (s_processes[key].isNotNull())
                    {
                        return new IProcess_SPtr(s_processes[key]);
                    }
                    s_processes.Remove(key);
                }

                s_geoSources[geoSource.Id] = geoSource;

                var process = Program.Engine.GetProcess(geoSource);

                if (process == null || process.isNull())
                {
                    throw new Exception("Engine.GetProcess() return null for GeoSource " + geoSource.Id);
                }

                s_processes[key] = process;

                return new IProcess_SPtr(process);
            }
        }

        /// <summary>
        /// Initialize a GeoSource using a given GeoSource ID object into a IProcess_SPtr.
        /// </summary>
        /// <param name="id">GeoSource ID to initialize.</param>
        /// <returns>IProcess_SPtr to perform analysis on.</returns>
        public static IProcess_SPtr Initialize(Guid id)
        {
            var key = GetProcessKey(id);

            lock (s_lock)
            {
                if (s_processes.ContainsKey(key))
                {
                    if (s_processes[key].isNotNull())
                    {
                        return new IProcess_SPtr(s_processes[key]);
                    }
                    s_processes.Remove(key);
                }

                var geoSource = GetGeoSource(id);
                if (geoSource == null)
                {
                    throw new Exception("Failed to find a GeoSource with Id " + id);
                }

                return Initialize(geoSource);
            }
        }

        /// <summary>
        /// Initialize a GeoSource and make sure it can be treated as Coverage process.
        /// 
        /// This function is useful when the only thing we want to do is to raster the data.
        /// </summary>
        /// <param name="id">GeoSource ID to initialize.</param>
        /// <returns>IProcess_SPtr to be used for rastering.</returns>
        public static IProcess_SPtr InitializeAsCoverage(Guid id)
        {
            var key = GetProcessKey(id) + "-as-coverage";

            lock (s_lock)
            {
                if (s_processes.ContainsKey(key))
                {
                    if (s_processes[key].isNotNull())
                    {
                        return new IProcess_SPtr(s_processes[key]);
                    }
                    s_processes.Remove(key);
                }

                var process = Initialize(id);

                if (process.ProvidesOutputType(ICoverage.iid))
                {
                    s_processes[key] = process;
                    return new IProcess_SPtr(process);
                }
                if (process.ProvidesOutputType(IFeatureCollection.iid))
                {
                    var coverage = CoverageFromFeatureCollection(process);
                    s_processes[key] = coverage;
                    return new IProcess_SPtr(coverage);
                }
                throw new Exception("Unsupported process output type");
            }
        }

        /// <summary>
        /// Initialize a GeoSource and make sure it can be treated as Coverage process.
        /// 
        /// This function is useful when the only thing we want to do is to raster the data.
        /// </summary>
        /// <param name="id">GeoSource ID to initialize.</param>
        /// <param name="style">Style to be used for rastering.</param>
        /// <returns>IProcess_SPtr to be used for rastering.</returns>
        public static IProcess_SPtr InitializeAsCoverage(Guid id, Style style)
        {
            var key = GetProcessKey(id, style);

            lock (s_lock)
            {
                if (s_processes.ContainsKey(key))
                {
                    return new IProcess_SPtr(s_processes[key]);
                }

                var process = Initialize(id);

                if (process.ProvidesOutputType(ICoverage.iid))
                {
                    s_processes[key] = process;
                    return new IProcess_SPtr(process);
                }
                if (process.ProvidesOutputType(IFeatureCollection.iid))
                {
                    var coverage = CoverageFromFeatureCollection(process, style);
                    s_processes[key] = coverage;
                    return new IProcess_SPtr(coverage);
                }
                throw new Exception("Unsupported process output type");
            }
        }

        private static IProcess_SPtr CoverageFromFeatureCollection(IProcess_SPtr featureCollection, Style style = null)
        {
            if (style == null)
            {
                style = new Style
                {
                    Fill = FieldStyle.FromColor(Color.White),
                    Line = FieldStyle.FromColor(Color.Gray)
                };
            }

            var styleProcess = style.ApplyStyle(featureCollection);

            var rasterizer =
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.StyledFeaturesRasterizer)
                    .AddInput(0, styleProcess)
                    .AddAttribute("UseAlpha", "1");
            var rasterizerProcess = PYXCOMFactory.CreateProcess(rasterizer);

            var styledFeatures =
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                    .AddInput(0, rasterizerProcess);
            var process = PYXCOMFactory.CreateProcess(styledFeatures);

            process.initProc(true);
            
            return process;
        }

        public static void Deinitialize()
        {
            lock (s_lock)
            {
                s_processes.Clear();
                s_geoSourceStateCache.Clear();
            }
        }

        internal static GeoSource GetGeoSource(Guid id)
        {
            lock (s_lock)
            {
                if (s_geoSources.ContainsKey(id))
                {
                    return s_geoSources[id];
                }
            }
            
            throw new Exception(String.Format("GeoSource not found (Id={0}",id));
        }

        /// <summary>
        /// Invalidate the cache for a specific GeoSource
        /// </summary>
        /// <param name="id">Id for the GeoSource</param>
        /// <param name="minAge">minium age to invalidate the geo-source (to avoid refreshing several times in a period of time)</param>
        public static bool InvalidateGeoSource(Guid id, TimeSpan minAge)
        {
            lock (s_lock)
            {
                s_geoSourceStateCache.Remove(id);
            }
            return true;
        }

        public static Task<List<string>> BoradcastGeoSourceInvalidation(Guid id)
        {
            InvalidateGeoSource(id, TimeSpan.FromSeconds(1));

            var cluster = Program.Cluster;
            return new HttpClusterProxy(cluster).Broadcast<string>(cluster.ServersRing, "/api/v1/GeoSource/" + id + "/Refresh");
        }
    }
}