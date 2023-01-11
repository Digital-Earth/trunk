using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Publishing
{
    /// <summary>
    /// Encapsulates statistics about a PYXIS pipeline field.
    /// </summary>
    public class FieldStatistics
    {
        /// <summary>
        /// Gets or sets the minimum count in a distribution of field values.
        /// </summary>
        public double? MinCount { get; set; }
        /// <summary>
        /// Gets or sets the maximum count in a distribution of field values.
        /// </summary>
        public double? MaxCount { get; set; }
        /// <summary>
        /// Gets or sets the minimum field value.
        /// </summary>
        public object Min { get; set; }
        /// <summary>
        /// Gets or sets the maximum field value.
        /// </summary>
        public object Max { get; set; }
        /// <summary>
        /// Gets or sets the average field value.
        /// </summary>
        public object Average { get; set; }
        /// <summary>
        /// Gets or sets the sum of all field value.
        /// </summary>
        public object Sum { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.FieldStatistics.Distribution of the field values.
        /// </summary>
        public Distribution Distribution { get; set; }
    }

    /// <summary>
    /// A binned distribution of values.
    /// </summary>
    /// <seealso cref="Pyxis.Core.IO.Distribution.Bin"/>
    public class Distribution
    {
        /// <summary>
        /// Gets or sets the histogram storing the bins in the distribution.
        /// </summary>
        public List<Bin> Histogram { get; set; }

        /// <summary>
        /// Represents an estimate of the values grouped together into a single, continuous region along a spectrum of possible values.
        /// </summary>
        /// <remarks>
        /// Bins are associated with a range of possible values.
        /// Min is The lower range bound that marks the smallest possible value that will go in the bin while Max is the upper range bound representing the largest possible value.
        /// The total number of values in a bin may be uncertain, so instead of having a single count of the number of entries, there are bounds: MinCount and MaxCount.
        /// MinCount is the lower bound on the number of values and MaxCount is the upper bound on the number of values.
        /// The Frequency is the probability that a value in the distribution is in the bin, i.e. has a value between Min and Max, inclusive.
        /// </remarks>
        public class Bin
        {
            /// <summary>
            /// Gets or sets the minimum boundary of the bin.
            /// </summary>
            public object Min { get; set; }
            /// <summary>
            /// Gets or sets the maximum boundary of the bin.
            /// </summary>
            public object Max { get; set; }
            /// <summary>
            /// Gets or sets the frequency of values in the bin.
            /// </summary>
            public double Frequency { get; set; }
            /// <summary>
            /// Gets or sets a lower bound on the frequency of values in the bin.
            /// </summary>
            public double MinCount { get; set; }
            /// <summary>
            /// Gets or sets an upper bound on the frequency of values in the bin.
            /// </summary>
            public double MaxCount { get; set; }
        }
    }
}
