using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Core.IO.GeoJson.Specialized;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// A utility for getting Pyxis.Core.IO.GeoJson.Feature in a PYXIS coverage process.
    /// </summary>
    public class CoverageGetter
    {
        private IProcess_SPtr m_process;
        private ICoverage_SPtr m_coverage;

        internal CoverageGetter(IProcess_SPtr process)
        {
            m_process = process;
            m_coverage = pyxlib.QueryInterface_ICoverage(m_process.getOutput());
        }

        /// <summary>
        /// Get the values of fields in a PYXIS process.
        /// </summary>
        /// <param name="geometry">The Pyxis.Core.IO.IGeometry specifying the area of interest.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection of the field values in <paramref name="geometry"/>.</returns>
        public FeatureCollection GetValue(IGeometry geometry)
        {
            var result = new FeatureCollection() { Features = new List<Feature>() };

            var cell = geometry as CellGeometry;

            if (cell == null)
            {
                return result;
            }

            if (!m_coverage.getGeometry().intersects(PYXCell.create(new PYXIcosIndex(cell.Index)).__deref__()))
            {
                return result;
            }

            var properties = new Dictionary<string, object>();
            int i = 0;
            foreach (var field in m_coverage.getCoverageDefinition().FieldDefinitions)
            {
                var name = field.getName();
                var value = m_coverage.getCoverageValue(new PYXIcosIndex(cell.Index), i);
                properties[name] = value.ToDotNetObject();
                i++;
            }

            result.Features.Add(new Feature(null, cell, properties));
            return result;
        }
    }
}
