using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using NUnit.Framework;
using Pyxis.IO.Sources.OGC;

namespace Pyxis.IO.Test
{
    [TestFixture]
    class XmlNodeExtractorTest
    {
        private XmlDocument GetDocument()
        {
            var doc = new XmlDocument();
            doc.LoadXml("<book><title>Tomorrow\'s Children: Eighteen Tales of Fantasy and Science Fiction</title><price>135.55</price><pages>432</pages><year>1966</year><author><name>Isaac Asimov</name><born>January 2, 1920</born></author></book>");
            return doc;
        }

        private XmlDocument GetNestedDocument()
        {
            var doc = new XmlDocument();
            doc.LoadXml("<root><a><b><c1></c1><c2><d>Found Me</d></c2></b></a></root>");
            return doc;
        }

        [Test]
        public void AddMultipleHeandlerWorks()
        {
            var title = "";
            var year = 0;
            var doc = GetDocument();            

            var extractor = new XmlNodeExtractor();

            extractor.On("title", (node) => title = node.InnerText);
            extractor.On("year", (node) => year = int.Parse(node.InnerText));
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("Tomorrow\'s Children: Eighteen Tales of Fantasy and Science Fiction",title);
            Assert.AreEqual(1966, year);
        }

        [Test]
        public void OverwriteHeandlerWorks()
        {
            var yearAsString = "";
            var year = 0;
            var doc = GetDocument();

            var extractor = new XmlNodeExtractor();

            //this will be overwritten by next statement
            extractor.On("year", (node) => yearAsString = node.InnerText);
            extractor.On("year", (node) => year = int.Parse(node.InnerText));
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("", yearAsString);
            Assert.AreEqual(1966, year);
        }

        [Test]
        public void NestedPathWorks()
        {
            var name = "";
            var doc = GetDocument();

            var extractor = new XmlNodeExtractor();

            extractor.On(new []{"author","name"}, (node) => name = node.InnerText);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("Isaac Asimov", name);
        }

        [Test]
        public void DeepPathWorks()
        {
            var message = "";
            var doc = GetNestedDocument();

            var extractor = new XmlNodeExtractor();

            extractor.On(new[] { "a", "b", "c2", "d"}, (node) => message = node.InnerText);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("Found Me", message);
        }

        [Test]
        public void NestedXmlNodeExtractorWorks()
        {
            var name = "";
            var born = "";
            var doc = GetDocument();

            var extractor = new XmlNodeExtractor();

            extractor.On("author", new XmlNodeExtractor()
                .On("name", node => name = node.InnerText)
                .On("born", node => born = node.InnerText));
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("Isaac Asimov", name);
            Assert.AreEqual("January 2, 1920", born);
        }
    }
}
