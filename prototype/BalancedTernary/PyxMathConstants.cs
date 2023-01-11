using System;
using System.Collections.Generic;
using System.Text;

namespace PYXIS.Mathematics
{
    public 	enum eHexDirection
	{
		knDirectionZero = 0,
		knDirectionOne,
		knDirectionTwo,
		knDirectionThree,
		knDirectionFour,
		knDirectionFive,
		knDirectionSix,
	}
    static class PyxMathConstants
    {
        private static eHexDirection[] m_GapDirection = 
            {
                	eHexDirection.knDirectionZero,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionOne,
	                eHexDirection.knDirectionFour,
	                eHexDirection.knDirectionFour,
	                eHexDirection.knDirectionFour,
	                eHexDirection.knDirectionFour,
	                eHexDirection.knDirectionFour,
	                eHexDirection.knDirectionFour
            };
        public static eHexDirection[] GapDirection
        {
            get { return m_GapDirection;}
        }

        public struct ConnectivityInformation
        {
            int m_destination;
            int m_rotation;
            public IcosFace Destination 
            { 
                get 
                { 
                    if (m_destination == -1)
                        return null;
                    else
                        return new IcosFace(m_destination);
                }
            }
            public int Rotation { get { return m_rotation; } }
            public ConnectivityInformation( int dest, int rot)
            {
                m_destination = dest;
                m_rotation = rot;
            }
            public ConnectivityInformation(char dest, int rot)
            {
                m_destination = (int) dest;
                m_rotation = rot;
            }
        }
        private static int[, ,] m_Res0Connect =
            {
                // Vertex 1
                {{-1,-1},{2,3},{3,2},{4,1},{5,0},{6,5}},
                // Vertex 2
                {{-1,-1},{1,3},{6,0},{11,0},{7,0},{3,0}},
                // Vertex 3
                {{-1,-1},{1,4},{2,0},{7,0},{8,0},{4,0}},
                // Vertex 4
                {{-1,-1},{1,5},{3,0},{8,0},{9,0},{5,0}},
                // Vertex 5
                {{-1,-1},{1,0},{4,0},{9,0},{10,0},{6,0}},
                // Vertex 6
                {{-1,-1},{1,1},{5,0},{10,0},{11,0},{2,0}},
                // Vertex 7
                {{3,0},{2,0},{11,0},{-1,-1},{12,1},{8,0}},
                // Vertex 8
                {{4,0},{3,0},{7,0},{-1,-1},{12,0},{9,0}},
                // Vertex 9
                {{5,0},{4,0},{8,0},{-1,-1},{12,5},{10,0}},
                // Vertex 10
                {{6,0},{5,0},{9,0},{-1,-1},{12,4},{11,0}},
                // Vertex 11
                {{2,0},{6,0},{10,0},{-1,-1},{12,3},{7,0}},
                // Vertex 12
                {{9,1},{8,0},{7,5},{-1,-1},{11,3},{10,2}}
            };
        //public static ConnectivityInformation GetConnectivityRes0(int from, eHexDirection dir)
        //{
        //    return new ConnectivityInformation( m_Res0Connect[from, (int)dir, 0],
        //        m_Res0Connect[from, (int)dir, 1]);
        //}

