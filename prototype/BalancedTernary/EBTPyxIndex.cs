using System;
using System.Collections.Generic;
using System.Text;

namespace PYXIS.Mathematics
{
    class EBTPyxIndex
    {
        /// <summary>
        /// Distance in "6" direction.
        /// </summary>
        private BalancedTernary m_u;

        /// <summary>
        /// Distance in "1" direction.
        /// </summary>
        private BalancedTernary m_v;

        public EBTPyxIndex()
        {
            m_u = new BalancedTernary();
            m_v = new BalancedTernary();
        }

        static private System.Text.RegularExpressions.Regex chunker = new System.Text.RegularExpressions.Regex("..");
        static private System.Text.RegularExpressions.MatchCollection GetDigitPairs(string text)
        {
            if ((text.Length % 2) != 0)
            {
                text = "0" + text;
            }

            return chunker.Matches(text);
        }

        private bool m_zeroPrepended = false;

        public static EBTPyxIndex operator *(EBTPyxIndex val, int scalar)
        {
            EBTPyxIndex result = new EBTPyxIndex(val);
            result.m_u *= scalar;
            result.m_v *= scalar;
            return result;
        }

        public EBTPyxIndex(string pyxIndex)
        {
            m_zeroPrepended = (pyxIndex.Length % 2) != 0;
            Resolution = pyxIndex.Length + (m_zeroPrepended ? 1 : 0);

            m_u = new BalancedTernary();
            m_v = new BalancedTernary();

            // TODO: Consider pre-building some small bt constants (-2..2)
            foreach (System.Text.RegularExpressions.Match pair in GetDigitPairs(pyxIndex))
            {
                if (m_u.Resolution > 0)
                {
                    m_u *= 3;
                    m_v *= 3;
                }

                switch (pair.Value)
                {
                    case "00":
                        //m_u += 0;
                        //m_v += 0;
                        break;
                    case "01":
                        //m_u += 0;
                        m_v += 1;
                        break;
                    case "02":
                        m_u += -1;
                        m_v += 1;
                        break;
                    case "03":
                        m_u += -1;
                        //m_v += 0;
                        break;
                    case "04":
                        //m_u += 0;
                        m_v += -1;
                        break;
                    case "05":
                        m_u += 1;
                        m_v += -1;
                        break;
                    case "06":
                        m_u += 1;
                        //m_v += 0;
                        break;

                    case "10":
                        m_u += 1;
                        m_v += 1;
                        break;
                    case "20":
                        m_u += -1;
                        m_v += 2;
                        break;
                    case "30":
                        m_u += -2;
                        m_v += 1;
                        break;
                    case "40":
                        m_u += -1;
                        m_v += -1;
                        break;
                    case "50":
                        m_u += 1;
                        m_v += -2;
                        break;
                    case "60":
                        m_u += 2;
                        m_v += -1;
                        break;
                }
            }
        }

        /// <summary>
        /// Vector magnitude, in steps from origin.
        /// </summary>
        /// <returns></returns>
        public int Magnitude()
        {
            int u = m_u.ToInt();
            int v = m_v.ToInt();
            if (u * v < 0)
            {
                return Math.Max(Math.Abs(u), Math.Abs(v));
            }
            else
            {
                return Math.Abs(u + v);
            }
        }

        public double MagnitudeTwo()
        {
            int u = m_u.ToInt();
            int v = m_v.ToInt();
            bool uDominates = Math.Abs(u) >= 2 * Math.Abs(v);
            bool vDominates = Math.Abs(v) >= 2 * Math.Abs(u);
            if ((uDominates || vDominates)  || ((u * v) > 0))
            {
                //int sign = ((u * v) >= 0) ? 1 : -1;

                if (Math.Abs(u) <= Math.Abs(v))
                {
                    // v is larger than u, we are in (abcd)
                    return Math.Abs(v + 0.5 * (u));
                }
                else
                {
                    // u is larger than v, we are in (efgh)
                    return Math.Abs(u + 0.5 * (v));
                }
            }
            else
            {
                if ((u * v) <= 0)
                {
                    double delta = (u + v) / 2.0;
                    if (Math.Abs( u) > Math.Abs(v))
                    {
                        return Math.Abs(u - delta);
                    }
                    else
                    {
                        return Math.Abs(v - delta);
                    }

                    if (u > v)
                        return Math.Abs(u + Math.Abs(Math.Abs(u) - Math.Abs(v)) / 2);
                    else
                        return Math.Abs(v + Math.Abs(Math.Abs(u) - Math.Abs(v)) / 2);
                    // Case 1 works for most.
////                    return Math.Max( Math.Abs(u), Math.Abs(v));
                    //if (u > v)
                    //    return Math.Abs(u) - Math.Abs(v) / 2;
                    //else
                    //    return Math.Abs(v) - Math.Abs(u) / 2;
                    //return Math.Max(Math.Abs(u), Math.Abs(v));
                }
                else
                {
                    return Math.Abs(u + v);
                }
            }
        }

