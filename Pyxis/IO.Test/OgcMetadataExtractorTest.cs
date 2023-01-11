using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Sources.OGC;

namespace Pyxis.IO.Test
{
    [TestFixture]
    class OgcMetadataExtractorTest
    {
        [Test]
        public void NameIsExtractFromXml()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service xmlns=\"http://www.opengis.net/wms\"><Title>Tennessee_Geology</Title></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("Tennessee_Geology",metadata.Name);
        }

        [Test]
        public void DescriptionIsExtractFromXml()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service xmlns=\"http://www.opengis.net/wms\"><Abstract>some text</Abstract></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("some text", metadata.Description);
        }

        [Test]
        public void ExtractIgnoreNamesapce()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><Abstract>some text</Abstract></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("some text", metadata.Description);
        }

        [Test]
        public void ExtractIgnoreCase()
        {
            XmlDocument doc = new XmlDocument();
            //Note- lowercase "abstract"
            doc.LoadXml("<Service><abstract>some text</abstract><TITLE>title</TITLE></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("some text", metadata.Description);
            Assert.AreEqual("title", metadata.Name);
        }

        [Test]
        public void ExtractNameLookInsideDescriptionTag()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><TITLE>title</TITLE></description></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("title", metadata.Name);
        }

        [Test]
        public void ExtractDescriptionLookInsideDescriptionTag()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><abstract>abs</abstract></description></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("abs", metadata.Description);
        }


        [Test]
        public void ExtractKeywordsWorks()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywords><keyword>A</keyword><keyword>B</keyword></keywords></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new [] { "A" , "B"}, metadata.Tags);
        }


        [Test]
        public void ExtractCommaSeperatedKeywordsWorks()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywords><keyword>A, B, C</keyword></keywords></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "A", "B", "C"}, metadata.Tags);
        }


        [Test]
        public void ExtractSpaceSeperatedKeywordsWorks()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywords><keyword>A B C</keyword></keywords></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "A", "B", "C" }, metadata.Tags);
        }

        [Test]
        public void ExtractComplexKeywodsWorks()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywords><keyword>Hello world</keyword><keyword>Yey</keyword></keywords></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "Hello world", "Yey" }, metadata.Tags);
        }

        [Test]
        public void ExtractEmptyKeywordsSetTagsToNull()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywords><keyword /></keywords></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(null, metadata.Tags);
        }


        [Test]
        public void ExtractKeywordsWorksInsideDescription()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><keywords><keyword>A</keyword><keyword>B</keyword></keywords></description></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "A", "B" }, metadata.Tags);
        }

        [Test]
        public void ExtractKeywordListWorks()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><keywordlist><keyword>A</keyword><keyword>B</keyword></keywordlist></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "A", "B" }, metadata.Tags);
        }

        [Test]
        public void ExtractKeywordListWorksInsideDescription()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><keywordlist><keyword>A</keyword><keyword>B</keyword></keywordlist></description></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual(new[] { "A", "B" }, metadata.Tags);
        }

        [Test]
        public void ExtractFindAdditionalNodes()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><title>hi</title></description><myTag>Hello</myTag></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            string message = null;
            extractor.On("myTag",(node) => message = node.InnerText);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("hi", metadata.Name);
            Assert.AreEqual("Hello", message);
        }

        [Test]
        public void ExtractFindAdditionalNodesCaseInvariant()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><description><title>hi</title></description><myTag>Hello</myTag></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            string message = null;
            extractor.On("mytag", (node) => message = node.InnerText);
            extractor.Parse(doc.DocumentElement);

            Assert.AreEqual("hi", metadata.Name);
            Assert.AreEqual("Hello", message);
        }

        [Test]
        public void ExtractCanOverwriteDefaultSettings()
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml("<Service><title>hi</title><myTag>Hello</myTag></Service>");

            var metadata = new Metadata();
            var extractor = new OgcMetadataExtractor(metadata);
            string message = null;
            extractor.On("title", (node) => message = node.InnerText);
            extractor.Parse(doc.DocumentElement);

            //name not got updated anymore (default behavior)
            Assert.AreEqual(null, metadata.Name);

            //but our callback got called
            Assert.AreEqual("hi", message);
        }
    }
}
