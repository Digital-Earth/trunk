using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Measurements
{
    internal static class SphericalDistanceUnits
    {
        public static readonly Unit Meter = new Unit(MeasurementType.SphericalDistance, "Meter", "m");
        public static readonly Unit KiloMeter = Meter.DefineWithRatio(1000, "KiloMeter", "km");
        
        public static readonly Unit Inch = Meter.DefineWithRatio(0.0254, "Inch", "in");
        public static readonly Unit Foot = Meter.DefineWithRatio(0.3048, "Foot", "ft");
        public static readonly Unit Yard = Meter.DefineWithRatio(0.9144, "Yard", "yd");
        public static readonly Unit Mile = Meter.DefineWithRatio(1609.344, "Mile", "mil");

        public static readonly Unit Radian = Meter.DefineWithRatio(6371007, "Radian", "rad");
        public static readonly Unit Degree = Radian.DefineWithRatio(Math.PI / 180, "Degree", "deg");
    }

    /// <summary>
    /// A Pyxis.Core.Measurements.Measurement of spherical distance.
    /// </summary>
    public class SphericalDistance : Measurement
    {
        /// <summary>
        /// Gets the spherical distance Pyxis.Core.Measurements.MeasurementType.
        /// </summary>
        public MeasurementType MeasurementType
        {
            get { return MeasurementType.SphericalDistance; }
        }

        private SphericalDistance(Unit unit)
            : base(1, unit)
        {
        }

        private SphericalDistance(double amount, Unit unit)
            : base(amount, unit)
        {
        }

        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in meters.
        /// </summary>
        public static readonly SphericalDistance Meter = new SphericalDistance(SphericalDistanceUnits.Meter);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in kilometers.
        /// </summary>
        public static readonly SphericalDistance KiloMeter = new SphericalDistance(SphericalDistanceUnits.KiloMeter);
        
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in inches.
        /// </summary>
        public static readonly SphericalDistance Inch = new SphericalDistance(SphericalDistanceUnits.Inch);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in feet.
        /// </summary>
        public static readonly SphericalDistance Foot = new SphericalDistance(SphericalDistanceUnits.Foot);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in yards.
        /// </summary>
        public static readonly SphericalDistance Yard = new SphericalDistance(SphericalDistanceUnits.Yard);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in miles.
        /// </summary>
        public static readonly SphericalDistance Mile = new SphericalDistance(SphericalDistanceUnits.Mile);

        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in radians.
        /// </summary>
        public static readonly SphericalDistance Radian = new SphericalDistance(SphericalDistanceUnits.Radian);
        /// <summary>
        /// A Pyxis.Core.Measurements.SphericalDistance measurement with units in degrees (arc degrees).
        /// </summary>
        public static readonly SphericalDistance Degree = new SphericalDistance(SphericalDistanceUnits.Degree);

        /// <summary>
        /// Convert a Pyxis.Core.Measurements.SphericalDistance measurement to another with different Pyxis.Core.Measurements.Unit.
        /// </summary>
        /// <param name="other">A Pyxis.Core.Measurements.SphericalDistance measurement containing the Pyxis.Core.Measurements.Unit to convert this instance.</param>
        /// <returns>The converted Pyxis.Core.Measurements.SphericalDistance measurement.</returns>
        public SphericalDistance As(SphericalDistance other)
        {
            return new SphericalDistance(this.Unit.ConvertAmountTo(Amount, other.Unit), other.Unit);
        }

        /// <summary>
        /// Convert the Amount of the current instance to an amount in different Pyxis.Core.Measurements.Unit.
        /// </summary>
        /// <param name="other">The Pyxis.Core.Measurements.SphericalDistance measurement containing the Pyxis.Core.Measurements.Unit to convert this instance's Amount.</param>
        /// <returns>The converted amount.</returns>
        public double AmountAs(SphericalDistance other)
        {
            return this.Unit.ConvertAmountTo(Amount, other.Unit);
        }

        /// <summary>
        /// Gets the Amount of this instance in meters.
        /// </summary>
        public double InMeters
        {
            get
            {
                return AmountAs(Meter);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in kilometers.
        /// </summary>
        public double InKiloMeters
        {
            get
            {
                return AmountAs(KiloMeter);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in inches.
        /// </summary>
        public double InInches
        {
            get
            {
                return AmountAs(Inch);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in feet.
        /// </summary>
        public double InFeet
        {
            get
            {
                return AmountAs(Foot);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in yards.
        /// </summary>
        public double InYards
        {
            get
            {
                return AmountAs(Yard);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in miles.
        /// </summary>
        public double InMiles
        {
            get
            {
                return AmountAs(Mile);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in radians.
        /// </summary>
        public double InRadians
        {
            get
            {
                return AmountAs(Radian);
            }
        }

        /// <summary>
        /// Gets the Amount of this instance in degrees (arc degrees).
        /// </summary>
        public double InDegrees
        {
            get
            {
                return AmountAs(Degree);
            }
        }

        /// <summary>
        /// Adds two specified Pyxis.Core.Measurements.SphericalDistance instances.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>A Pyxis.Core.Measurements.SphericalDistance representing the sum of <paramref name="a"/> and <paramref name="b"/> using the Pyxis.Core.Measurements.Unit of <paramref name="a"/>.</returns>
        public static SphericalDistance operator +(SphericalDistance a, SphericalDistance b)
        {
            return new SphericalDistance(a.Amount + b.Unit.ConvertAmountTo(b.Amount, a.Unit), a.Unit);
        }

        /// <summary>
        /// Subtracts two specified Pyxis.Core.Measurements.SphericalDistance instances.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>A Pyxis.Core.Measurements.SphericalDistance representing the difference of <paramref name="a"/> and <paramref name="b"/> using the Pyxis.Core.Measurements.Unit of <paramref name="a"/>.</returns>
        public static SphericalDistance operator -(SphericalDistance a, SphericalDistance b)
        {
            return new SphericalDistance(a.Amount - b.Unit.ConvertAmountTo(b.Amount, a.Unit), a.Unit);
        }

        /// <summary>
        /// Create a Pyxis.Core.Measurements.SphericalDistance by multiplying an existing instance amount by a scalar.
        /// </summary>
        /// <param name="a">The scaling factor.</param>
        /// <param name="b">The Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>The scaled Pyxis.Core.Measurements.SphericalDistance.</returns>
        public static SphericalDistance operator *(double a, SphericalDistance b)
        {
            return new SphericalDistance(a * b.Amount, b.Unit);
        }

        /// <summary>
        /// Create a Pyxis.Core.Measurements.SphericalDistance by dividing an existing instance amount by a scalar.
        /// </summary>
        /// <param name="a">The Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The divisor.</param>
        /// <returns>The scaled Pyxis.Core.Measurements.SphericalDistance.</returns>
        /// <exception cref="System.DivideByZeroException">The divisor is zero.</exception>
        public static SphericalDistance operator /(SphericalDistance a, double b)
        {
            return new SphericalDistance(a.Amount / b, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is less than another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is less than <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator <(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount < b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is greater than another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is greater than <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator >(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount > b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is less than or equal to another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is less than or equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator <=(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount <= b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is greater than or equal to another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is greater than or equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator >=(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount >= b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is equal to another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator ==(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount == b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether a specified Pyxis.Core.Measurements.SphericalDistance is not equal to another specified Pyxis.Core.Measurements.SphericalDistance.
        /// </summary>
        /// <param name="a">The first Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <param name="b">The second Pyxis.Core.Measurements.SphericalDistance instance.</param>
        /// <returns>true if <paramref name="a"/> is not equal to <paramref name="b"/>; otherwise, false.</returns>
        public static bool operator !=(SphericalDistance a, SphericalDistance b)
        {
            return a.Amount != b.Unit.ConvertAmountTo(b.Amount, a.Unit);
        }

        /// <summary>
        /// Returns a value indicating whether this instance and a specified System.Object represent the same type and measurement.
        /// </summary>
        /// <param name="obj">The object to compare with this instance.</param>
        /// <returns>true if <paramref name="obj"/> is a Pyxis.Core.Measurements.SphericalDistance and equal to this instance; otherwise, false.</returns>
        public override bool Equals(object obj)
        {
            if (obj is SphericalDistance)
            {
                return this == (SphericalDistance)obj;
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
