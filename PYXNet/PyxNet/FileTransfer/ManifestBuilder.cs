using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace PyxNet.FileTransfer
{
    /// <summary>
    /// Class to create a manifest file using the build path it receives.
    /// </summary>
    public class ManifestBuilder
    {
        private static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool( false);

        /// <summary>
        /// Location of the most current build of WorldView.
        /// </summary>
        private String m_buildLocation;

        /// <summary>
        /// The full file name of the manifest file we are creating (includes full path).
        /// </summary>
        private String m_outputFileName;

        public String OutputFileName
        {
            get { return m_outputFileName; }
            set { m_outputFileName = value; }
        }


        /// <summary>
        /// Gets or sets a value indicating whether [force synchronous check sums].
        /// </summary>
        /// <value>
        /// 	<c>true</c> if [force synchronous check sums]; otherwise, <c>false</c>.
        /// </value>
        private bool ForceSynchronousCheckSums
        {
            get;
            set;
        }

        /// <summary>
        /// Constructor. For legacy usage, assumes "SynchronousCheckSums" flag is false.
        /// </summary>
        /// <param name="buildPath">The path of where the current build resides.</param>
        public ManifestBuilder(String buildPath, String fullFileName) : 
            this(buildPath,fullFileName,false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestBuilder"/> class.
        /// </summary>
        /// <param name="buildPath">The build path.</param>
        /// <param name="fullFileName">Full name of the file.</param>
        /// <param name="synchronousCheckSums">if set to <c>true</c> [synchronous check sums].</param>
        public ManifestBuilder(String buildPath, String fullFileName, bool synchronousCheckSums)  
        {
            m_buildLocation = buildPath;
            OutputFileName = fullFileName;
            ForceSynchronousCheckSums = synchronousCheckSums;
        }

        /// <summary>
        /// Creates the manifest file for the files which comprise the current build.
        /// </summary>
        public void Build()
        {
            string manifestText = GenerateManifestText();

            Trace.WriteLine("Writing manifest file...");

            // Write the file...
            if (File.Exists(OutputFileName))
            {
                File.Delete(OutputFileName);
            }
            using (StreamWriter sw = File.CreateText(OutputFileName))
            {
                sw.Write(manifestText);
                sw.Close();
            }

            Trace.WriteLine("Manifest file created.");
        }

        private List<string> m_extensionsToIgnore;

        /// <summary>
        /// Gets the extentions to ignore.
        /// </summary>
        /// <value>The extentions to ignore.</value>
        public List<string> ExtensionsToIgnore
        {
            get
            {
                // Lazy Initialization.
                if (m_extensionsToIgnore == null)
                {
                    m_extensionsToIgnore = new List<string>();
                    m_extensionsToIgnore.Add(".exp");
                    m_extensionsToIgnore.Add(".lib");
                    m_extensionsToIgnore.Add(".pdb");
                    m_extensionsToIgnore.Add(".xml");
                }

                return m_extensionsToIgnore;
            }
        }

        /// <summary>
        /// Populates a data structure of file names and relative directory
        /// paths. This method recursively searches the entire directory
        /// where the most current release resides.
        /// </summary>
        /// <param name="relativeDir">The current directory we are in. This is
        /// basically the full path for each file, minus the root directory where
        /// the most current release resides. We store a relative directory in the XML so
        /// each file entry has the data we need, and we won't have to do any special
        /// reading when the Launcher creates this files.</param>
        private IEnumerable<Pyxis.Utilities.ManifestEntry> GetFileInfo(string relativeDir)
        {
            char dirSeparator = System.IO.Path.DirectorySeparatorChar;

            DirectoryInfo currentDirectory = new DirectoryInfo(m_buildLocation + dirSeparator + relativeDir);

            foreach (FileInfo file in currentDirectory.GetFiles())
            {
                if (!ExtensionsToIgnore.Contains(file.Extension))
                {
                    yield return new Pyxis.Utilities.ManifestEntry(file, relativeDir, ForceSynchronousCheckSums);
                }
            }

            // Recursively search all of the current dir's subdirs
            foreach (DirectoryInfo subDir in currentDirectory.GetDirectories())
            {
                foreach (Pyxis.Utilities.ManifestEntry file in GetFileInfo(relativeDir + dirSeparator + subDir.Name))
                {
                    yield return file;
                }
            }
        }

        /// <summary>
        /// Generates the manifest and returns the textual (XML) representation of it.
        /// </summary>
        /// <returns></returns>
        private string GenerateManifestText()
        {
            return Manifest.SerializeManifestToString();
        }

        private Pyxis.Utilities.Manifest m_manifest = null;

        public Pyxis.Utilities.Manifest Manifest
        {
            get 
            {
                if (m_manifest == null)
                {
                    m_manifest = GenerateManifest();
                }
                return m_manifest; 
            }
        }

        /// <summary>
        /// Generates the manifest.
        /// </summary>
        /// <returns></returns>
        private Pyxis.Utilities.Manifest GenerateManifest()
        {
            Pyxis.Utilities.Manifest manifest = new Pyxis.Utilities.Manifest();

            // Write out each file and its relative directory path            
            foreach (Pyxis.Utilities.ManifestEntry file in GetFileInfo(""))
            {
                manifest.Entries.Add(file);
            }

            return manifest;
        }
    }
}
