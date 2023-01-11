using System.IO;
using System.Xml.Serialization;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for ManifestEntry
    /// </summary>
    [TestFixture]
    public class ManifestEntryTests
    {
        public static ManifestEntry CreateFakeManifestEntry()
        {
            return new ManifestEntry("Fake file name", "c:file.txt", "ssss", 1042);
        }

        private static void StreamToFile(string fileName, ManifestEntry contents)
        {
            XmlSerializer mySerializer = new XmlSerializer(typeof(ManifestEntry));
            // To write to a file, create a StreamWriter object.
            StreamWriter myWriter = new StreamWriter(fileName);
            mySerializer.Serialize(myWriter, contents);
            myWriter.Close();
        }

        private static ManifestEntry ReadFromFile(string fileName)
        {
            // Construct an instance of the XmlSerializer with the type
            // of object that is being deserialized.
            XmlSerializer myDeserializer = new XmlSerializer(typeof(ManifestEntry));
            // To read the file, create a FileStream.
            FileStream myFileStream = new FileStream(fileName, FileMode.Open);

            // Call the Deserialize method and cast to the object type.
            ManifestEntry deserializedObect = (ManifestEntry)
                myDeserializer.Deserialize(myFileStream);
            myFileStream.Close();

            return deserializedObect;
        }

        public static Pyxis.Utilities.TemporaryFile CreateFakeManifest(ManifestEntry contents)
        {
            Pyxis.Utilities.TemporaryFile result = new Pyxis.Utilities.TemporaryFile();
            result.Reset();
            StreamToFile(result.Name, contents);

            return result;
        }

        [Test]
        public void Serialization()
        {
            ManifestEntry myFileInformation = CreateFakeManifestEntry();
            using (Pyxis.Utilities.TemporaryFile manifestFile = CreateFakeManifest(myFileInformation))
            {
                ManifestEntry deserializedObect = ReadFromFile(manifestFile.Name);

                Assert.AreEqual(deserializedObect, myFileInformation,
                    "Serialized objects are not equal.");

                // Mangle the deserialized object to make sure that equality is working.
                deserializedObect.FileName = "";
                Assert.AreNotEqual(deserializedObect, myFileInformation,
                    "Different objects should not be equal.");
            }
        }
    }
}