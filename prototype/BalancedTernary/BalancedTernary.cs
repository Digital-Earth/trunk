using System;
using System.Collections.Generic;
using System.Text;

namespace PYXIS.Mathematics
{
    /// <summary>
    /// Represents a balanced ternary (multi-digit) number.
    /// </summary>
    // TODO: Consider making this a struct, over-riding equality and GetHashValue, 
    // or making it atomic.
    // TODO: Consider adding multiplication, division?
    public class BalancedTernary
    {
        public void Prepend(int value)
        {
            m_value.Insert(0, (sbyte)value);
        }

        public static BalancedTernary operator*( BalancedTernary v, int value)
        {
            // We can only multiply by three!
            System.Diagnostics.Trace.Assert(value == 3);

            BalancedTernary result = new BalancedTernary(v);
            if (value == 3)
                result.Prepend(0);
            return result;
        }

        /// <summary>
        /// Negates the value.  n -> -n
        /// </summary>
        public void Negate()
        {
            for (int index = 0; index < m_value.Count; ++index)
            {
                m_value[index] = (SByte) (-m_value[index]);
            }
        }

        /// <summary>
        /// Unary negation.  n -> -n
        /// </summary>
        /// <param name="val">The value to negate.</param>
        /// <returns>-val</returns>
        public static BalancedTernary operator -(BalancedTernary val)
        {
            BalancedTernary result = new BalancedTernary(val);
            result.Negate();
            return result;
        }

        /// <summary>
        /// Subraction operator.
        /// </summary>
        /// <param name="minuend">
        /// The minuend is the "left hand side" of the subtraction expression.
        /// </param>
        /// <param name="subtrahend">
        /// The subtrahend is the "right hand side" of the subtraction expression.
        /// </param>
        /// <returns></returns>
        public static BalancedTernary operator -(BalancedTernary minuend, BalancedTernary subtrahend)
        {
            return minuend + (-subtrahend);
        }

        /// <summary>
        /// Represents the balanced ternary number as a string, where T=-1
        /// </summary>
        private System.Collections.Generic.List<System.SByte> m_value = new List<sbyte>();

        //private int Resolution{
        //    get { return m_stringEncoding.Length;}
        //    set 
        //    {
        //        System.Diagnostics.Trace.Assert(value > 0);

        //        // Terribly inefficient, yes;)
        //        while (value > m_stringEncoding.Length)
        //        {
        //            m_stringEncoding = "0" + m_stringEncoding;
        //        }
        //        if (value < m_stringEncoding.Length)
        //        {
        //            m_stringEncoding = m_stringEncoding.Remove(value);
        //        }
        //    }
        //}

        private SByte CalculateCarry(int index)
        {
            SByte carry = 0;
            while (m_value[index] < -1)
            {
                m_value[index] += 3;
                --carry;
            }
            while (m_value[index] > 1)
            {
                m_value[index] -= 3;
                ++carry;
            }
            return carry;
        }

        public BalancedTernary Add(BalancedTernary addend)
        {
            // Make sure this is at least as big as the addend.
            while (m_value.Count < addend.m_value.Count)
            {
                m_value.Add(0);
            }

            SByte index = 0;
            SByte carry = 0;

            foreach (SByte currentValue in addend.m_value)
            {
                m_value[index] += (SByte)(currentValue + carry);
                carry = CalculateCarry( index);
                ++index;
            }
            while (carry != 0)
            {
                while (m_value.Count < (index + 1))
                {
                    m_value.Add(0);
                }
                m_value[index] += carry;
                carry = CalculateCarry(index);
                ++index;
            }
            return this;
        }

        private static BalancedTernary one = new BalancedTernary("1");
        public static BalancedTernary operator++( BalancedTernary val)
        {

            return val.Add(one);
        }

        public BalancedTernary(String btValue)
        {
            foreach (char c in btValue)
            {
                SByte currentVal = 0;
                switch (c)
                {
                    case '1':
                        currentVal = 1;
                        break;
                    case 'T':
                        currentVal = -1;
                        break;
                }
                m_value.Insert(0, currentVal);
            }
        }

        public BalancedTernary()
        {
            m_value.Clear();
            m_value.Add(0);
        }

        public BalancedTernary(int value)
        {
            m_value.Clear();
            if (value == 0)
            {
                m_value.Add(0);
            }
            while (value != 0)
            {
                int nextValue = value / 3;
                int remainder = value - (nextValue * 3);
                if (remainder > 1)
                {
                    remainder -= 3;
                    ++nextValue;
                }
                if (remainder < -1)
                {
                    remainder += 3;
                    --nextValue;
                }
                m_value.Add((sbyte) remainder);
                value = nextValue;
            }
        }

        public BalancedTernary(BalancedTernary copy)
        {
            //m_value = new List<sbyte>();
            m_value.AddRange( copy.m_value);
            //foreach (SByte b in copy.m_value)
            //{
            //    m_value.Add(b);
            //}
        }

        public static BalancedTernary operator +(BalancedTernary lhs, BalancedTernary rhs)
        {
            return new BalancedTernary(lhs).Add(rhs);
        }

        public static BalancedTernary operator +(BalancedTernary lhs, int rhs)
        {
            return new BalancedTernary(lhs).Add(new BalancedTernary(rhs));
        }

