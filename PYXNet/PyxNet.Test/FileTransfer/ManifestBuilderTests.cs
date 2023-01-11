using NUnit.Framework;
using Pyxis.Utilities;
using Pyxis.Utilities.Test;
using PyxNet.FileTransfer;

namespace PyxNet.Test.FileTransfer
{
    /// <summary>
    /// Unit tests for ManifestBuilder
    /// </summary>
    [TestFixture]
    public class ManifestBuilderTests
    {
        [Test]
        public void GenerateRandomManifest()
        {
            // Create a directory containing random stuff...
            string dataDirectory = TestData.CreateRandomDirectory();

            // Generate the manifest.
            using (Pyxis.Utilities.TemporaryFile manifestFileName = new Pyxis.Utilities.TemporaryFile())
            {
                ManifestBuilder manifestBuilder = new ManifestBuilder(
                    dataDirectory, manifestFileName.Name);
                manifestBuilder.Build();

                // Quick check 
                Manifest result = Manifest.ReadFromFile(manifestFileName.Name);

                // TODO: Test for contents being exactly the files that we want.
            }
        }
    }
}