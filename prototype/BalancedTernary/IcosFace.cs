using System;
using System.Collections.Generic;
using System.Text;

namespace PYXIS.Mathematics
{
    public class IcosFace
    {
        // TODO: Make this an int....
        private string m_face;
        public IcosFace(string face)
        {
            m_face = face;
        }
        internal IcosFace(int face)
        {
            if (face < 13)
            {
                m_face = face.ToString();
            }
            else
            {
                m_face = ((char)face).ToString();
            }
        }
        public override string ToString()
        {
            return m_face;
        }
        private static System.Text.RegularExpressions.Regex s_isNumeric =
            new System.Text.RegularExpressions.Regex("[0-9]");
        private static System.Text.RegularExpressions.Regex s_isUppercase =
            new System.Text.RegularExpressions.Regex("[A-Z]");
        private static System.Text.RegularExpressions.Regex s_isLowercase =
            new System.Text.RegularExpressions.Regex("[a-z]");
        private int GetIntValue()
        {
            return m_face[0] - '1';
        }
        public bool IsVertex
        {
            get 
            { 
                return s_isNumeric.IsMatch( m_face);
            }
        }
        public int VertexIndex
        {
            get
            {
                if (s_isNumeric.IsMatch(m_face))
                {
                    int result = Convert.ToInt32(m_face) - 1;
                    System.Diagnostics.Trace.Assert(result < 12);
                    System.Diagnostics.Trace.Assert(result >= 0);
                    return result;
                }

                throw new InvalidOperationException("This IcosFace is not a Vertex.  Consider using IsVertex or FaceIndex");
            }
        }

        public bool IsFace
        {
            get { return s_isUppercase.IsMatch(m_face) || s_isLowercase.IsMatch(m_face); }
        }

        public int FaceIndex
        {
            get
            {
                if (s_isUppercase.IsMatch(m_face))
                    return m_face[0] - 'A';
                else if (s_isLowercase.IsMatch(m_face))
                    return m_face[0] - 'a';
                throw new InvalidOperationException("This IcosFace is not a Face (it is a Vertex).  Consider using IsFace, or VertexIndex.");
            }
        }

        public static implicit operator int(IcosFace f)
        {
            if (s_isNumeric.IsMatch(f.m_face))
                return f.m_face[0] - '0';
            else if (s_isUppercase.IsMatch(f.m_face))
                return f.m_face[0];
            else if (s_isLowercase.IsMatch(f.m_face))
                return f.m_face[0] - 'a' + 'A';
            throw new Exception(String.Format("Unexpected face {0}", f.m_face));
        }
    }

    namespace Test
    {
        using NUnit.Core;
        using NUnit.Framework;

        [TestFixture]
        public class IcosFaceTest
        {
            [Test]
            public void Construction()
            {
                IcosFace face = new IcosFace( "A");
                IcosFace alias = new IcosFace( "a");
                Assert.AreEqual( (int) face, (int) alias);
                IcosFace anotherFace = new IcosFace("2");
                Assert.AreNotEqual((int)face, (int)anotherFace);
               
            }

            [Test]
            public void IntegerConstruction()
            {
                for (int i = 1; i <= 12; ++i)
                {
                    IcosFace f = new IcosFace(i);
                    Assert.IsTrue(f.IsVertex, "Must be a vertex index");
                    Assert.Greater(f.VertexIndex, -1, "Vertex index must be non-negative");
                    Assert.Less(f.VertexIndex, 13, "Vertex index must be in range.");
                    Assert.IsFalse(f.IsFace, "Vertex cannot be a face");
                }

                for (int c = (int)'A'; c <= (int)'T'; ++c)
                {
                    IcosFace f = new IcosFace(c);
                    Assert.IsTrue(f.IsFace, "Must be a face index");
                    Assert.Greater(f.FaceIndex, -1, "Face index must be non-negative");
                    Assert.Less(f.FaceIndex, 31, "Face index must be in range.");
                    Assert.IsFalse(f.IsVertex, "Face cannot be a vertex.");
                }
            }
        }
    }
}