        public int ToInt()
        {
            int result = 0;
            int factor = 1;
            foreach (SByte b in m_value)
            {
                result += b * factor;
                factor *= 3;
            }
            return result;
        }

        public static implicit operator int( BalancedTernary val)
        {
            return val.ToInt();
        }

        public override string ToString()
        {
            StringBuilder result = new StringBuilder();
            foreach (SByte b in m_value)
            {
                switch (b)
                {
                    case -1:                    
                        result.Insert( 0, "T");
                        break;
                    case 0:
                        result.Insert( 0, "0");
                        break;
                    case 1:
                        result.Insert( 0, "1");
                        break;
                    default:
                        result.Insert( 0, "?");
                        break;
                }
            }
            return result.ToString();
        }
        public SByte this[ int index]
        {
            get 
            { 
                if (index >= m_value.Count)
                    return 0;
                else
                    return m_value[index];
            }
            set { m_value[index] = value; }
        }
        public int Resolution
        {
            get { return m_value.Count; }
            //set // TODO!
        }
    }

    namespace Test
    {
        using NUnit.Framework;
        [TestFixture]
        public class BalancedTernaryTest
        {
            [Test]
            public void Initialization()
            {
                BalancedTernary val = new BalancedTernary();
                System.Diagnostics.Trace.WriteLine(val);
            }

            private static string[] s_wellKnownValues = {
                "0", "1", "1T", "10", "11", "1TT", "1T0", "1T1", "10T", "100"};

            private void AssertEquality(BalancedTernary val, string expectedString, int expectedValue)
            {
                if ((expectedString != val.ToString()) || (expectedValue != val.ToInt()))
                {
                    System.Diagnostics.Trace.WriteLine(
                        String.Format("{0}:{1}:{2}", val.ToString(), expectedString, expectedValue));
                }
                Assert.AreEqual(expectedString, val.ToString());
                Assert.AreEqual(expectedValue, val);
            }

            [Test]
            public void Increment()
            {
                BalancedTernary val = new BalancedTernary();
                for (int index = 0; index < s_wellKnownValues.Length; ++index)
                {
                    AssertEquality(val, s_wellKnownValues[index], index);
                    ++val;
                }

                System.Diagnostics.Trace.WriteLine(val);
            }

            /// <summary>
            /// Simple test that string construction creates the correct values.
            /// </summary>
            [Test]         
            public void StringConstruction()
            {
                BalancedTernary three = new BalancedTernary("10");
                Assert.AreEqual(three, 3);
                BalancedTernary two = new BalancedTernary("1T");
                Assert.AreEqual(two, 2);
            }

            /// <summary>
            /// Tests the integer constructor.
            /// </summary>
            [Test]
            public void IntConstruction()
            {
                for (int index = 0; index < s_wellKnownValues.Length; ++index)
                {
                    BalancedTernary value = new BalancedTernary(index);
                    AssertEquality(value, s_wellKnownValues[index], index);
                }

                AssertEquality(new BalancedTernary(-1), "T", -1);
                AssertEquality(new BalancedTernary(-4), "TT", -4);
                AssertEquality(new BalancedTernary(-5), "T11", -5);
                AssertEquality(new BalancedTernary(-6), "T10", -6);
            }

            /// <summary>
            /// Tests simple addition.
            /// </summary>
            [Test]
            public void Addition()
            {
                BalancedTernary a = new BalancedTernary("10");
                BalancedTernary b = new BalancedTernary("10");
                BalancedTernary c = a + b;
                Assert.AreEqual( c, 6);
            }

            [Test]
            public void Negation()
            {
                BalancedTernary a = new BalancedTernary("1T1");
                AssertEquality( -a, "T1T", -7);
                AssertEquality( -(-a), "1T1", 7);
            }

            [Test]
            public void Subtraction()
            {
                BalancedTernary a = new BalancedTernary("11T01T0");
                BalancedTernary b = new BalancedTernary("1T1");
                Assert.AreEqual( (a - b).ToInt(), a.ToInt() - b.ToInt());
                Assert.AreEqual((b - a).ToInt(), b.ToInt() - a.ToInt());
            }

            /// <summary>
            /// A profiling diagnostic, to compare the performance of int's 
            /// with BT's.  The current test results:
            ///   Integer = 31.25
            ///   BalancedTernary = 8421.875
            /// Which is approximately 300X slower.
            /// </summary>
            [System.Diagnostics.Conditional( "PROFILE")]
            private void DoProfileTiming()
            {
                DateTime start = DateTime.Now;
                int x = 100;
                int y = 1;
                for (int i = 0; i < 10000000; ++i)
                {
                    x += y;
                }
                TimeSpan integerOperation = DateTime.Now - start;
                start = DateTime.Now;
                BalancedTernary a = new BalancedTernary( 100);
                BalancedTernary b = new BalancedTernary(1);
                for (int i = 0; i < 10000000; ++i)
                {
                    a += b;
                }
                TimeSpan btOperation = DateTime.Now - start;
                System.Diagnostics.Trace.WriteLine(String.Format("Integer = {0}", integerOperation.TotalMilliseconds));
                System.Diagnostics.Trace.WriteLine(String.Format("BalancedTernary = {0}", btOperation.TotalMilliseconds));
            }

            [Test]
            public void ProfileTiming()
            {
                DoProfileTiming();
            }
        }
    }
}