        private int m_resolution;
        public int Resolution
        {
            get { return m_resolution; }
            set { m_resolution = value; }
        }

        ///// <summary>
        ///// Determines which direction (1-6) the vector lies in.  0 for origin!
        ///// </summary>
        ///// <returns></returns>
        //public int Hextent()
        //{
        //    // TODO: Finish this.
        //    int u = m_u.ToInt();
        //    int v = m_v.ToInt();
        //    if (Math.Abs(u) > Math.Abs(v))
        //    {
        //        if (u > 0)
        //        {
        //            return 6;
        //        }
        //        else
        //        {
        //            return 3;
        //        }
        //    }
        //    else if (Math.Abs(u) < Math.Abs(v))
        //    {
        //    }
        //    else
        //    {
        //    }
        //}

        public class OverflowException: Exception
        {
            public OverflowException(string index)
                : base(String.Format("Overflow at {0}", index))
            {
            }
        }

        static bool s_verboseDebugging = false;

        private bool m_overflow = false;

        public override String ToString()
        {
            String result = GetString();
            if (m_overflow)
            {
                throw new OverflowException(result);
            }
            return result;
        }

        public String GetString()
        {
            m_overflow = false;
            StringBuilder result = new StringBuilder(m_u.Resolution * 2);
            SByte uCarry = 0;
            SByte vCarry = 0;
            for (int index = 0; index < m_u.Resolution; ++index)
            {
                SByte u = (SByte) (m_u[index] + uCarry); 
                SByte v = (SByte) (m_v[index] + vCarry);
                uCarry = 0;
                vCarry = 0;
                switch (u)
                {
                    case -1:
                        switch (v)
                        {
                            case -1:
                                // 040 (0T,0T)
                                // 060 (1T, T)
                                // 020 (T, 1T)
                                // An odd resolution!
                                {
                                    SByte uPrev = m_u[index + 1];
                                    SByte vPrev = m_v[index + 1];
                                    if (uPrev == vPrev)
                                    {
                                        result.Insert(0, "40");
                                    }
                                    else if (uPrev > vPrev)
                                    {
                                        result.Insert(0, "60");
                                        uCarry = -1;
                                    }
                                    else
                                    {
                                        result.Insert(0, "20");
                                        vCarry = -1;
                                    }
                                }
                                break;
                            case 0:
                                result.Insert( 0, "03");
                                break;
                            case 1:
                                result.Insert( 0, "02");
                                break;
                        }
                        break;
                    case 0:
                        switch (v)
                        {
                            case -1:
                                result.Insert( 0, "04");
                                break;
                            case 0:
                                result.Insert( 0, "00");
                                break;
                            case 1:
                                result.Insert( 0, "01");
                                break;
                        }
                        break;
                    case 1:
                        switch (v)
                        {
                            case -1:
                                result.Insert( 0, "05");
                                break;
                            case 0:
                                result.Insert( 0, "06");
                                break;
                            case 1:
                                // 010 (01,01)
                                // 030 (T1, 1)
                                // 050 (1, T1)
                                // An odd resolution!
                                {
                                    SByte uPrev = m_u[index + 1];
                                    SByte vPrev = m_v[index + 1];
                                    if (uPrev == vPrev)
                                    {
                                        result.Insert(0, "10");
                                    }
                                    else if (uPrev > vPrev)
                                    {
                                        result.Insert(0, "50");
                                        vCarry = 1;
                                    }
                                    else
                                    {
                                        result.Insert(0, "30");
                                        uCarry = 1;
                                    }
                                }
                                break;
                        }
                        break;
                }
            }
            if (result.Length > Resolution)
            {
                // Throw away all leading "0"'s.
                string shortResult = result.ToString();
                while ((shortResult.Length > Resolution) && (shortResult.Substring(0, 2) == "00"))
                {
                    shortResult = shortResult.Substring(2);
                }

                int originalResolution = Resolution - (m_zeroPrepended ? 1 : 0);
                if (m_zeroPrepended && (shortResult.Length > 0) && (shortResult[0] == '0'))
                {
                    shortResult = shortResult.Substring(1);
                }
                while (shortResult.Length < originalResolution)
                {
                    shortResult = "0" + shortResult;
                }

                if (shortResult.Length > originalResolution)
                {
                    if (s_verboseDebugging)
                    {
                        System.Diagnostics.Trace.WriteLine("Overflow!");
                    }
                    m_overflow = true;
                    //throw new OverflowException(shortResult);
                }

                if (s_verboseDebugging)
                {
                    System.Diagnostics.Trace.WriteLine(String.Format(
                        "{0} -> {1}", result, shortResult.Substring(0, originalResolution)));
                }
                return shortResult.Substring(0, originalResolution);
            }
            if (s_verboseDebugging)
            {
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "{0} -> unchanged", result));
            }

