using System.IO;
using System.Xml.Serialization;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for Manifest
    /// </summary>
    [TestFixture]
    public class ManifestTests
    {
        [Test]
        public void Serialization()
        {
            string fileName = "TestManifest.xml";
            try
            {
                if (System.IO.File.Exists(fileName))
                {
                    System.IO.File.Delete(fileName);
                }

                Manifest myManifest = new Manifest();
                myManifest.Entries.Add(new ManifestEntry("File name", "c:file.txt", "ssss", 1042));
                myManifest.Entries.Add(new ManifestEntry("File name2", "c:file2.txt", "ssss    ", 10421));

                XmlSerializer mySerializer = new XmlSerializer(typeof(Manifest));
                // To write to a file, create a StreamWriter object.
                StreamWriter myWriter = new StreamWriter(fileName);
                mySerializer.Serialize(myWriter, myManifest);
                myWriter.Close();

                // Construct an instance of the XmlSerializer with the type
                // of object that is being deserialized.
                XmlSerializer myDeserializer = new XmlSerializer(typeof(Manifest));
                // To read the file, create a FileStream.
                FileStream myFileStream = new FileStream(fileName, FileMode.Open);

                // Call the Deserialize method and cast to the object type.
                Manifest deserializedObect = (Manifest)
                    myDeserializer.Deserialize(myFileStream);
                myFileStream.Close();

                Assert.AreEqual(deserializedObect, myManifest,
                    "Serialized objects are not equal.");

                deserializedObect.Id = new ManifestId();
                Assert.AreNotEqual(deserializedObect, myManifest,
                    "Different objects should not be equal.");
            }
            finally
            {
                if (System.IO.File.Exists(fileName))
                {
                    System.IO.File.Delete(fileName);
                }
            }

        }
    }
}