using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Pyxis.Contract.Publishing;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Provides globe annotation mouse event data.
    /// </summary>
    public class GlobeAnnotationMouseEventArgs : MouseEventArgs
    {
        private readonly IFeaturesIdsProvider m_featuresIdProvider;

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Globe.GlobeAnnotationMouseEventArgs class.
        /// </summary>
        /// <param name="args">System.Windows.Forms.MouseEventArgs providing mouse event data.</param>
        /// <param name="styledGeoSource">They Pyxis.UI.Layers.Globe.StyledGeoSource the annotation event occurred on.</param>
        /// <param name="featureIds">Identifier of the annotated features.</param>
        public GlobeAnnotationMouseEventArgs(MouseEventArgs args, StyledGeoSource styledGeoSource, List<string> featureIds)
            : base(args.Button, args.Clicks, args.X, args.Y, args.Delta)
        {
            m_styledGeoSource = styledGeoSource;
            m_featuresIdProvider = new ListFeaturesIdsProvider(featureIds);
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Globe.GlobeAnnotationMouseEventArgs class.
        /// </summary>
        /// <param name="args">System.Windows.Forms.MouseEventArgs providing mouse event data.</param>
        /// <param name="styledGeoSource">They Pyxis.UI.Layers.Globe.StyledGeoSource the annotation event occurred on.</param>
        /// <param name="featureIdsProvider">Identifier of the annotated features.</param>
        public GlobeAnnotationMouseEventArgs(MouseEventArgs args, StyledGeoSource styledGeoSource, IFeaturesIdsProvider featureIdsProvider)
            : base(args.Button, args.Clicks, args.X, args.Y, args.Delta)
        {
            m_styledGeoSource = styledGeoSource;
            m_featuresIdProvider = featureIdsProvider;
        }

        private readonly StyledGeoSource m_styledGeoSource;

        /// <summary>
        /// Gets the count of annotated features.
        /// </summary>
        public int FeaturesCount
        {
            get
            {
                return m_featuresIdProvider.FeatureCount;
            }
        }

        /// <summary>
        /// Gets the features identifiers of the annotation.
        /// </summary>
        /// <returns>The features identifiers.</returns>
        public IEnumerable<string> GetFeaturesIds()
        {
            return m_featuresIdProvider.GetFeaturesId();
        }

        /// <summary>
        /// Gets the Pyxis.UI.Layers.Globe.GeoSource the annotation event occurred on.
        /// </summary>
        public GeoSource GeoSource { get { return m_styledGeoSource.GeoSource; } }
        /// <summary>
        /// Gets the identifier of the Pyxis.UI.Layers.Globe.StyledGeoSource in the Pyxis.UI.Layers.Globe.ViewState.
        /// </summary>
        public Guid ViewStateId { get { return m_styledGeoSource.Id; } }
    }

    /// <summary>
    /// Interface allowing retrieval of features identifiers of the annotation. 
    /// </summary>
    public interface IFeaturesIdsProvider
    {
        /// <summary>
        /// Gets the count of annotated features.
        /// </summary>
        int FeatureCount { get; }

        /// <summary>
        /// Gets the features identifiers of the annotation.
        /// </summary>
        /// <returns>The features identifiers.</returns>
        IEnumerable<string> GetFeaturesId();
    }

    /// <summary>
    /// Implementing IFeaturesIdsProvider from a list of features.
    /// </summary>
    internal class ListFeaturesIdsProvider : IFeaturesIdsProvider
    {
        private readonly List<string> m_featuresId;

        public ListFeaturesIdsProvider(List<string> featuresIds)
        {
            m_featuresId = featuresIds;
        }

        public int FeatureCount
        {
            get
            {
                return m_featuresId.Count;
            }
        }

        public IEnumerable<string> GetFeaturesId()
        {
            return m_featuresId;
        }
    }

    /// <summary>
    /// Implementing IFeaturesIdsProvider from a feature group.
    /// This class would do lazy featuresIds extraction to allow efficient queries like GetFeaturesId().Skip(10).Take(10).
    /// </summary>
    internal class FeaturesGroupFeaturesIdProvider : IFeaturesIdsProvider
    {
        private readonly IFeatureGroup_SPtr m_group;

        public FeaturesGroupFeaturesIdProvider(IFeatureGroup_SPtr group)
        {
            m_group = group;
        }

        public int FeatureCount
        {
            get 
            {
                return m_group.getFeaturesCount().max;
            }
        }

        public IEnumerable<string> GetFeaturesId()
        {
            var iterator = m_group.getIterator();
            while (!iterator.end())
            {
                yield return iterator.getFeature().getID();
                iterator.next();
            }
        }
    }
}