            return result.ToString();
        }

        public void Rotate(int steps)
        {
            int u = m_u.ToInt();
            int v = m_v.ToInt();
            switch (steps % 6)
            {
                case 1:
                    m_u = new BalancedTernary( 0 * u + 1 * v);
                    m_v = new BalancedTernary(-1 * u + 1 * v);
                    break;
                case 2:
                    m_u = new BalancedTernary(-1 * u - 1 * v);
                    m_v = new BalancedTernary( 1 * u + 0 * v);
                    break;
                case 3:
                    m_u = new BalancedTernary(-1 * u + 0 * v);
                    m_v = new BalancedTernary( 0 * u - 1 * v);
                    break;
                case 4:
                    m_u = new BalancedTernary( 0 * u + 1 * v);
                    m_v = new BalancedTernary(-1 * u - 1 * v);
                    break;
                case 5:
                    m_u = new BalancedTernary( 1 * u + 1 * v);
                    m_v = new BalancedTernary( -1 * u + 0 * v);
                    break;
            }
        }

        public void Negate()
        {
            m_v.Negate();
            m_u.Negate();
        }

        public EBTPyxIndex(EBTPyxIndex copy)
        {
            m_zeroPrepended = copy.m_zeroPrepended;
            m_resolution = copy.m_resolution;
            m_u = copy.m_u;
            m_v = copy.m_v;
        }

        public EBTPyxIndex( int u, int v, int resolution)
        {
            m_resolution = resolution;
            m_u = new BalancedTernary(u);
            m_v = new BalancedTernary(v);
            m_zeroPrepended = false;
        }
        public static EBTPyxIndex operator +(EBTPyxIndex index, EBTPyxIndex addend)
        {
            EBTPyxIndex result = new EBTPyxIndex(addend);
            result.Resolution = index.Resolution;
            result.m_u += index.m_u;
            result.m_v += index.m_v;
            if (s_verboseDebugging)
            {
                System.Diagnostics.Trace.WriteLine(String.Format("{0}+{1}={2}", index, addend, result));
            }
            return result;
        }
        public static EBTPyxIndex operator +(EBTPyxIndex index, string addend)
        {           
            // TODO: Consider factoring in a call to the previous function
            EBTPyxIndex result = new EBTPyxIndex(addend);
            result.Resolution = index.Resolution;
            result.m_u += index.m_u;
            result.m_v += index.m_v;
            if (s_verboseDebugging)
            {
                System.Diagnostics.Trace.WriteLine(String.Format("{0}+{1}={2}", index, addend, result));
            }

            result.TestOverflow();

            return result;
        }
        public static EBTPyxIndex SafeAdd(EBTPyxIndex index, EBTPyxIndex addend)
        {
            EBTPyxIndex result = new EBTPyxIndex(addend);
            result.Resolution = index.Resolution;
            result.m_u += index.m_u;
            result.m_v += index.m_v;
            return result;
        }

        public void TestOverflow()
        {
            // TODO: We might want to do something more clever than this!
            ToString();
        }

        public bool OverflowFlag
        {
            get
            {
                GetString();
                return m_overflow;
            }
        }

        public void ExtendResolution(int additionalResolution)
        {
            // TODO: Make ExtendResolution handle odd resolutions.
            System.Diagnostics.Trace.Assert( (additionalResolution % 2) == 0, 
                "ExtendResolution only supports steps of 2");
            for (int steps = 0; steps < additionalResolution; steps += 2)
            {
                m_resolution += 2;
                m_u *= 3;
                m_v *= 3;
            }
        }
    }

    namespace Test
    {
        using NUnit.Core;
        using NUnit.Framework;
        [TestFixture]
        public class EBTPxIndexTest
        {
            private void TestConstruction(string indexText)
            {
                EBTPyxIndex index = new EBTPyxIndex(indexText);
                if (index.ToString() != indexText)
                {
                    string test = index.ToString();
                    System.Diagnostics.Trace.WriteLine( string.Format( "Expecting {0}, saw {1}",
                        indexText, test));
                }
                Assert.AreEqual(index.ToString(), indexText);
            }

            [Test]
            public void Construction()
            {
                TestConstruction("10101");
                TestConstruction("0505");
                TestConstruction("1005");
                TestConstruction("10050010");

                TestConstruction("001");
                TestConstruction("002");
                TestConstruction("003");
                TestConstruction("004");
                TestConstruction("005");
                TestConstruction("006");
                TestConstruction("010");
                TestConstruction("020");
                TestConstruction("030");
                TestConstruction("040");
                TestConstruction("050");
                TestConstruction("060");
                TestConstruction("020010404");
                TestConstruction("0200104040100100020300006020");
            }

            public void TestAddition( string direction)
            {
                try
                {
                    EBTPyxIndex index = new EBTPyxIndex("10050010");
                    for (int count = 0; count < 1000000; ++count)
                    {
                        index = index + direction;
                        System.Diagnostics.Trace.WriteLine(index.ToString());
                    }
                    Assert.Fail("Didn't overflow in direction {0}", direction);
                }
                catch (EBTPyxIndex.OverflowException ex)
                {
                    System.Diagnostics.Trace.WriteLine(String.Format("While adding {0}, {1}", direction, ex.Message));
                    return;
                }
            }

//            [Test]
            public void Addition()
            {
                TestAddition("01");
                TestAddition("02");
                TestAddition("03");
                TestAddition("04");
                TestAddition("05");
                TestAddition("06");
            }


            [Test]
            public void TestExtendResolution()
            {
                EBTPyxIndex cell = new EBTPyxIndex( "0101");
                Assert.AreEqual(cell.ToString(), "0101");
                cell.ExtendResolution(2);
                Assert.AreEqual(cell.ToString(), "010100");
            }

            private void TestDistance(EBTPyxIndex index, double distance)
            {
                double d = index.MagnitudeTwo();
                if (Math.Abs(d - distance) > 0.01)
                {
                    System.Diagnostics.Trace.WriteLine(index.MagnitudeTwo());
                 //   Assert.Fail("Out of bounds");
                }
            }

            [Test]
            public void Magnitude2()
            {
                TestDistance(new EBTPyxIndex(1, -3, 4), 2.5);
                TestDistance(new EBTPyxIndex(2, 1, 4), 2.5);
                TestDistance(new EBTPyxIndex(2, -1, 4), 1.5);
                TestDistance(new EBTPyxIndex(-2, 1, 4), 1.5);
                TestDistance(new EBTPyxIndex(-2, -1, 4), 2.5);
                TestDistance(new EBTPyxIndex(-3, -3, 4), 4.5);
                TestDistance(new EBTPyxIndex(3, 3, 4), 4.5);
                TestDistance(new EBTPyxIndex(-3, 3, 4), 3);
                TestDistance(new EBTPyxIndex(-3, 2, 4), 2.5);
                int magnitude = 3;

                int inCount = 0;
                int outCount = 0;
                int boundaryCount = 0;

                for (int u = -magnitude * 2; u <= magnitude * 2; ++u)
                {
                    for (int v = -magnitude * 2; v <= magnitude * 2; ++v)
                    {
                        EBTPyxIndex index = new EBTPyxIndex( u, v, magnitude * 2);
                        double realMagnitude = index.MagnitudeTwo();
                        if (realMagnitude == magnitude)
                        {
                            System.Diagnostics.Trace.Write('X');
                            boundaryCount++;
                        }
                        else if (realMagnitude < magnitude)
                        {
                            if ((u == 0) && (v == 0))
                                System.Diagnostics.Trace.Write('*');
                            else
                            System.Diagnostics.Trace.Write('+');
                            inCount++;
                        }
                        else
                        {
                            System.Diagnostics.Trace.Write('-');
                            outCount++;
                        }
                        //System.Diagnostics.Trace.WriteLine(String.Format("Point {2} ({0}, {1}) is {3}",
                        //    u, v, index, (realMagnitude < magnitude) ? "in" :
                        //    (realMagnitude == magnitude) ? "on the edge" : "out"));
                    }
                    System.Diagnostics.Trace.WriteLine("");
                }
                System.Diagnostics.Trace.WriteLine( String.Format( "In = {0}, Out = {1}, B = {2}", inCount, outCount, boundaryCount));
            }
        }
    }
}
