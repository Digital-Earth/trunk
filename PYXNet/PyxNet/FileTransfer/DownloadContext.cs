using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.FileTransfer
{
    /// <summary>
    /// Manages the download process, in terms of file location, verification, 
    /// and movement between the temp directory and the final directory.
    /// </summary>
    public class DownloadContext
    {
        #region Properties

        /// <summary>
        /// Base directory for temporary files.
        /// </summary>
        private string m_tempDirectory =
            GetPathWithSeparator(PyxNet.FileTransfer.ManifestDownloader.CacheDirectory);

        /// <summary>
        /// Gets or sets the base directory for temporary files.
        /// </summary>
        /// <value>The temp directory.</value>
        public string TempDirectory
        {
            get { return m_tempDirectory; }
            set { m_tempDirectory = value; }
        }
        	
        /// <summary>
        /// Gets or sets the base directory for final storage.
        /// </summary>
        /// <value>The final directory.</value>
        public string FinalDirectory
        {
            get; set;
        }

        #endregion Properties

        #region Constructor

        /// <summary>
        /// Returns the given path, with a terminal separator if there is not one already.
        /// </summary>
        /// <param name="path">The path with a separator at the end.</param>
        /// <returns></returns>
        private static string GetPathWithSeparator(string path)
        {
            if (path.EndsWith(System.IO.Path.DirectorySeparatorChar.ToString()) ||
                path.EndsWith(System.IO.Path.AltDirectorySeparatorChar.ToString()))
            {
                return path;
            }
            else
            {
                return path + System.IO.Path.DirectorySeparatorChar;
            }
        }

        public DownloadContext(string finalDirectory)
        {
            FinalDirectory = GetPathWithSeparator(finalDirectory);
        }

        public DownloadContext(string finalDirectory, string tempDirectory) :
            this( finalDirectory)
        {
            m_tempDirectory = GetPathWithSeparator(tempDirectory);
        }

        #endregion Constructor

        /// <summary>
        /// Constructs the final directory for the given filespec (directory only - excludes filename).
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public string ConstructFinalDirectory(Pyxis.Utilities.ManifestEntry filespec)
        {
            return System.IO.Path.Combine(this.FinalDirectory, filespec.FilePath);
        }

        /// <summary>
        /// Constructs the temp directory for the given filespec (directory only - excludes filename).
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public string ConstructTempDirectory(Pyxis.Utilities.ManifestEntry filespec)
        {
            // the fileStamp might have '/' or '\' - which make the Path.Combine to fail. so we replace them with '_'

            return System.IO.Path.Combine(this.TempDirectory, filespec.FileStampAsDirectoryName);
        }

        /// <summary>
        /// Constructs the final path for the given filespec.
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public string ConstructFinalPath(Pyxis.Utilities.ManifestEntry filespec)
        {
            return System.IO.Path.Combine(ConstructFinalDirectory(filespec), filespec.FileName);
        }

        /// <summary>
        /// Constructs the temp path for the given filespec.
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public string ConstructTempPath(Pyxis.Utilities.ManifestEntry filespec)
        {
            return System.IO.Path.Combine(ConstructTempDirectory(filespec), filespec.FileName);
        }

        /// <summary>
        /// Verifies that the file matches the given checksum.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <param name="checksum">The checksum.</param>
        /// <returns></returns>
        public static bool VerifyFile(string filename, Pyxis.Utilities.ManifestEntry filespec)
        {
            // If there is no file, then the checksum doesn't match!
            if (!System.IO.File.Exists(filename))
            {
                return false;
            }

            //validate the file have the expected size we are looking for
            if (new System.IO.FileInfo(filename).Length != filespec.FileSize)
            {
                return false;
            }

            // If we have no checksum, then we match anything.
            if (filespec.FileStamp == null)
            {
                return true;
            }

            //validate the checksum
            string fileChecksum = Pyxis.Utilities.ChecksumSingleton.Checksummer.getFileCheckSum_synch(filename);
            return filespec.FileStamp.Equals(fileChecksum);            
        }

        /// <summary>
        /// Does the given filespec exist in the temp directory?
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public bool ExistsInTemp(Pyxis.Utilities.ManifestEntry filespec)
        {
            string tempDirectory = ConstructTempDirectory(filespec);
            if (!System.IO.Directory.Exists(tempDirectory))
            {
                return false;
            }
            return (System.IO.Directory.Exists(tempDirectory) &&
                (System.IO.Directory.GetFiles(tempDirectory, "*.*").Length > 0));
        }

        /// <summary>
        /// Finds the file in temp directory, if it exists (and is correct).
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns>The name of the (first) file that matches.</returns>
        private string FindInTemp(Pyxis.Utilities.ManifestEntry filespec)
        {
            // Check for the file in the temp directory.
            string tempPath = ConstructTempPath(filespec);
            if (VerifyFile(tempPath, filespec))
            {
                return tempPath;
            }

            // If there is a sub-directory below the temp dir, matching the checksum, then look there too.
            string tempDirectory = ConstructTempDirectory(filespec);
            if (!System.IO.Directory.Exists(tempDirectory))
            {
                return null;
            }

            foreach (string filename in System.IO.Directory.GetFiles(tempDirectory, "*.*"))
            {
                if (VerifyFile(filename, filespec))
                {
                    return filename;
                }
            }
            return null;
        }
        /// <summary>
        /// Verifies that the filespec matches its checksum in the temp directory.
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public bool VerifyTemp(Pyxis.Utilities.ManifestEntry filespec)
        {
            return (FindInTemp(filespec) != null);
        }

        /// <summary>
        /// Does the given filespec exist in the final directory?
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public bool ExistsInFinal(Pyxis.Utilities.ManifestEntry filespec)
        {
            return System.IO.File.Exists(ConstructFinalPath(filespec));
        }

        /// <summary>
        /// Verifies that the filespec matches its checksum in the final directory.
        /// </summary>
        /// <param name="filespec">The filespec.</param>
        /// <returns></returns>
        public bool VerifyFinal(Pyxis.Utilities.ManifestEntry filespec)
        {
            return filespec.Verify(this.FinalDirectory);
        }

        public static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false, "Download Context");

        /// <summary>
        /// Moves the given filespec from temp to final, overwriting if necessary.
        /// </summary>
        /// <param name="filespec">The filespec to move.</param>
        public void MoveToFinal(Pyxis.Utilities.ManifestEntry filespec)
        {
            // Construct the path strings.
            string tempPath = FindInTemp( filespec);
            if (tempPath == null)
            {
                throw new InvalidOperationException("File not found.");
            }
            string finalPath = ConstructFinalPath(filespec);

            // Trace.
            Trace.WriteLine("Moving {0}[{1}/{2}] to {3}[{4}/{5}].",
                tempPath,
                ExistsInTemp(filespec),
                VerifyTemp(filespec),
                finalPath,
                ExistsInFinal(filespec),
                VerifyFinal(filespec)
                );

            // Delete any file that already exists at that path.
            if (ExistsInFinal(filespec))
            {
                System.IO.File.Delete(finalPath);
            }

            // Create the final directory, if not already there.
            string outputDirectory =
                System.IO.Path.GetDirectoryName(finalPath);
            if (!System.IO.Directory.Exists(outputDirectory))
            {
                System.IO.Directory.CreateDirectory(outputDirectory);
            }

            // Move from temp to final.
            System.IO.File.Move(tempPath, finalPath);
        }

        /// <summary>
        /// Copies the given filespec from temp to final, overwriting if necessary.
        /// </summary>
        /// <param name="filespec">The filespec to copy.</param>
        internal void CopyToFinal(Pyxis.Utilities.ManifestEntry filespec)
        {
            // Construct the path strings.
            string tempPath = FindInTemp(filespec);
            string finalPath = ConstructFinalPath(filespec);

            // Trace.
            Trace.DebugWriteLine("Copying {0}[{1}/{2}] to {3}[{4}/{5}].",
                tempPath,
                ExistsInTemp(filespec),
                VerifyTemp(filespec),
                finalPath,
                ExistsInFinal(filespec),
                VerifyFinal(filespec)
                );

            for (int i = 3; i >= 0; --i)
            {
                // Occasionally another thread might be using this file.  Keep trying on failure.
                try
                {
                    // Delete any file that already exists at that path.
                    if (ExistsInFinal(filespec))
                    {
                        System.IO.File.Delete(finalPath);
                    }

                    // Create the final directory, if not already there.
                    string outputDirectory =
                        System.IO.Path.GetDirectoryName(finalPath);
                    if (!System.IO.Directory.Exists(outputDirectory))
                    {
                        System.IO.Directory.CreateDirectory(outputDirectory);
                    }

                    // Copy from temp to final.
                    System.IO.File.Copy(tempPath, finalPath);
                }
                catch (Exception ex)
                {
                    if (i == 0)
                    {
                        throw;
                    }
                    Trace.WriteLine(
                        "Ignoring exception during FileTransfer.CopyToFinal of %0. %1",
                        finalPath, ex.ToString());
                    System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(500));
                }
            }
        }

        /// <summary>
        /// A single manifest entry, mapped to a specific download context.  
        /// This is used to verify that the download was accomplished, and 
        /// to move/deploy the file to the final location.
        /// </summary>
        public class DownloadedManifestEntry
        {
            private DownloadContext m_downloadContext;
            private Pyxis.Utilities.ManifestEntry m_manifestEntry;

            /// <summary>
            /// Gets the manifest entry.
            /// </summary>
            /// <value>The manifest entry.</value>
            public Pyxis.Utilities.ManifestEntry ManifestEntry
            {
                get { return m_manifestEntry; }
            }

            public DownloadedManifestEntry(
                DownloadContext downloadContext, Pyxis.Utilities.ManifestEntry manifestEntry)
            {
                m_downloadContext = downloadContext;
                m_manifestEntry = manifestEntry;
            }

            private bool? m_valid = null;

            /// <summary>
            /// Gets a value indicating whether this <see cref="DownloadedManifestEntry"/> is valid.
            /// "Valid" is defined as existing in either the final or temp directory, with the 
            /// correct checksum.
            /// </summary>
            /// <value><c>true</c> if valid; otherwise, <c>false</c>.</value>
            public bool Valid
            {
                get 
                {
                    if (m_valid.HasValue == false)
                    {
                        if (m_downloadContext.VerifyFinal(m_manifestEntry))
                        {
                            m_valid = true;
                            m_needsUpdate = false;
                        }
                        else if (m_downloadContext.VerifyTemp(m_manifestEntry))
                        {
                            m_valid = true;
                            m_needsUpdate = true;
                        }
                        else
                        {
                            m_valid = false;
                            m_needsUpdate = true; //??
                        }
                    }
                    return m_valid.Value; 
                }
            }

            private bool? m_needsUpdate = null;

            /// <summary>
            /// Gets a value indicating whether this file needs to be updated 
            /// to the final directory.
            /// </summary>
            /// <value><c>true</c> if [needs update]; otherwise, <c>false</c>.</value>
            public bool NeedsUpdate
            {
                get
                {
                    return Valid && m_needsUpdate.Value;
                }
            }

            /// <summary>
            /// Updates this instance, copying it to the output directory if necessary.
            /// </summary>
            public void Update()
            {
                if (!this.Valid)
                {
                    throw new InvalidOperationException(
                        string.Format("Cannot update file {0}, no valid copy found.",
                        m_downloadContext.ConstructFinalPath(m_manifestEntry)));
                }

                if (m_needsUpdate.Value == true)
                {
                    m_downloadContext.CopyToFinal(m_manifestEntry);
                    m_needsUpdate = false;
                }
            }

            /// <summary>
            /// Gets the location that the file is currently in.
            /// </summary>
            /// <value>The location.</value>
            public string Location
            {
                get
                {
                    if (!this.Valid)
                    {
                        throw new InvalidOperationException(
                            string.Format("Cannot update file {0}, no valid copy found.",
                            m_downloadContext.ConstructFinalPath(m_manifestEntry)));
                    }

                    if (m_needsUpdate.Value == true)
                    {
                        return m_downloadContext.ConstructTempPath(m_manifestEntry);
                    }
                    else
                    {
                        return m_downloadContext.ConstructFinalPath(m_manifestEntry);
                    }
                }
            }

            public override string ToString()
            {
                return this.Location;
            }
        }

        /// <summary>
        /// This is a manifest tied to a specific download context.  It is used to 
        /// track the progress of a download, to report on files that need to be 
        /// downloaded, and to manage the "update" of those files.
        /// </summary>
        public class DownloadedManifest
        {
            private DownloadContext m_downloadContext;

            private Pyxis.Utilities.Manifest m_manifest;
            public Pyxis.Utilities.Manifest Manifest
            {
                get { return m_manifest; }
                set { m_manifest = value; }
            }

            private List<DownloadedManifestEntry> m_entries = new List<DownloadedManifestEntry>();
            public List<DownloadedManifestEntry> Entries
            {
                get { return m_entries; }
            }

            public Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

            public DownloadedManifest(
                DownloadContext downloadContext, Pyxis.Utilities.Manifest manifest)
            {
                m_downloadContext = downloadContext;
                m_manifest = manifest;
                foreach (Pyxis.Utilities.ManifestEntry entry in manifest.Entries)
                {
                    m_entries.Add( new DownloadedManifestEntry( m_downloadContext, entry));
                }

                Trace.DebugWriteLine(
                    "Created a DownloadedManifest checker with {0} records.",
                    m_entries.Count);
            }

            private bool? m_valid = null;
            public bool Valid
            {
                get 
                {
                    if (m_valid.HasValue == false)
                    {
                        m_valid = true;
                        foreach (DownloadedManifestEntry entry in m_entries)
                        {
                            if (!entry.Valid)
                            {
                                m_valid = false;
                            }
                        }
                    }
                    return m_valid.Value; 
                }
            }

            public void Update()
            {
                if (!this.Valid)
                {
                    Trace.DebugWriteLine("Update called on invalid DownloadedContext.");
                    throw new InvalidOperationException(
                        "Cannot update manifest: no valid copy found.");
                }

                foreach (DownloadedManifestEntry entry in m_entries)
                {
                    Trace.DebugWriteLine("Updating {0} (was {1}).  {2}",
                        entry.ToString(), entry.Valid, entry.Location);
                    entry.Update();
                }
            }
        }

// ALSO support "in-memory" files.
//	 FileContents[ ManifestEntry] (get, set)

    }
}
