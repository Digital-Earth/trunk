using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    class LimitedSizeDictionaryTests
    {
        [Test]
        public void Add_MoreTimesThanMax_KeepDictionarySameSize()
        {
            var dictionary = new LimitedSizeDictionary<string,string>(2)
            {
                {"a","one"},
                {"b","two"},
                {"c","three"},
            };

            Assert.AreEqual(2, dictionary.Count);
        }

        [Test]
        public void Add_MoreItemsThanMax_RemoveTheLastTouchedItem()
        {
            var dictionary = new LimitedSizeDictionary<string, string>(2)
            {
                {"a","one"},
                {"b","two"},
                {"c","three"}
            };

            Assert.True(!dictionary.ContainsKey("a"));            
        }

        [Test]
        public void Add_ItemTwice_ThrowsAnException()
        {
            var dictionary = new LimitedSizeDictionary<string, string>(2)
            {
                {"a","one"},
                {"b","two"},                
            };

            Assert.Throws<ArgumentException>(()=>dictionary.Add("a","three"));
        }

        [Test]
        public void GetItem_ChangeItemLastAccessTime()
        {
            var dictionary = new LimitedSizeDictionary<string, string>(2)
            {
                {"a","one"},
                {"b","two"},                
            };

            var item = dictionary["a"];
            dictionary.Add("c","three");

            Assert.True(dictionary.ContainsKey("a"));
        }
    }
}
