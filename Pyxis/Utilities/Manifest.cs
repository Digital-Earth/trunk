using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Holds a "manifest", detailing a collection of resources (files, or 
    /// perhaps other manifests).
    /// </summary>
    [Serializable]
    public class Manifest
    {
        static System.Xml.Serialization.XmlSerializer s_xmlSerializer;
        static Manifest()
        {
            Type[] extendedTypemap = { typeof(ManifestEntry) };
            s_xmlSerializer = new System.Xml.Serialization.XmlSerializer(typeof(Manifest), extendedTypemap);
        }

        private string m_installationDirectory = "WorldView";

        /// <summary>
        /// Gets or sets the installation directory.  The contents of this
        /// manifest will be placed in WorldView-InstallDir/InstallationDirectory.
        /// </summary>
        /// <value>The installation directory.</value>
        public string InstallationDirectory
        {
            get { return m_installationDirectory; }
            set { m_installationDirectory = value; }
        }

        private string m_commandLine = "";

        /// <summary>
        /// Gets or sets the command line.  This is the command that launches
        /// the actual application, and can be null if there is nothing to 
        /// launch (or if this manifest is a supplement to another manifest.)
        /// </summary>
        /// <value>The command line.</value>
        public string CommandLine
        {
            get { return m_commandLine; }
            set { m_commandLine = value; }
        }

        private ManifestId m_id = new ManifestId();

        /// <summary>
        /// Gets or sets the id.
        /// </summary>
        /// <value>The id.</value>
        public ManifestId Id
        {
            get { return m_id; }
            set { m_id = value; }
        }

        private List<ManifestEntry> m_entries = new List<ManifestEntry>();

        /// <summary>
        /// Gets or sets the entries.
        /// </summary>
        /// <value>The entries.</value>
        public List<ManifestEntry> Entries
        {
            get { return m_entries; }
            set { m_entries = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Manifest"/> class.  Used for
        /// serialization only.
        /// </summary>
        public Manifest()
        {
        }

        // override object.Equals
        public override bool Equals(object obj)
        {
            //       
            // See the full list of guidelines at
            //   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpgenref/html/cpconequals.asp    
            // and also the guidance for operator== at
            //   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpgenref/html/cpconimplementingequalsoperator.asp
            //

            if (obj == null || GetType() != obj.GetType())
            {
                return false;
            }

            Manifest other = obj as Manifest;
            foreach (ManifestEntry entry in other.Entries)
            {
                if (!Entries.Contains(entry))
                {
                    return false;
                }
            }
            foreach (ManifestEntry ourEntry in Entries)
            {
                if (!other.Entries.Contains(ourEntry))
                {
                    return false;
                }
            }
            return other.Id == Id;
        }

        // override object.GetHashCode
        public override int GetHashCode()
        {
            return Entries.GetHashCode();
        }

        /// <summary>
        /// Verifies that the files in the specified base directory matches our checksums.
        /// </summary>
        /// <param name="baseDirectory">The base directory.</param>
        /// <returns></returns>
        public bool Verify(string baseDirectory)
        {
            foreach (ManifestEntry entry in this.Entries)
            {
                if (!entry.Verify(baseDirectory))
                {
                    return false;
                }
            }
            return true;
        }

        public string SerializeManifestToString()
        {
            System.IO.StringWriter outputStream = new System.IO.StringWriter(
                new System.Text.StringBuilder());

            s_xmlSerializer.Serialize(outputStream, this);

            // To cut down on the size of the xml, we'll strip extraneous stuff.
            string result = outputStream.ToString().Replace("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"", "");
            result = result.Replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"", "");
            result = result.Replace("<?xml version=\"1.0\" encoding=\"utf-16\"?>", "").Trim();

            return result;
        }

        public long TotalSize
        {
            get
            {
                return Entries.Sum(x => x.FileSize);
            }
        }


        #region Static/Helper Functions

        public static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

        /// <summary>
        /// Reads the manifest from a file.
        /// </summary>
        /// <param name="manifestFilePath">The manifest file path.</param>
        /// <returns>A new manifest.  (Throws if the manifest can't be read.)</returns>
        public static Manifest ReadFromFile(String manifestFilePath)
        {
            using (System.IO.TextReader sr = new System.IO.StreamReader(manifestFilePath))
            {
                return ReadFromStream(sr);
            }
        }

        /// <summary>
        /// Reads the manifest from a string.
        /// </summary>
        /// <param name="manifestText">The manifest text (an XML string).</param>
        /// <returns>
        /// A new manifest.  (Throws if the manifest can't be read.)
        /// </returns>
        public static Manifest ReadFromString(String manifestText)
        {
            using (System.IO.TextReader sr = new System.IO.StringReader(manifestText))
            {
                return ReadFromStream(sr);
            }
        }

        /// <summary>
        /// Reads a manifest from the stream.
        /// </summary>
        /// <param name="sr">The stream containing an XML manifest.</param>
        /// <returns></returns>
        private static Manifest ReadFromStream(System.IO.TextReader sr)
        {         
            try
            {
                object result = s_xmlSerializer.Deserialize(sr);
                return result as Manifest;
            }
            catch (Exception e)
            {
                Trace.WriteLine("Error reading manifest: {0}", e.Message);
                throw;
            }
        }

        #endregion Static/Helper Functions
    }
}
