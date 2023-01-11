namespace Pyxis.Utilities.Test
{
    [NUnit.Framework.TestFixture]
    public class ManifestIdTests
    {
        /// <summary>
        /// Test the copy constructor for ManifestId.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestManifestIdCopyConstruction()
        {
            ManifestId test = new ManifestId();
            ManifestId reconstructed = new ManifestId(test);

            NUnit.Framework.Assert.IsTrue(test == reconstructed,
                "The ManifestId was not the same after copy.");
        }
    }
}