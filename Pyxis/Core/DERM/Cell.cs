using System.Collections.Generic;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.Measurements;

namespace Pyxis.Core.DERM
{
    /// <summary>
    /// Cell provides access to the Pyxis DERM by iterating over cells and retrieving values from GeoSources
    /// </summary>
    public class Cell
    {
        private readonly Derm m_derm;
        private readonly PYXIcosIndex m_index;
        
        internal Cell(Derm derm, PYXIcosIndex index)
        {
            m_derm = derm;
            m_index = index;
        }     

        /// <summary>
        /// Create a Cell object using a given Pyxis Index.
        /// </summary>
        /// <param name="derm">Derm to use to retrieve values from GeoSources.</param>
        /// <param name="index">Pyxis Index to use.</param>
        public Cell(Derm derm, string index)
            : this(derm, new PYXIcosIndex(index))
        {
        }

        /// <summary>
        /// Return the value retrieved for a given GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to use.</param>
        /// <returns>Value for the given cell</returns>
        public object ValueOf(GeoSource geoSource)
        {
            return m_derm.ValueGetter.GetValue(geoSource, m_index);
        }

        /// <summary>
        /// Return the value retrieved for a given GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to use.</param>
        /// <param name="fieldIndex">Specific field to use for the given GeoSource</param>
        /// <returns>Value for the given cell</returns>
        public object ValueOf(GeoSource geoSource, int fieldIndex)
        {
            return m_derm.ValueGetter.GetValue(geoSource, m_index, fieldIndex);
        }

        /// <summary>
        /// Return the value retrieved for a given GeoSource with a specific type (casting)
        /// </summary>
        /// <typeparam name="T">type of result</typeparam>
        /// <param name="geoSource">GeoSource to use.</param>
        /// <returns>Value for the given cell</returns>
        public T ValueOf<T>(GeoSource geoSource)
        {
            return (T) ValueOf(geoSource);
        }

        /// <summary>
        /// Return the value retrieved for a given GeoSource with a specific type (casting)
        /// </summary>
        /// <typeparam name="T">type of result</typeparam>
        /// <param name="geoSource">GeoSource to use.</param>
        /// <param name="fieldIndex">Specific field to use for the given GeoSource</param>
        /// <returns>Value for the given cell</returns>
        public T ValueOf<T>(GeoSource geoSource, int fieldIndex)
        {
            return (T) ValueOf(geoSource, fieldIndex);            
        }

        /// <summary>
        /// Get the Parent Cell of the current Cell
        /// </summary>
        /// <returns>Parent Cell</returns>
        public Cell Parent()
        {
            var newIndex = new PYXIcosIndex(m_index);
            newIndex.decrementResolution();
            return new Cell(m_derm, newIndex);
        }

        /// <summary>
        /// Get enumerable list of the current cell's children.
        /// </summary>
        /// <returns>Enumerable list of Cells</returns>
        public IEnumerable<Cell> Children()
        {
            var newIndices = new PYXChildIterator(m_index);
            while (!newIndices.end())
            {
                var newIndex = new PYXIcosIndex(newIndices.getIndex());
                yield return new Cell(m_derm, newIndex);

                newIndices.next();
            }            
        }

        /// <summary>
        /// Get enumerable list of the current cell's neighbors.
        /// </summary>
        /// <returns>Enumerable list of Cells</returns>
        public IEnumerable<Cell> Neighbors()
        {
            var newIndices = new PYXNeighbourIterator(m_index);
            while (!newIndices.end())
            {
                var newIndex = new PYXIcosIndex(newIndices.getIndex());
                yield return new Cell(m_derm, newIndex);

                newIndices.next();
            }
        }

        /// <summary>
        /// Get enumerable list of the current cell vertices (return as cells, whith thier center as the cell vertices).
        /// </summary>
        /// <returns>Enumerable list of Cells</returns>
        public IEnumerable<Cell> Vertices()
        {
            var newIndices = new PYXVertexIterator(m_index);
            while (!newIndices.end())
            {
                var newIndex = new PYXIcosIndex(newIndices.getIndex());
                yield return new Cell(m_derm, newIndex);

                newIndices.next();
            }
        }
        
        
        /// <summary>
        /// Get the index representing the current cell.
        /// </summary>
        public string Index
        {
            get
            {
                return m_index.ToString();
            }
        }

        /// <summary>
        /// Get the center of the cell in GeographicPosition
        /// </summary>
        public GeographicPosition Center
        {
            get
            {
                return new GeographicPosition(PointLocation.fromPYXIndex(m_index));
            }
        }

        public SphericalDistance Radius
        {
            get
            {
                return SnyderProjection.getInstance().resolutionToPrecision(m_index.getResolution()) * SphericalDistance.Radian;
            }
        }
    }
}
