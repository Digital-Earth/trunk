using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Threading.Tasks;
using ApplicationUtility;
using GeoWebCore.Services.Cache;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.MultiDomain;
using Pyxis.Utilities;
using File = System.IO.File;

namespace GeoWebCore.Services
{
    internal static class GeoSourceInitializer
    {
        private static readonly object s_lock = new object();

        private const int ProcessCacheSize = 100;

        private static readonly LimitedSizeDictionary<string, IProcess_SPtr> s_processes =
            new LimitedSizeDictionary<string, IProcess_SPtr>(ProcessCacheSize);

        private static readonly InMemoryGeoSourceCache s_inMemoryGeoSourceCache = new InMemoryGeoSourceCache();

        private static readonly LimitedSizeDictionary<Guid, IGeoSourceState> s_geoSourceStateCache = new LimitedSizeDictionary<Guid, IGeoSourceState>(100);

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

        public static Engine Engine { get; set; }

        public enum LocalGeoSourceType
        {
            Temporary,
            Persistent
        }

        private const string LocalCacheTemporaryPath = "temporary";
        private const string LocalCachePersistentPath = "persistent";

        private static string GeoLocalGeoSourceDirectory(LocalGeoSourceType type)
        {
            if (String.IsNullOrEmpty(s_localGeoSourceCacheFolder))
            {
                throw new Exception("GeoSourceInitialize has not been initialized yet");
            }

            if (type == LocalGeoSourceType.Persistent)
            {
                return Path.Combine(s_localGeoSourceCacheFolder, LocalCachePersistentPath);
            }
            else
            {
                return Path.Combine(s_localGeoSourceCacheFolder, LocalCacheTemporaryPath);
            }
        }

        private static string GetGeoSourceLocalPath(Guid id, LocalGeoSourceType type)
        {
            return Path.Combine(GeoLocalGeoSourceDirectory(type), id + ".json");
        }

        private static string GetGeoSourceResolverLocalPath(Guid id, LocalGeoSourceType type)
        {
            return Path.Combine(GeoLocalGeoSourceDirectory(type), id + ".resolver.json");
        }

        public static IProcess_SPtr InitializeLocalGeoSource(GeoSource geoSource)
        {
            var process = Initialize(geoSource);
            WriteLocalGeoSourceToDisk(geoSource);
            return process;
        }

        private static void WriteLocalGeoSourceToDisk(GeoSource geoSource)
        {
            var path = GetGeoSourceLocalPath(geoSource.Id, LocalGeoSourceType.Temporary);
            var tempPath = path + "." + DateTime.Now.Ticks;

            try
            {
                File.WriteAllText(tempPath, JsonConvert.SerializeObject(geoSource));
                if (File.Exists(path))
                {
                    File.Delete(path);
                }
                File.Move(tempPath, path);
            }
            finally
            {
                if (File.Exists(tempPath))
                {
                    File.Delete(tempPath);
                }
            }
        }


        public static GeoSource ResolveGeoSourceDomain(Guid id, Dictionary<string,object> domains)
        {
            foreach (var type in new[] {LocalGeoSourceType.Temporary, LocalGeoSourceType.Persistent})
            {
                var path = GetGeoSourceResolverLocalPath(id, type);

                if (!File.Exists(path))
                {
                    continue;
                }

                var resolver = JsonConvert.DeserializeObject<StaticMultiDomianGeoSourceResolver>(File.ReadAllText(path));
                var resource = resolver.Resolve(domains);

                if (resource != null)
                {
                    //TODO: we should also deal with version here...
                    return GetGeoSource(resource.Id);
                }
            }

            return null;
        }

        private static bool TryLoadLocalGeoSourceFromDisk(Guid id,out GeoSource geoSource) 
        {
            foreach (var type in new[] {LocalGeoSourceType.Temporary, LocalGeoSourceType.Persistent})
            {
                var path = GetGeoSourceLocalPath(id, type);
                if (File.Exists(path))
                {
                    var resolverPath = GetGeoSourceResolverLocalPath(id, type);

                    if (File.Exists(resolverPath))
                    {
                        geoSource = JsonConvert.DeserializeObject<MultiDomainGeoSource>(File.ReadAllText(path));
                    }
                    else
                    {
                        geoSource = JsonConvert.DeserializeObject<GeoSource>(File.ReadAllText(path));
                    }
                    
                    return geoSource != null;
                }
            }
            geoSource = null;
            return false;
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

                var process = Engine.GetProcess(geoSource);

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
                s_inMemoryGeoSourceCache.Clear();
                s_geoSourceStateCache.Clear();
            }
        }

        internal static GeoSource GetGeoSource(Guid id)
        {
            return s_inMemoryGeoSourceCache.GetGeoSource(id);
        }

        private static GeoSource ResolveGeoSourceFromLocalCache(Guid id)
        {
            GeoSource geoSource;
            if (TryLoadLocalGeoSourceFromDisk(id, out geoSource))
            {
                return geoSource;
            }
            return null;
        }

        internal static void Initialize(Engine engine, EngineConfig config)
        {
            Engine = engine;

            //help PYXIS Engine to resolve local stored geosources
            config.ResourceReferenceResolvers.Add((reference) =>
            {
                //let GeoSourceInitializer locate this resource
                var resource = GeoSourceInitializer.GetGeoSource(reference.Id);

                //verify this is the right version
                if (resource != null && resource.Version == reference.Version)
                {
                    return resource;
                }

                return null;
            });

            //initalize InMemoryGeoSourceCache
            //step1: resolve localy created GeoSources
            s_inMemoryGeoSourceCache.AssignResolver(ResolveGeoSourceFromLocalCache);

            //step2: resolve geosources from gallery
            s_inMemoryGeoSourceCache.AssignResolver(id => Engine.GetChannel().GeoSources.GetById(id));

            s_localGeoSourceCacheFolder = AppServices.getCacheDir("localGeoSource");

            Directory.CreateDirectory(GeoLocalGeoSourceDirectory(LocalGeoSourceType.Persistent));
            Directory.CreateDirectory(GeoLocalGeoSourceDirectory(LocalGeoSourceType.Temporary));
        }

        /// <summary>
        /// Invalidate the cache for a specific GeoSource
        /// </summary>
        /// <param name="id">Id for the GeoSource</param>
        /// <param name="minAge">minium age to invalidate the geo-source (to avoid refreshing several times in a period of time)</param>
        public static bool InvalidateGeoSource(Guid id, TimeSpan minAge)
        {
            if (!s_inMemoryGeoSourceCache.InvalidateGeoSource(id, minAge))
            {
                return false;
            }
            lock (s_lock)
            {
                s_geoSourceStateCache.Remove(id);
            }
            return true;
        }

        public static Task<List<string>> BoradcastGeoSourceInvalidation(Guid id)
        {
            InvalidateGeoSource(id, TimeSpan.FromSeconds(1));

            var cluster = Program.RunInformation.Cluster;
            return cluster.Broadcast<string>(cluster.ServersRing, "/api/v1/GeoSource/" + id + "/Refresh");
        }
    }
}