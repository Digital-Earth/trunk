using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Measurements
{
    /// <summary>
    /// Describes a unit of measure.
    /// </summary>
    public class Unit
    {
        /// <summary>
        /// Gets the name.
        /// </summary>
        public string Name { get; private set;}
        /// <summary>
        /// Gets the abbreviation.
        /// </summary>
        public string Abbreviation { get; private set; }
        /// <summary>
        /// Gets the type of quantity being measured.
        /// </summary>
        public MeasurementType MeasurementType { get; private set; }

        private double RatioToBaseUnit { get; set; }

        internal Unit(MeasurementType measurementType, string name, string abbreviation, double ratio)
        {
            MeasurementType = measurementType;
            Abbreviation = abbreviation;
            Name = name;
            RatioToBaseUnit = ratio;
        }

        internal Unit(MeasurementType measurementType, string name, string abbreviation)
            : this(measurementType, name, abbreviation, 1)
        {
        }

        /// <summary>
        /// Use the current Pyxis.Core.Measurements.Unit to define a new unit of measure using a conversion ratio.
        /// </summary>
        /// <param name="ratio">The conversion ratio to convert from the new unit of measure to the current one.</param>
        /// <param name="name">The name of the new unit of measure.</param>
        /// <param name="abbreviation">The abbreviation of the new unit of measure.</param>
        /// <returns>The new Pyxis.Core.Measurements.Unit.</returns>
        public Unit DefineWithRatio(double ratio, string name, string abbreviation)
        {
            return new Unit(MeasurementType, name, abbreviation, ratio * RatioToBaseUnit);            
        }

        /// <summary>
        /// Convert an amount measured using a Pyxis.Core.Measurements.Unit to the current unit of measure.
        /// </summary>
        /// <param name="amount">The amount to convert.</param>
        /// <param name="other">The Pyxis.Core.Measurements.Unit of <paramref name="amount"/>.</param>
        /// <returns>The value of <paramref name="amount"/> using the current Pyxis.Core.Measurements.Unit.</returns>
        /// <exception cref="System.InvalidCastException">Pyxis.Core.Measurements.Unit must have the same MeasurementType to convert.</exception>
        public double ConvertAmountTo(double amount, Unit other)
        {
            if (MeasurementType != other.MeasurementType)
            {
                throw new InvalidCastException("Can't convert unit " + Name + " into " + other.Name);
            }
            return amount * this.RatioToBaseUnit / other.RatioToBaseUnit;
        }

        /// <summary>
        /// Returns a value indicating whether this instance is equal to a specified object.
        /// </summary>
        /// <param name="obj">An object to compare with this instance.</param>
        /// <returns>true if <paramref name="obj"/> is a Pyxis.Core.Measurements.Unit object that represents the same unit of measure as the current Pyxis.Core.Measurements.Unit instance; otherwise, false.</returns>
        public override bool Equals(object obj)
        {
            if (Object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if (obj is Unit)
            {
                var otherUnit = obj as Unit;

                return RatioToBaseUnit == otherUnit.RatioToBaseUnit && MeasurementType == otherUnit.MeasurementType;
            }
            return false;
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>A 32-bit signed integer hash code.</returns>
        public override int GetHashCode()
        {
            return RatioToBaseUnit.GetHashCode() ^ MeasurementType.GetHashCode();
        }
    }
}
