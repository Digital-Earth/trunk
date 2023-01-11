using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Measurements
{
    internal static class SphericalAreaUnits
    {
        public static readonly Unit SquareMeter = new Unit(MeasurementType.SphericalArea, "Square Meter", "m2");
        public static readonly Unit Hectare = SquareMeter.DefineWithRatio(10000, "Hectare", "ha");
        public static readonly Unit SquareKiloMeter = SquareMeter.DefineWithRatio(1000 * 1000, "Square KiloMeter", "km2");
    }

    /// <summary>
    /// A Pyxis.Core.Measurements.Measurement of spherical area.
    /// </summary>
    public class SphericalArea : Measurement
    {
        /// <summary>
        /// Gets the spherical area Pyxis.Core.Measurements.MeasurementType.
        /// </summary>
        public MeasurementType MeasurementType
        {
            get { return MeasurementType.SphericalArea; }
        }

        private SphericalArea(Unit unit)
            : base(1, unit)
        {
        }

        private SphericalArea(double amount, Unit unit)
            : base(amount, unit)
        {
        }

        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalArea measurement with units in square meter.
        /// </summary>
        public static readonly SphericalArea SquareMeter = new SphericalArea(SphericalAreaUnits.SquareMeter);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalArea measurement with units in square kilometer.
        /// </summary>
        public static readonly SphericalArea SquareKiloMeter = new SphericalArea(SphericalAreaUnits.SquareKiloMeter);
        
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalArea measurement with units in hectare.
        /// </summary>
        public static readonly SphericalArea Hectare = new SphericalArea(SphericalAreaUnits.Hectare);
        
        /// <summary>
        /// Convert a Pyxis.Core.Measurements.SphericalArea measurement to another with different Pyxis.Core.Measurements.Unit.
        /// </summary>
        /// <param name="other">A Pyxis.Core.Measurements.SphericalArea measurement containing the Pyxis.Core.Measurements.Unit to convert this instance.</param>
        /// <returns>The converted Pyxis.Core.Measurements.SphericalArea measurement.</returns>
        public SphericalArea As(SphericalArea other)
        {
            return new SphericalArea(this.Unit.ConvertAmountTo(Amount, other.Unit), other.Unit);
        }

        /// <summary>
        /// Convert the Amount of the current instance to an amount in different Pyxis.Core.Measurements.Unit.
        /// </summary>
        /// <param name="other">The Pyxis.Core.Measurements.SphericalArea measurement containing the Pyxis.Core.Measurements.Unit to convert this instance's Amount.</param>
        /// <returns>The converted amount.</returns>
        public double AmountAs(SphericalArea other)
        {
            return this.Unit.ConvertAmountTo(Amount, other.Unit);
        }

        /// <summary>
        /// Gets the Amount of this instance in square meter.
        /// </summary>
        public double InSquareMeters
        {
            get
            {
                return AmountAs(SquareMeter);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in square kilometers.
        /// </summary>
        public double InSquareKiloMeters
        {
            get
            {
                return AmountAs(SquareKiloMeter);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in hectares.
        /// </summary>
        public double InHectares
        {
            get
            {
                return AmountAs(Hectare);
            }
        }

        /// <summary>
        /// Adds two specified Pyxis.Core.Measurements.SphericalArea instances.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>A Pyxis.Core.Measurements.SphericalArea representing the sum of <paramref name="a"/> and <paramref name="b"/> using the Pyxis.Core.Measurements.Unit of <paramref name="a"/>.</returns>
        public static SphericalArea operator +(SphericalArea a, SphericalArea b)
        {
            return new SphericalArea(a.Amount + b.Unit.ConvertAmountTo(b.Amount, a.Unit), a.Unit);
        }

        /// <summary>
        /// Subtracts two specified Pyxis.Core.Measurements.SphericalArea instances.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>A Pyxis.Core.Measurements.SphericalArea representing the difference of <paramref name="a"/> and <paramref name="b"/> using the Pyxis.Core.Measurements.Unit of <paramref name="a"/>.</returns>
        public static SphericalArea operator -(SphericalArea a, SphericalArea b)
        {
            return new SphericalArea(a.Amount - b.Unit.ConvertAmountTo(b.Amount, a.Unit), a.Unit);
        }

        /// <summary>
        /// Create a Pyxis.Core.Measurements.SphericalArea by multiplying an existing instance amount by a scalar.
        /// </summary>
        /// <param name="a">The scaling factor.</param>
        /// <param name="b">The Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>The scaled Pyxis.Core.Measurements.SphericalArea.</returns>
        public static SphericalArea operator *(double a, SphericalArea b)
        {
            return new SphericalArea(a * b.Amount, b.Unit);
        }

        /// <summary>
        /// Create a Pyxis.Core.Measurements.SphericalArea by dividing an existing instance amount by a scalar.
        /// </summary>
        /// <param name="a">The Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The divisor.</param>
        /// <returns>The scaled Pyxis.Core.Measurements.SphericalArea.</returns>
        /// <exception cref="System.DivideByZeroException">The divisor is zero.</exception>
        public static SphericalArea operator /(SphericalArea a, double b)
        {
            return new SphericalArea(a.Amount / b, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is less than another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is less than <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator <(SphericalArea a, SphericalArea b)
        {
            return a.Amount < b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is greater than another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is greater than <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator >(SphericalArea a, SphericalArea b)
        {
            return a.Amount > b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is less than or equal to another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is less than or equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator <=(SphericalArea a, SphericalArea b)
        {
            return a.Amount <= b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is greater than or equal to another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is greater than or equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator >=(SphericalArea a, SphericalArea b)
        {
            return a.Amount >= b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is equal to another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator ==(SphericalArea a, SphericalArea b)
        {
            return a.Amount == b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalArea is not equal to another specified Pyxis.Core.Measurements.SphericalArea.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalArea instance.</param>
        /// <returns>true if <paramref name="a"/> is not equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator !=(SphericalArea a, SphericalArea b)
        {
            return a.Amount != b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether this instance and a specified System.Object represent the same type and measurement.
        /// </summary>
        /// <param name="obj">The object to compare with this instance.</param>
        /// <returns>true if <paramref name="obj"/> is a Pyxis.Core.Measurements.SphericalArea and equal to this instance; otherwise, false.</returns>
        public override bool Equals(object obj)
        {
            if (obj is SphericalArea)
            {
                return this == (SphericalArea)obj;
            }
            return false;
        }

        /// <summary>
        /// Returns the hash code for this instance.
        /// </summary>
        /// <returns>A 32-bit signed integer hash code.</returns>
        public override int GetHashCode()
        {
            return Amount.GetHashCode() ^ Unit.GetHashCode();
        }
    }
}
