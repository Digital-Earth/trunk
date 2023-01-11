using System.Collections.Generic;
using NUnit.Framework;

namespace PyxNet.Test
{
    /// <summary>
    /// Testing for QueryHashTable class.
    /// </summary>
    [TestFixture]
    public class QueryHashTableTests
    {
        /// <summary>
        /// Test strings used in some of the test functions.
        /// </summary>
        const string s1 = "This is one thing I could add.";
        const string lowerCases1 = "this is one thing i could add.";
        const string s2 = "Blue Marble";
        const string crazyCases2 = "BLue mArBlE";
        const string s3 = "Fred Borland";
        const string upperCases3 = "FRED BORLAND";
        const string s4 = "Some longer string that we can see if it hashes out to something.";
        const string s5 = "Just a little bit of nonsense.";
        const string s6 = "One more little string to test with.";

        /// <summary>
        /// Test construction.
        /// </summary>
        [Test]
        public void TestConstruction()
        {
            const int numToConstruct = 10;

            List<QueryHashTable> QHTs = new List<QueryHashTable>(numToConstruct);

            for (int count = 0; count < numToConstruct; ++count)
            {
                QHTs.Add(new QueryHashTable());
            }

            QHTs.Clear();
        }

        /// <summary>
        /// Test the addition and containment of strings.
        /// </summary>
        [Test]
        public void TestStringValues()
        {
            QueryHashTable QHT = new QueryHashTable();

            // Empty hash table shouldn't contain any strings.
            Assert.IsFalse(QHT.MayContain(s1));
            Assert.IsFalse(QHT.MayContain(s2));
            Assert.IsFalse(QHT.MayContain(s3));

            QHT.Add(s1);
            Assert.IsTrue(QHT.MayContain(s1));

            QHT.Add(s2);
            Assert.IsTrue(QHT.MayContain(s1));
            Assert.IsTrue(QHT.MayContain(s2));

            QHT.Add(s3);
            Assert.IsTrue(QHT.MayContain(s1));
            Assert.IsTrue(QHT.MayContain(s2));
            Assert.IsTrue(QHT.MayContain(s3));

            // the hash should be case insensitive
            Assert.IsTrue(QHT.MayContain(lowerCases1));
            Assert.IsTrue(QHT.MayContain(crazyCases2));
            Assert.IsTrue(QHT.MayContain(upperCases3));
        }

        /// <summary>
        /// Test the conversion to and from messages.
        /// </summary>
        [Test]
        public void TestMessageConversion()
        {
            QueryHashTable QHT = new QueryHashTable();
            QHT.Add(s2);
            QHT.Add(s4);
            QHT.Add(s6);

            // if they aren't there before, then how can we expect to see them later.
            Assert.IsTrue(QHT.MayContain(s2));
            Assert.IsTrue(QHT.MayContain(s4));
            Assert.IsTrue(QHT.MayContain(s6));

            // create a message from the QueryHashTable
            Message aMessage = QHT.ToMessage();

            // recreate the QueryHashTable from the message.
            QueryHashTable reconstructedQHT = new QueryHashTable(aMessage);

            // the same strings should hit the reconstructed table
            Assert.IsTrue(reconstructedQHT.MayContain(s2));
            Assert.IsTrue(reconstructedQHT.MayContain(s4));
            Assert.IsTrue(reconstructedQHT.MayContain(s6));
        }

        /// <summary>
        /// Test the combination of two hash tables.
        /// </summary>
        [Test]
        public void TestAddingHashTables()
        {
            QueryHashTable QHT1 = new QueryHashTable();
            QHT1.Add(s1);
            QHT1.Add(s2);
            QHT1.Add(s3);
            QueryHashTable QHT2 = new QueryHashTable();
            QHT2.Add(s4);
            QHT2.Add(s5);
            QHT2.Add(s6);

            // Add in the first hash table too.
            QHT2.Add(QHT1);

            // the combined hash should contain all the strings
            Assert.IsTrue(QHT2.MayContain(s1));
            Assert.IsTrue(QHT2.MayContain(s2));
            Assert.IsTrue(QHT2.MayContain(s3));
            Assert.IsTrue(QHT2.MayContain(s4));
            Assert.IsTrue(QHT2.MayContain(s5));
            Assert.IsTrue(QHT2.MayContain(s6));

            // the hash should be case insensitive
            Assert.IsTrue(QHT2.MayContain(lowerCases1));
            Assert.IsTrue(QHT2.MayContain(crazyCases2));
            Assert.IsTrue(QHT2.MayContain(upperCases3));
        }

        /// <summary>
        /// Test the firing of the OnChange event.
        /// </summary>
        [Test]
        public void TestOnChange()
        {
            bool onChangeFired = false;
            QueryHashTable QHT = new QueryHashTable();
            QHT.OnChange += delegate(object sender, QueryHashTable.ChangeEventArgs args) { onChangeFired = true; };

            QHT.Add (s1);
            // adding a string is a change.
            Assert.IsTrue(onChangeFired);

            onChangeFired = false;
            QHT.Add(QHT);
            // adding another table is a change too.
            Assert.IsTrue(onChangeFired);
        }
    }
}