using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for SimpleDatabase
    /// </summary>
    [TestFixture]
    public class SimpleDatabaseTests
    {
        [Serializable]
        public class DatabaseOuterTest
        {
            private List<DatabaseTestObject> m_collection = new List<DatabaseTestObject>();

            public List<DatabaseTestObject> Collection
            {
                get { return m_collection; }
                set { m_collection = value; }
            }

        }

        [Serializable]
        public class DatabaseTestObject
        {
            private int m_integerValue = 42;

            public int IntegerValue
            {
                get { return m_integerValue; }
                set { m_integerValue = value; }
            }
            private string m_stringValue = "Hello there";

            public string StringValue
            {
                get { return m_stringValue; }
                set { m_stringValue = value; }
            }
        }

        [Test]
        public void CreateDatabaseFromClassDefinitions()
        {
            SimpleDatabase.DisplayAllProviders();

            using (Pyxis.Utilities.TemporaryFile dbFilename = new TemporaryFile())
            {
                dbFilename.Delete();

                using (SimpleDatabase test = new SimpleDatabase(dbFilename.Name))
                {
                    // First, test for "object not found"
                    Guid id = Guid.NewGuid();
                    Assert.IsNull( test.Read<DatabaseOuterTest>(id), "The database should be empty.");

                    // Make a couple of objects...
                    DatabaseOuterTest myObject = new DatabaseOuterTest();
                    myObject.Collection.Add(new DatabaseTestObject());
                    myObject.Collection[0].StringValue = "This is not a default string.";

                    // Write an object to the database.
                    test.Write(myObject, id);

                    // And read it back.
                    DatabaseOuterTest readBack = test.Read<DatabaseOuterTest>( id);

                    // Finally, check to make sure the correct stuff was read back.
                    Assert.AreEqual(myObject.Collection.Count, readBack.Collection.Count,
                        "Database returned the wrong data.  (Wrong number of items in collection).");
                    for (int i = 0; i < myObject.Collection.Count; ++i)
                    {
                        Assert.AreEqual(myObject.Collection[i].IntegerValue, readBack.Collection[i].IntegerValue,
                            "Database returned the wrong data.  (Integer values did not match).");
                        Assert.AreEqual(myObject.Collection[i].StringValue, readBack.Collection[i].StringValue,
                            "Database returned the wrong data.  (Strings did not match).");
                    }
                }
            }
        }
    }
}