        private static int[, ,] m_Res1VertexConnectivity = 
            {
	            // Vertex 1
	            {{-1,-1},{'A',2},{'B',1},{'C',0},{'D',5},{'E',4}},
	            // Vertex 2
	            {{-1,-1},{'E',0},{'J',0},{'O',0},{'F',0},{'A',0}},
	            // Vertex 3
	            {{-1,-1},{'A',0},{'F',0},{'K',0},{'G',0},{'B',0}},
	            // Vertex 4
	            {{-1,-1},{'B',0},{'G',0},{'L',0},{'H',0},{'C',0}},
	            // Vertex 5
	            {{-1,-1},{'C',0},{'H',0},{'M',0},{'I',0},{'D',0}},
	            // Vertex 6
	            {{-1,-1},{'D',0},{'I',0},{'N',0},{'J',0},{'E',0}},
	            // Vertex 7
	            {{'F',0},{'O',0},{'T',0},{-1,-1},{'P',0},{'K',0}},
	            // Vertex 8
	            {{'G',0},{'K',0},{'P',0},{-1,-1},{'Q',0},{'L',0}},
	            // Vertex 9
	            {{'H',0},{'L',0},{'Q',0},{-1,-1},{'R',0},{'M',0}},
	            // Vertex 10
	            {{'I',0},{'M',0},{'R',0},{-1,-1},{'S',0},{'N',0}},
	            // Vertex 11
	            {{'J',0},{'N',0},{'S',0},{-1,-1},{'T',0},{'O',0}},
	            // Vertex 12
	            {{'Q',0},{'P',5},{'T',4},{-1,-1},{'S',2},{'R',1}}
            };
        public static ConnectivityInformation GetConnectivityRes1(IcosFace from, eHexDirection dir)
        {
            System.Diagnostics.Trace.Assert(Enum.IsDefined(typeof(eHexDirection), dir));
            System.Diagnostics.Trace.Assert(dir != eHexDirection.knDirectionZero);
            int rawDir = ((int)dir) - 1;

            if (from.IsVertex)
            {
                //System.Diagnostics.Trace.Assert( from > 0)
                return new ConnectivityInformation(m_Res1VertexConnectivity[from.VertexIndex, rawDir, 0],
                    m_Res1VertexConnectivity[from.VertexIndex, rawDir, 1]);
            }
            else
            {
                return new ConnectivityInformation(m_Res1FaceConnect[from.FaceIndex, rawDir, 0],
                    m_Res1FaceConnect[from.FaceIndex, rawDir, 1]);
            }
        }
        private static int[, ,] m_Res1FaceConnect = 
        {
	        // Face A
	        {{1,4},{'E',1},{2,0},{'F',0},{3,0},{'B',5}},
	        // Face B
	        {{1,5},{'A',1},{3,0},{'G',0},{4,0},{'C',5}},
	        // Face C
	        {{1,0},{'B',1},{4,0},{'H',0},{5,0},{'D',5}},
	        // Face D
	        {{1,1},{'C',1},{5,0},{'I',0},{6,0},{'E',5}},
	        // Face E
	        {{1,2},{'D',1},{6,0},{'J',0},{2,0},{'A',5}},
	        // Face F
	        {{'A',0},{2,0},{'O',0},{7,0},{'K',0},{3,0}},
	        // Face G
	        {{'B',0},{3,0},{'K',0},{8,0},{'L',0},{4,0}},
	        // Face H
	        {{'C',0},{4,0},{'L',0},{9,0},{'M',0},{5,0}},
	        // Face I
	        {{'D',0},{5,0},{'M',0},{10,0},{'N',0},{6,0}},
	        // Face J
	        {{'E',0},{6,0},{'N',0},{11,0},{'O',0},{2,0}},
	        // Face K
	        {{3,0},{'F',0},{7,0},{'P',0},{8,0},{'G',0}},
	        // Face L
	        {{4,0},{'G',0},{8,0},{'Q',0},{9,0},{'H',0}},
	        // Face M
	        {{5,0},{'H',0},{9,0},{'R',0},{10,0},{'I',0}},
	        // Face N
	        {{6,0},{'I',0},{10,0},{'S',0},{11,0},{'J',0}},
	        // Face O
	        {{2,0},{'J',0},{11,0},{'T',0},{7,0},{'F',0}},
	        // Face P
	        {{'K',0},{7,0},{'T',5},{12,1},{'Q',1},{8,0}},
	        // Face Q
	        {{'L',0},{8,0},{'P',5},{12,0},{'R',1},{9,0}},
	        // Face R
	        {{'M',0},{9,0},{'Q',5},{12,5},{'S',1},{10,0}},
	        // Face S
	        {{'N',0},{10,0},{'R',5},{12,4},{'T',1},{11,0}},
	        // Face T
	        {{'O',0},{11,0},{'S',5},{12,2},{'P',1},{7,0}}
        };
    }

    namespace Test
    {
        using NUnit.Core;
        using NUnit.Framework;

        [TestFixture]
        public class Test
        {
            private bool TestFacesConnect(IcosFace from, IcosFace to)
            {
                foreach (eHexDirection d in Enum.GetValues(typeof(eHexDirection)))
                {
                    if (d != eHexDirection.knDirectionZero)
                    {
                        PyxMathConstants.ConnectivityInformation c =
                            PyxMathConstants.GetConnectivityRes1(from, d);
                        if (c.Destination != null)
                        {
                            if (c.Destination.ToString().CompareTo(to.ToString()) == 0)
                                return true;
                        }
                    }
                }
                return false;
            }

            private void TestFace(IcosFace face)
            {
                foreach (eHexDirection d in Enum.GetValues( typeof(eHexDirection)))
                {
                    if (d != eHexDirection.knDirectionZero)
                    {
                        PyxMathConstants.ConnectivityInformation c = 
                            PyxMathConstants.GetConnectivityRes1(face, d);
                        if (c.Destination != null)
                        {
                            if (!TestFacesConnect(c.Destination, face))
                                TestFacesConnect(c.Destination, face);
                            Assert.IsTrue(TestFacesConnect(c.Destination, face), 
                                String.Format("Face {0} connects one-way to face {1} (in direction {2})",
                                face, c.Destination, d));
                        }
                    }
                }
            }

            [Test]
            public void Connectivity()
            {
                IcosFace faceI = new IcosFace("I");
                IcosFace face10 = new IcosFace("10");

                PyxMathConstants.ConnectivityInformation connectivityI = 
                    PyxMathConstants.GetConnectivityRes1(faceI, eHexDirection.knDirectionFour);
                Assert.IsNotNull(connectivityI);
                Assert.AreEqual(connectivityI.Destination, face10);
                Assert.AreNotEqual(connectivityI.Destination, faceI);

                PyxMathConstants.ConnectivityInformation connectivity10 =
                    PyxMathConstants.GetConnectivityRes1(face10, eHexDirection.knDirectionOne);
                Assert.IsNotNull(connectivity10);
                Assert.AreEqual(connectivity10.Destination, faceI);
                Assert.AreNotEqual(connectivity10.Destination, face10);
            }

            [Test]
            public void ConnectivityIsBidirectional()
            {
                for (char face = 'A'; face <= 'T'; ++face)
                {
                    TestFace(new IcosFace( face.ToString()));
                }

                for (int vertex = 1; vertex <= 12; ++vertex)
                {
                    TestFace(new IcosFace(vertex.ToString()));
                }
            }
        }
    }
}
