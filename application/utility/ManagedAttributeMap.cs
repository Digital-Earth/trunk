/******************************************************************************
ManagedAttributeMap.cs

begin      : August 14, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// Wraps the swig-generated Attribute_Map and provides methods to 
    /// perform operations on it.
    /// </summary>
    public class ManagedAttributeMap
    {
        /// <summary>
        /// Gets the underlying attribute map.
        /// </summary>
        /// <value>The attribute map.</value>
        public Attribute_Map AttributeMap
        {
            get
            {
                return m_attributeMap;
            }
        }
        private Attribute_Map m_attributeMap = null;
        private Vector_String m_vecKeys = null;
        private Vector_String m_vecValues = null;

        /// <summary>
        /// Initializes a new instance of the <see cref="ManagedAttributeMap"/> class.
        /// </summary>
        public ManagedAttributeMap()
        {
            m_attributeMap = new Attribute_Map();
            m_vecKeys = new Vector_String();
            m_vecValues = new Vector_String();
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="ManagedAttributeMap"/> class.
        /// </summary>
        /// <param name="attributeMap">The attribute map.</param>
        public ManagedAttributeMap(Attribute_Map attributeMap)
        {
            if (attributeMap != null)
            {
                m_attributeMap = attributeMap;
                m_vecKeys = pyxlib.getKeysFromMap(m_attributeMap);
                m_vecValues = pyxlib.getValuesFromMap(m_attributeMap);
            }
        }

        public string this[string attributeName]
        {
            get
            {
                return GetValue(attributeName);
            }
            set
            {
                SetValue(attributeName, value);
            }
        }
        
        /// <summary>
        /// Gets the value for the provided key from attribute map.
        /// </summary>
        /// <param name="key">The key.</param>
        /// <returns></returns>
        private string GetValue(string key)
        {
            string value = string.Empty;

            if (m_attributeMap == null)
            {
                return value;
            }

            for (int i = 0; i < m_vecKeys.Count; ++i)
            {
                if (m_vecKeys[i] == key)
                {
                    value = m_vecValues[i];
                    break;
                }
            }

            return value;
        }

        /// <summary>
        /// Sets the value for the provided key.
        /// </summary>
        /// <param name="key">The key.</param>
        /// <param name="value">The value.</param>
        private void SetValue(
            string key,
            string value)
        {
            if (m_attributeMap != null)
            {
                m_attributeMap.set(key, value);
                m_vecKeys = pyxlib.getKeysFromMap(m_attributeMap);
                m_vecValues = pyxlib.getValuesFromMap(m_attributeMap);
            }
        }        

        /// <summary>
        /// Determines whether the attribute map contains the provided value 
        /// for the provided key.
        /// </summary>
        /// <param name="key">The key.</param>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        private bool ContainsValue(
            string key,
            string value)
        {
            if (m_attributeMap == null)
            {
                return false;
            }

            for (int i = 0; i < m_vecKeys.Count; ++i)
            {
                if (m_vecKeys[i] == key)
                {
                    return (m_vecValues[i] == value);
                }
            }

            return false;
        }
    }

    #region Unit Tests

    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for the ManagedAttributeMap.
        /// </summary>
        [TestFixture]
        public class ManagedAttributeMapTester
        {
            /// <summary>
            /// Tests that operations on a ManagedAttributeMap with an empty 
            /// AttributeMap do not fail.
            /// </summary>
            [Test]
            public void TestEmptyAttributeMap()
            {
                ManagedAttributeMap map = new ManagedAttributeMap();
                Assert.AreEqual(string.Empty, map["key1"]);
            }

            /// <summary>
            /// Tests that operations on a ManagedAttributeMap created using 
            /// its default constructor do not fail.
            /// </summary>
            [Test]
            public void TestWithDefaultConstructor()
            {
                ManagedAttributeMap map = new ManagedAttributeMap();
                map["key1"] = "value1";
                Assert.AreEqual("value1", map["key1"]);

                map["key1"] = "value2";
                Assert.AreEqual("value2", map["key1"]);
            }

            /// <summary>
            /// Tests that operations on a ManagedAttributeMap created using 
            /// an existing Attribute_Map do not fail.
            /// </summary>
            [Test]
            public void TestWithExistingAttributeMap()
            {
                Attribute_Map attributeMap = new Attribute_Map();
                attributeMap.set("key1", "value1");
                ManagedAttributeMap map = new ManagedAttributeMap(attributeMap);
                Assert.AreEqual("value1", map["key1"]);

                map["key1"] = "value2";
                Assert.AreEqual("value2", map["key1"]);

                // ensure the member AttributeMap is set by value
                map.AttributeMap.set("key1", "value3");
                Assert.AreEqual("value2", map["key1"]);                
            }
        }
    }

    #endregion Unit Tests
}
