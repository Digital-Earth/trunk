using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// A utility for getting Pyxis.Core.IO.GeoJson.Feature in a PYXIS feature process.
    /// </summary>
    public class FeatureGetter
    {
        private readonly Engine m_engine;
        private readonly IProcess_SPtr m_process;
        private readonly IFeatureCollection_SPtr m_featureCollection;

        internal FeatureGetter(Engine engine,IProcess_SPtr process)
        {
            m_engine = engine;
            m_process = process;
            m_featureCollection = pyxlib.QueryInterface_IFeatureCollection(m_process.getOutput());
        }

        public int GetFeaturesCount()
        {
            var m_group = pyxlib.QueryInterface_IFeatureGroup(m_featureCollection);

            if (m_group.isNotNull())
            {
                return m_group.getFeaturesCount().max;
            }
            else
            {
                throw new Exception("Pipeline not support feature count, enumarte features is required.");
            }            
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.FeatureCollection of all the features in the PYXIS process.
        /// </summary>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection of all the features in the PYXIS process.</returns>
        public FeatureCollection GetFeatures(FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            return GetFeatures(null, extractionFlags);
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.Feature for a given feature based on the ID in the PYXIS process.
        /// </summary>
        /// <param name="id">The Feature ID</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns></returns>
        public Feature GetFeature(string id, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            return Feature.FromIFeature(m_featureCollection.getFeature(id),extractionFlags);
        }

        /// <summary>
        /// Enumerate all features in a GeoSource.
        /// </summary>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns></returns>
        public IEnumerable<Feature> EnumerateFeatures(FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            return EnumerateFeatures(null, extractionFlags);
        }

        /// <summary>
        /// Enumerate all features in a GeoSource that intersect the given geometry
        /// </summary>
        /// <param name="geometry">IGeometry to check intersection against</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns></returns>
        public IEnumerable<Feature> EnumerateFeatures(IGeometry geometry, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            if (geometry == null)
            {
                return m_featureCollection.GetFeaturesEnumerator().Select(x=>Feature.FromIFeature(x, extractionFlags));                
            }
            else
            {
                var pyxGeometry = m_engine.ToPyxGeometry(geometry).__deref__();
                return m_featureCollection.GetFeaturesEnumerator(pyxGeometry).Select(x=>Feature.FromIFeature(x, extractionFlags));                                
            }            
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.FeatureCollection of all the features in the PYXIS process intersecting a Pyxis.Core.IO.IGeometry.
        /// </summary>
        /// <param name="geometry">The Pyxis.Core.IO.IGeometry to use for specifying the area of interest.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection of all the features in the PYXIS process intersecting <paramref name="geometry"/>.</returns>
        public FeatureCollection GetFeatures(IGeometry geometry,FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {            
            return new FeatureCollection()
            {
                Features = EnumerateFeatures(geometry,extractionFlags).ToList()
            };
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.FeatureCollection of all the features in the PYXIS process intersecting a Pyxis.Core.IO.IGeometry.
        /// </summary>
        /// <param name="geometry">The Pyxis.Core.IO.IGeometry to use for specifying the area of interest.</param>
        /// <param name="skip">The number of Pyxis.Core.IO.GeoJson.Feature to skip.</param>
        /// <param name="take">The number of Pyxis.Core.IO.GeoJson.Feature to take.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection taking features in the PYXIS process intersecting <paramref name="geometry"/> after skipping a specified number of features.</returns>
        public FeatureCollection GetFeatures(IGeometry geometry, int skip, int take, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(m_process.getOutput());
            if (geometry == null)
            {
                return FeatureCollection.FromPYXFeatureIterator(m_engine, featureCollection.getIterator(), skip, take, null, extractionFlags);
            }
            else
            {
                var pyxGeometry = m_engine.ToPyxGeometry(geometry).__deref__();
                return FeatureCollection.FromPYXFeatureIterator(m_engine, featureCollection.getIterator(pyxGeometry), skip, take, pyxGeometry, extractionFlags);
            }
        }

        /// <summary>
        /// Search for Pyxis.Core.IO.GeoJson.Feature matching search criteria.
        /// </summary>
        /// <param name="query">The Pyxis.Core.Analysis.FeaturesSearchQuery specifying the search criteria</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection of features matching the specified search criteria.</returns>
        public FeatureCollection Search(FeaturesSearchQuery query, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            return Search(query.SearchString, query.FieldsToSearch, query.Geometry, query.Skip ?? 0, query.Take ?? 10);
        }

        /// <summary>
        /// Search for Pyxis.Core.IO.GeoJson.Feature matching search criteria.
        /// </summary>
        /// <param name="searchString">The search string.</param>
        /// <param name="fields">The names of the fields to search.</param>
        /// <param name="geometry">The Pyxis.Core.IO.IGeometry to specify the area of interest for the search.</param>
        /// <param name="skip">The number of Pyxis.Core.IO.GeoJson.Feature to skip.</param>
        /// <param name="take">The number of Pyxis.Core.IO.GeoJson.Feature to take.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>A Pyxis.Core.IO.GeoJson.FeatureCollection of features matching the specified search criteria.</returns>
        public FeatureCollection Search(string searchString, List<string> fields, IGeometry geometry, int skip, int take, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(m_process.getOutput());
            var fieldsIndices = fields.Select(x => featureCollection.getFeatureDefinition().getFieldIndex(x)).ToList();

            var indexProcess = PYXCOMFactory.CreateProcess(new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeatureCollectionIndex)
                    .AddInput(0, m_process)
                    .AddAttribute("FieldsIndices", String.Join(" ", fieldsIndices)));

            if (indexProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                return new FeatureCollection() { Features = new List<Feature>() };
            }

            var index = pyxlib.QueryInterface_IFeatureCollectionIndex(indexProcess.getOutput());

            if (geometry == null)
            {
                return FeatureCollection.FromPYXFeatureIterator(m_engine, index.getIterator(new PYXValue(searchString)), skip, take, null, extractionFlags);
            }
            else
            {
                var pyxGeometry = m_engine.ToPyxGeometry(geometry).__deref__();
                return FeatureCollection.FromPYXFeatureIterator(m_engine, index.getIterator(pyxGeometry, new PYXValue(searchString)), skip, take, pyxGeometry, extractionFlags);
            }
        }
    }
}
