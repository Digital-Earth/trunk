using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;

namespace Pyxis.Core.DERM
{
    /// <summary>
    /// ValueGetter is a utility to retrieve values from GeoSources
    /// </summary>
    internal class ValueGetter
    {
        private readonly Engine m_engine;
        private readonly Dictionary<Guid, ICoverage_SPtr> m_coverages = new Dictionary<Guid, ICoverage_SPtr>();
        private readonly object m_coverageLock = new object();
        
        /// <summary>
        /// Create a ValueGetter using a given Engine to initialize GeoSource.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine to use</param>
        public ValueGetter(Engine engine)
        {
            m_engine = engine;
        }

        /// <summary>
        /// Return a value for a specific cell
        /// </summary>
        /// <param name="geoSource">GeoSource to retrieve value from</param>
        /// <param name="index">Cell index to get value of</param>
        /// <param name="fieldIndex">specifies which field to use inside the GeoSource (based on field index)</param>
        /// <returns>value of the cell</returns>
        public object GetValue(GeoSource geoSource, PYXIcosIndex index, int fieldIndex = 0)
        {
            ICoverage_SPtr coverage;
            lock (m_coverageLock)
            {
                if (!m_coverages.TryGetValue(geoSource.Id, out coverage))
                {
                    coverage = m_coverages[geoSource.Id] = pyxlib.QueryInterface_ICoverage(m_engine.GetProcess(geoSource).getOutput());;
                }
            }

            return coverage.getCoverageValue(index, fieldIndex).ToDotNetObject();
        }
    }
}
