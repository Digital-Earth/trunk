using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Measurements
{
    /// <summary>
    /// Represents a measurement made of a specific type of quantity.
    /// </summary>
    public interface ITypedMeasurement
    {
        /// <summary>
        /// Gets the type of quantity being measured.
        /// </summary>
        MeasurementType MeasurementType { get; }
    }

    /// <summary>
    /// Defines a measurement taken of a quantity including the unit of measure.
    /// </summary>
    public class Measurement
    {
        /// <summary>
        /// Gets the unit of measure.
        /// </summary>
        public Unit Unit { get; private set; }
        /// <summary>
        /// Gets the amount of the measure.
        /// </summary>
        public double Amount { get; private set; }

        /// <summary>
        ///  Initializes a new instance of the Pyxis.Core.Measurements.Measurement class.
        /// </summary>
        /// <param name="amount">The amount of the measurement.</param>
        /// <param name="unit">The unit of measure.</param>
        public Measurement(double amount, Unit unit)
        {
            this.Amount = amount;
            this.Unit = unit;
        }

        /// <summary>
        /// Returns a string that represents the current Pyxis.Core.Measurements.Measurement.
        /// </summary>
        /// <returns>A string that represents the current Pyxis.Core.Measurements.Measurement.</returns>
        public override string ToString()
        {
            return String.Format("{0} [{1}]", Amount, Unit.Abbreviation);
        }
    }
}
