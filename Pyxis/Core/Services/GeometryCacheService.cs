using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using Pyxis.Utilities;

namespace Pyxis.Core.Services
{
    /// <summary>
    /// A Core service that caches the last mapping between GeoJson geometries and PYXGeometry objects
    /// </summary>
    internal class GeometryCacheService : ServiceBase
    {
        private Engine Engine { get; set; }
        private readonly object m_lock = new object();
        private LimitedSizeDictionary<string, Task<PYXGeometry_SPtr>> m_geometries;

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.Services.GeometryCacheService.
        /// </summary>
        /// <param name="engine">engine object</param>
        public GeometryCacheService(Engine engine)
        {
            Engine = engine;
        }
        
        /// <summary>
        /// Starts the Pyxis.Core.Services.PipelineService by initializing an empty process cache.
        /// </summary>
        protected override void StartService()
        {
            lock (m_lock)
            {
                m_geometries = new LimitedSizeDictionary<string, Task<PYXGeometry_SPtr>>(100);
            }
        }

        /// <summary>
        /// Stops the Pyxis.Core.Services.PipelineService by emptying the process cache.
        /// </summary>
        protected override void StopService()
        {
            lock (m_lock)
            {
                m_geometries.Clear();
                m_geometries = null;
            }
        }

        internal string KeyFromResource(IGeometry geometry, int resolution)
        {
            return JsonConvert.SerializeObject(geometry)+"," + resolution;
        }

        private Task<PYXGeometry_SPtr> CreateConversionTask(IGeometry geometry, int resolution)
        {
            return Task<PYXGeometry_SPtr>.Factory.StartNew(() => geometry.ToPyxGeometry(Engine, resolution));
        }

        /// <summary>
        /// Get a PYXGeometry from the given IGeometry.
        /// This function will try to look for that IGeometry on the cache first.
        /// If not found in Cache, it would ask the IGeometry to create a PYXGeometry and store the result for later use.
        /// </summary>
        /// <param name="geometry">IGeometry to convert into PYXGeometry</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>resulted PYXGeometry</returns>
        public PYXGeometry_SPtr ToPyxGeometry(IGeometry geometry, int resolution)
        {
            Task<PYXGeometry_SPtr> pyxGeometryCreationTask = null;
            var key = KeyFromResource(geometry, resolution);

            lock (m_lock)
            {                
                if (!m_geometries.TryGetValue(key, out pyxGeometryCreationTask))
                {
                    pyxGeometryCreationTask = CreateConversionTask(geometry, resolution);
                    m_geometries[key] = pyxGeometryCreationTask;
                }                
            }

            try
            {
                return pyxGeometryCreationTask.Result;
            }
            catch (Exception)
            {
                lock(m_lock)
                {
                    m_geometries.Remove(key);
                }
                throw;
            }
        }
    }
}
