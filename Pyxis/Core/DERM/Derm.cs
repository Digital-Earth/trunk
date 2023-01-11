using System.Collections.Generic;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.Measurements;

namespace Pyxis.Core.DERM
{
    /// <summary>
    /// Derm provides methods for enumerating cells on the Pyxis.Core.DERM
    /// </summary>
    public class Derm
    {
        private readonly Engine m_engine;
        internal ValueGetter ValueGetter { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="engine">The engine.</param>
        public Derm(Engine engine)
        {
            m_engine = engine;
            ValueGetter = new ValueGetter(m_engine);
        }

        internal IEnumerable<Cell> CellsInGeometry(PYXGeometry_SPtr pyxGeometry)
        {
            var iterator = pyxGeometry.getIterator();

            while (!iterator.end())
            {
                var newIndex = new PYXIcosIndex(iterator.getIndex());
                yield return new Cell(this, newIndex);
                iterator.next();
            }
        }

        /// <summary>
        /// Enumerate all cells inside an IGeometry
        /// </summary>
        /// <param name="geometry">IGeometry to use as enumeration.</param>
        /// <returns>Enumerable set of cells</returns>
        public IEnumerable<Cell> CellsInGeometry(IGeometry geometry)
        {
            var pyxGeometry = geometry.ToPyxGeometry(m_engine);

            return CellsInGeometry(pyxGeometry);
        }

        /// <summary>
        /// Enumerate all cells inside an IGeometry Mark at a specified resolution.
        /// </summary>
        /// <param name="geometry">IGeometry to use as enumeration.</param>
        /// <param name="resolution">Pyxis resolution to use to enumerate cells on</param>
        /// <returns>Enumerable set of cells</returns>
        public IEnumerable<Cell> CellsInGeometry(IGeometry geometry, int resolution)
        {
            var pyxGeometry = geometry.ToPyxGeometry(m_engine).clone();
            pyxGeometry.setCellResolution(resolution);
            return CellsInGeometry(pyxGeometry);
        }

        /// <summary>
        /// Create a Cell object using a given Pyxis Index.
        /// </summary>        
        /// <param name="index">Pyxis Index to use.</param>
        public Cell Cell(string index)
        {
            return new Cell(this,new PYXIcosIndex(index));
        }

        public Cell Cell(GeographicPosition center, int resolution)
        {
            return new Cell(this, center.ToPointLocation().asPYXIcosIndex(resolution));
        }

        public IEnumerable<Cell> PrimeCells()
        {
            var iterator = PYXIcosIterator.create(1);
            while (!iterator.end())
            {
                yield return new Cell(this, iterator.getIndex());
                iterator.next();
            }
        }

        public int ResolutionFromDistance(double distance)
        {
            var resolution = SnyderProjection.getInstance().precisionToResolution((distance * SphericalDistance.Meter).InRadians);
            return resolution;
        }

        public int ResolutionFromDistance(SphericalDistance distance)
        {
            var resolution = SnyderProjection.getInstance().precisionToResolution(distance.InRadians);
            return resolution;
        }
    }
}
