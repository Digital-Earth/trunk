using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using Pyxis.Utilities;
using PyxNet.Publishing;

namespace PyxNet.Test.Publishing
{
    /// <summary>
    /// Testing for IDynamicPublishedItem class.
    /// </summary>
    [TestFixture]
    public class DynamicPublishedItemTests
    {
        static public class FakeSingleItemPublisher
        {
            static public int KeywordChanges { get; set; }

            static private QueryHashTable queryHashTable = new QueryHashTable();

            static public void OnKeywordsChanged(object sender, EventArgs e)
            {
                var publishedItem = sender as Publisher.IDynamicPublishedItem;
                KeywordChanges++;
                var newQueryHashTable = new QueryHashTable();
                foreach (var keyword in publishedItem.Keywords)
                {
                    newQueryHashTable.Add(keyword);
                }
                queryHashTable.Set(newQueryHashTable);
            }

            static public bool MayContain(string keyword)
            {
                return queryHashTable.MayContain(keyword);
            }
        }

        public class DynamicDataPublisher : Publisher.IDynamicPublishedItem
        {
            private List<string> m_keywords = new List<string>();
            public IEnumerable<string> Keywords { get { return m_keywords; } }

            public QueryResult Matches(Query query, Stack stack)
            {
                if (Keywords.FirstOrDefault(x => query.Contents.Contains(x)) != null)
                {
                    return new QueryResult(new Message());
                }
                return null;
            }

            public event EventHandler KeywordsChanged;

            public void AddKeywords(params string[] keywords)
            {
                var initialCount = m_keywords.Count;
                m_keywords.AddRange(keywords.Select(x => x.Trim().ToLower()).Distinct().Except(Keywords));
                if (initialCount != m_keywords.Count)
                {
                    KeywordsChanged.SafeInvoke(this);
                }
            }
        }

        /// <summary>
        /// Test keyword change event.
        /// </summary>
        [Test]
        public void TestChangeTriggered()
        {
            var dynamicDataPublisher = new DynamicDataPublisher();
            dynamicDataPublisher.KeywordsChanged += FakeSingleItemPublisher.OnKeywordsChanged;
            dynamicDataPublisher.AddKeywords("this", "is", "a", "test");
            dynamicDataPublisher.KeywordsChanged -= FakeSingleItemPublisher.OnKeywordsChanged;

            Assert.AreEqual(1, FakeSingleItemPublisher.KeywordChanges);
        }

        [Test]
        public void TestQHTHitAfterChange()
        {
            var addedKeyword = "Pyxis";

            var dynamicDataPublisher = new DynamicDataPublisher();
            dynamicDataPublisher.KeywordsChanged += FakeSingleItemPublisher.OnKeywordsChanged;
            dynamicDataPublisher.AddKeywords("this", "is", "a", "test");

            Assert.IsFalse(FakeSingleItemPublisher.MayContain(addedKeyword));

            dynamicDataPublisher.AddKeywords(addedKeyword);

            dynamicDataPublisher.KeywordsChanged -= FakeSingleItemPublisher.OnKeywordsChanged;

            Assert.IsTrue(FakeSingleItemPublisher.MayContain(addedKeyword));
        }
    }
}