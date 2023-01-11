using System;
using System.Collections.Generic;

namespace PyxisCLI.Server.Models
{
    /// <summary>
    /// Icons Model is a simplified version of the vector data that allow client to display icons.
    /// </summary>
    public class IconsModel
    {
        /// <summary>
        /// The GeoSource that was used to generate the icons data.
        /// </summary>
        public Guid GeoSource { get; set; }

        /// <summary>
        /// The features group ID that was used to generate the icons data.
        /// </summary>
        public string GroupId { get; set; }
        
        /// <summary>
        /// List of all sub features groups.
        /// </summary>
        public List<GroupIcon> Groups { get; set; }

        /// <summary>
        /// List of all icons in the feature group.
        /// </summary>
        public List<FeatureIcon> Icons { get; set; }

        /// <summary>
        /// Basic icon model.
        /// 
        /// An icon is a point on the unit sphere (X,Y,Z) and a Radius.
        /// Moreover, each icon has an "ID" associated with it, for later reference.
        /// </summary>
        public class Icon
        {
            /// <summary>
            /// Id of the icon
            /// </summary>
            public string Id { get; set; }

            /// <summary>
            /// The X coordinate to place the icon (on unit shpere)
            /// </summary>
            public double X { get; set; }

            /// <summary>
            /// The Y coordinate to place the icon (on unit shpere)
            /// </summary>
            public double Y { get; set; }

            /// <summary>
            /// The Z coordinate to place the icon (on unit shpere)
            /// </summary>
            public double Z { get; set; }

            /// <summary>
            /// Radius for the icon, in terms of the data that is coverd by the icon.
            /// </summary>
            public double Radius { get; set; }
        }

        /// <summary>
        /// Icon model for a feature.
        /// 
        /// The Icon ID is the feature ID.
        /// 
        /// Moreover, list of feature the properties is returned.
        /// </summary>
        public class FeatureIcon : Icon
        {
            /// <summary>
            /// Data Properties assoicated with the icon
            /// </summary>
            public Dictionary<string, object> Values { get; set; }
        }

        /// <summary>
        /// Icon model for a features group.
        /// 
        /// The Icon ID is the feature group ID.
        /// 
        /// Moreover, the size of the feature group and list of feature group the properties is returned.
        /// </summary>
        public class GroupIcon : Icon
        {
            /// <summary>
            /// How many features are represented by the IconGroup
            /// </summary>
            public int Count { get; set; }

            /// <summary>
            /// Data Properties assoicated with the icon group. each value have Min,Max,Average fields under it.
            /// </summary>
            public Dictionary<string, object> Values { get; set; }
        }        
    }
}