using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class UriQueryBuilderTests
    {
        [Test]
        public void ParseUriAndParamaters()
        {
            var queryBuilder = new UriQueryBuilder("http://www.pyxisinnovation.com/api/products?p=WorldView&v=10.0.0.10");

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products", queryBuilder.ServerUri);
            Assert.AreEqual(2,queryBuilder.Parameters.Count);
            Assert.AreEqual("WorldView", queryBuilder.Parameters["p"]);
            Assert.AreEqual("10.0.0.10", queryBuilder.Parameters["v"]);
        }

        [Test]
        public void ParseParamatersWithSeveralValues()
        {
            var queryBuilder = new UriQueryBuilder("http://www.pyxisinnovation.com/api/products?p=WorldView&p=GWSS&v=10.0.0.10&p=Hub");

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products", queryBuilder.ServerUri);
            Assert.AreEqual(2, queryBuilder.Parameters.Count);
            Assert.AreEqual(new []{"WorldView","GWSS","Hub"}, queryBuilder.Parameters.GetValues("p"));
            Assert.AreEqual(new []{"10.0.0.10"}, queryBuilder.Parameters.GetValues("v"));
        }

        [Test]
        public void SetValuesBackToUrl()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&p=GWSS&v=10.0.0.10&p=Hub";
            var orderedUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&p=GWSS&p=Hub&v=10.0.0.10"; //all 'p' paramters are togther
            var queryBuilder = new UriQueryBuilder(originalUrl);

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products", queryBuilder.ServerUri);
            Assert.AreEqual(2, queryBuilder.Parameters.Count);
            Assert.AreEqual(new [] { "WorldView", "GWSS", "Hub"}, queryBuilder.Parameters.GetValues("p"));
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual(orderedUrl, finalUri);
        }

        [Test]
        public void KeepingUrlWithNonDefaultPort()
        {
            var originalUrl = "http://www.pyxisinnovation.com:8080/api/products?p=WorldView&v=10.0.0.10";                
            var queryBuilder = new UriQueryBuilder(originalUrl);
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual(originalUrl, finalUri);
        }

        [Test]
        public void RemoveParameter()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&v=10.0.0.10&user=Idan&p=Hub";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.RemoveParameter("user");
                
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?p=WorldView&p=Hub&v=10.0.0.10", finalUri);
        }

        [Test]
        public void RemoveParamterThatDoesNotExists()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&v=10.0.0.10&p=Hub";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.RemoveParameter("user");

            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?p=WorldView&p=Hub&v=10.0.0.10", finalUri);
        }

        [Test]
        public void RemoveParamterWithManyValues()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&v=10.0.0.10&user=Idan&p=Hub";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.RemoveParameter("p");

            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan", finalUri);
        }

        [Test]
        public void OverwriteParamterWithManyValues()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=WorldView&v=10.0.0.10&user=Idan&p=Hub";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.OverwriteParameter("p","GWSS");

            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan&p=GWSS", finalUri);
        }

        [Test]
        public void OverwriteParamterWithNoValues()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.OverwriteParameter("p", "GWSS");

            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan&p=GWSS", finalUri);
        }

        [Test]
        public void DealingOkWithParamtersWithNoValues()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan&emptyParamter=&someParameter=";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan&emptyParamter=&someParameter=", finalUri);
        }

        [Test]
        public void SetDefaultValueForParamter()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.SetDefaultParameter("p", "WorldView");
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?v=10.0.0.10&user=Idan&p=WorldView", finalUri);
        }

        [Test]
        public void SetDefaultValueForParamterDoesntOverwrite()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products?p=GWSS&v=10.0.0.10&user=Idan";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.SetDefaultParameter("p", "WorldView");
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?p=GWSS&v=10.0.0.10&user=Idan", finalUri);
        }

        [Test]
        public void EncodingValuesCorrectly()
        {
            var originalUrl = "http://www.pyxisinnovation.com/api/products";
            var queryBuilder = new UriQueryBuilder(originalUrl);

            queryBuilder.SetDefaultParameter("p", "PYXIS World View");
            queryBuilder.SetDefaultParameter("v", "current&public");
            var finalUri = queryBuilder.ToString();

            Assert.AreEqual("http://www.pyxisinnovation.com/api/products?p=PYXIS%20World%20View&v=current%26public", finalUri);

            var parsedParamters = new UriQueryBuilder(finalUri);

            Assert.AreEqual("PYXIS World View",parsedParamters.Parameters["p"]);
            Assert.AreEqual("current&public", parsedParamters.Parameters["v"]);
        }
    }
}