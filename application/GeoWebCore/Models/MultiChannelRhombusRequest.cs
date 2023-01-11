using System;
using System.Collections.Generic;

namespace GeoWebCore.Models
{
    /// <summary>
    /// MultiChannelRhombusRequest allow the user to return an RGBA image and assign different GeoSource/field for each color channel
    /// </summary>
    public class MultiChannelRhombusRequest
    {
        /// <summary>
        /// Describe a request for a given channel
        /// </summary>
        public class ChannelRequest
        {
            /// <summary>
            /// Channel name is : "R","G","B","A"
            /// </summary>
            public string Channel { get; set; }

            /// <summary>
            /// Guid for the GeoSource to use
            /// </summary>
            public Guid GeoSource { get; set; }

            /// <summary>
            /// Minimum value to be used as color=0
            /// </summary>
            public double Min { get; set; }

            /// <summary>
            /// Maximum value to be used as color=255
            /// </summary>
            public double Max { get; set; }

            /// <summary>
            /// String value to match against
            /// </summary>
            public string StringValue { get; set; }
            
            /// <summary>
            /// Only needed for features
            /// </summary>
            public string Field { get; set; }
        }

        /// <summary>
        /// Rhombus key
        /// </summary>
        public string Key { get; set;}

        /// <summary>
        /// Rhombus size
        /// </summary>
        public int Size { get; set; }

        /// <summary>
        /// Chanels request
        /// </summary>
        public List<ChannelRequest> Channels { get; set; }
    }
}