using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for HttpTool
    /// </summary>
    [TestFixture]
    public class HttpToolTests
    {
        [Test]
        public void GoodUrl()
        {
            Assert.IsTrue(Pyxis.Utilities.HttpTool.UrlIsReachable("http://www.pyxisinnovation.com/", false),
                "Well known URL should be reachable.");
            Assert.IsFalse(Pyxis.Utilities.HttpTool.UrlIsReachable("http://www.slashdot.org", true),
                "Slashdot always redirects.");
        }
    }
}