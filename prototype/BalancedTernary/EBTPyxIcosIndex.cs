using System;
using System.Collections.Generic;
using System.Text;

namespace PYXIS.Mathematics
{

    class EBTPyxIcosIndex
    {
        public IcosFace m_face;
        public EBTPyxIndex m_subindex;
        private static System.Text.RegularExpressions.Regex s_indexExpression =
            new System.Text.RegularExpressions.Regex("-");
        public EBTPyxIcosIndex( string icosIndex)
        {
            string[] substrings = s_indexExpression.Split( icosIndex);
            if (substrings.Length != 2)
            {
                throw new ArgumentException( 
                    String.Format("Badly formed icos index {0}.", icosIndex),
                    icosIndex);
            }
            m_face = new IcosFace(substrings[0]);
            m_subindex = new EBTPyxIndex(substrings[1]);
        }
        public EBTPyxIcosIndex(IcosFace face, EBTPyxIndex subindex)
        {
            m_face = face;
            m_subindex = subindex;
        }

        public override String ToString()
        {
            return String.Format("{0}-{1}", m_face, m_subindex.ToString());
        }

        private static EBTPyxIndex GetDirectionVector(eHexDirection d)
        {
            EBTPyxIndex result = new EBTPyxIndex();
            result.Resolution = 2;
            switch (d)
            {
                case eHexDirection.knDirectionOne:
                    result += "1";
                    break;
                case eHexDirection.knDirectionTwo:
                    result += "2";
                    break;
                case eHexDirection.knDirectionThree:
                    result += "3";
                    break;
                case eHexDirection.knDirectionFour:
                    result += "4";
                    break;
                case eHexDirection.knDirectionFive:
                    result += "5";
                    break;
                case eHexDirection.knDirectionSix:
                    result += "6";
                    break;
            }
            result *= 3;
            result.Negate();
            return result;
        }
        public EBTPyxIcosIndex(EBTPyxIcosIndex copy)
        {
            m_face = copy.m_face;
            m_subindex = copy.m_subindex;
        }

        public static EBTPyxIcosIndex operator+( EBTPyxIcosIndex lhs, EBTPyxIndex rhs)
        {
            System.Collections.Generic.List<IcosFace> facesTried = new List<IcosFace>();
            IcosFace face = lhs.m_face;
            EBTPyxIndex result = lhs.m_subindex + rhs;
            while (result.OverflowFlag)
            {
                int bestDistance = -1;
                IcosFace bestFace = face;
                EBTPyxIndex bestResult = null;
                int bestRotation = 0;

                foreach (eHexDirection d in Enum.GetValues( typeof( eHexDirection)))
                {
                    if (d != eHexDirection.knDirectionZero)
                    {
                        PyxMathConstants.ConnectivityInformation currentConnectivity = 
                            PyxMathConstants.GetConnectivityRes1(face, d);
                        if (currentConnectivity.Destination != null)
                        {

                            EBTPyxIndex currentResult = result + GetDirectionVector( d);
                            if ((bestDistance == -1) || (currentResult.Magnitude() < bestDistance))
                            {
                                bestDistance = currentResult.Magnitude();
                                bestFace = new IcosFace( currentConnectivity.Destination);
                                bestResult = currentResult;
                                bestRotation = currentConnectivity.Rotation;
                            }
                        }
                    }
                }

                System.Diagnostics.Trace.Assert( bestResult != null);
                face = bestFace;
                result = bestResult;
                result.Rotate(bestRotation);
            }
            return new EBTPyxIcosIndex( face, result);
        }
    }

    namespace Test
    {
        using NUnit.Core;
        using NUnit.Framework;

        [TestFixture]
        public class EBTPyxIcosIndexTest
        {
            [Test]
            public void Construction()
            {
                EBTPyxIcosIndex index = new EBTPyxIcosIndex("A-0101");
            }

            [Test]
            public void Addition()
            {
                EBTPyxIcosIndex index = new EBTPyxIcosIndex("A-0");
                EBTPyxIndex vector = new EBTPyxIndex("01");
                while (index.ToString()[0] == 'A')
                {
                    index += vector;
                    System.Diagnostics.Trace.WriteLine(String.Format("Now at {0}", index));
                }
                System.Diagnostics.Trace.WriteLine(index);
            }

            private void TestGenerateContainedCells(string index, int resolutionDepth)
            {
                System.Diagnostics.Trace.WriteLine(String.Format("{0} has {1} cells at resolution {2}.", index,
                    CountContainedCells(index, resolutionDepth), resolutionDepth));
            }

            private int CountContainedCells(string index, int resolutionDepth)
            {
                return CountContainedCells(new EBTPyxIcosIndex(index), resolutionDepth);
            }

            private System.Collections.Generic.List<EBTPyxIcosIndex> 
                GenerateContainedCells(EBTPyxIcosIndex cell, int resolutionDepth)
            {
                System.Collections.Generic.List<EBTPyxIcosIndex> result = 
                    new List<EBTPyxIcosIndex>();

                EBTPyxIcosIndex centre = new EBTPyxIcosIndex(cell);
                centre.m_subindex.ExtendResolution(resolutionDepth);
                int extent = resolutionDepth + 1;
                for (int u = -extent; u <= extent; ++u)
                {
                    for (int v = -extent; v <= extent; ++v)
                    {
                        EBTPyxIndex currentVector = new EBTPyxIndex(u, v, resolutionDepth);
                        if (currentVector.Magnitude() <= extent)
                        {
                            EBTPyxIcosIndex currentCell = cell + new EBTPyxIndex(u, v, resolutionDepth);
                            result.Add(currentCell);
                        }
                    }
                }
                return result;
            }

            private int CountContainedCells(EBTPyxIcosIndex cell, int resolutionDepth)
            {
                return GenerateContainedCells(cell, resolutionDepth).Count;
            }

            [Test]
            public void TestContainment()
            {
                TestGenerateContainedCells("1-3", 2);
                TestGenerateContainedCells("1-10050010", 2);
                TestGenerateContainedCells("A-001", 2);
                TestGenerateContainedCells("A-010", 2);
                TestGenerateContainedCells("A-020010404", 6);
                TestGenerateContainedCells("3-0200104040100100020300006020", 22);
            }
        }
    }
}
