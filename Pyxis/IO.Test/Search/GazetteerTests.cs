using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Search;

namespace Pyxis.IO.Test.Search
{
    [TestFixture]
    class GazetteerTests
    {
        [Test]
        public void AddDatasetWithoutMetadataWorks()
        {
            var gazetteer = new Gazetteer();

            Assert.DoesNotThrow(()=>
            {
                gazetteer.Add(new DataSet()
                {
                    Uri = "http://localhost/dataset/1"
                });
            });


            Assert.AreEqual(0, gazetteer.Search(new TextScorer("dataset")).Count());
        }

        [Test]
        public void AddDatasetWitMetadataWorks()
        {
            var gazetteer = new Gazetteer();

            Assert.DoesNotThrow(() =>
            {
                gazetteer.Add(new DataSet()
                {
                    Uri = "http://localhost/dataset/1",
                    Metadata =
                    {
                        Name = "amazing dataset"
                    }
                });
            });


            Assert.AreEqual(1, gazetteer.Search(new TextScorer("dataset")).Count());
        }
    }
}
