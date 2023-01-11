/******************************************************************************
Program.cs

begin      : 7/19/2007 11:50:00 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.IO;
using System.Text;
using Pyxis.Utilities;

namespace ManifestCreator
{
    /// <summary>
    /// Controller class to create a manifest file of the most current successful build/release.
    /// </summary>
    class Program
    {
        /// <summary>
        /// Main method of our application that is called upon execution.
        /// </summary>
        /// <param name="args">arg[0] will be the path to create a manifest for.
        ///                    Can be left blank to use default.</param>
        /// <returns>0 if sucessful; -1 if unsucessful.</returns>
        static int Main(string[] args)
        {
            String defaultExeFile = "PYXIS WorldView";
            String manifestFilename = "PYXIS.WorldView.CurrentBuild.manifest";
            String manifestFullFilename = "";
            String manifestPath;
            String installationDirectory = "";
            String version = "";

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("id|installdir", x => installationDirectory = x),
                new ArgsParser.Option("n|name", x => manifestFilename = x),
                new ArgsParser.Option("e|exe", x => defaultExeFile = x),
                new ArgsParser.Option("v|version", x => version = x)
                );

            // Make sure build path is included as command line arg or use default location for release.
            if (args.Length < 1)
            {
                Console.WriteLine("No build-directory path was supplied as a command line argument. Using default (Release)...");
                manifestPath = "\\\\aristotle\\Builds\\Releases\\WorldView";
            }
            else
            {
                manifestPath = args[0];
            }

            if (String.IsNullOrEmpty(installationDirectory))
            {
                installationDirectory = LastDirectory(manifestPath);
            }

            // Get the save path if it was supplied as a command line arg.
            if (args.Length > 1) // ignores any extra command line args.
            {
                manifestFilename = args[1]; // get the second command line arg.
            }

            // Build the full file name of the manifest file
            manifestFullFilename = manifestFilename;

            Console.WriteLine("Saving manifest in '{0}'.", manifestFullFilename);

            // Verify that the directory exists
            if (Directory.Exists(manifestPath))
            {
                try
                {
                    //--
                    //-- First attempt to create the manifest
                    //-- Build manifest with "ForceSynchronousCheckSums" flag set.
                    //--
                    ManifestBuilder manifestBuilder =
                        new ManifestBuilder(manifestPath, manifestFullFilename, true);
                    
                    manifestBuilder.Manifest.InstallationDirectory = installationDirectory;
                    manifestBuilder.Manifest.CommandLine = GuessRelativeCommandLine(manifestPath, defaultExeFile);
                    manifestBuilder.Build();
                    VerifyXML(manifestFullFilename);
                    CreateJSON(manifestFullFilename,version);
                    return 0;
                }
                catch (Exception exFirstFailure)
                {
                    Console.Error.WriteLine("An error occured during the creation of the manifest file: "
                        + exFirstFailure.Message);
                }

                return -1; // file not created
            }
            else
            {
                Console.Error.WriteLine("Cannot find application directory.");
                return -1; // file not created
            }
        }

        private static void CreateJSON(string manifestFile, string version)
        {
            string fileName = manifestFile + ".json";
            if (File.Exists(fileName))
            {
                File.Delete(fileName);
            }

            // Create a new file 
            using (FileStream fs = File.Create(fileName))
            {
                // Add some text to file
                var content = PrepareJson(manifestFile, version);
                Byte[] title = Encoding.UTF8.GetBytes(content);
                fs.Write(title, 0, title.Length);
            }
        }
        private static string PrepareJson(string filename,string version)
        {
            FileInfo info = new FileInfo(filename);
            var length = info.Length.ToString();
            var checksum = ChecksumSingleton.Checksummer.getFileCheckSum_synch(filename);
            string result = "{\"version\":\"" + version +
                "\",\"published\":true,\"current\":false,\"checksum\":\"" + checksum +
                "\",\"size\":" + length + 
                ",\"comments\":\"\",\"creationDate\":\"" + DateTime.Now + "\"}";
            return result;
        }

        /// <summary>
        /// Guesses the relative command line.
        /// </summary>
        /// <param name="path">The path of the directory.</param>
        /// <param name="defaultExecutableName">
        /// The fall-back executable file name to use, minus the extension, if multiples are found.
        /// </param>
        /// <returns></returns>
        private static string GuessRelativeCommandLine(string path, string defaultExecutableName)
        {
            string relativeCommandLine = "";

            // Search the top-level directory for files with extension "exe*",
            // according to http://msdn.microsoft.com/en-us/library/wz42302f.aspx.
            string [] executableFiles = Directory.GetFiles(path, "*.exe");

            switch (executableFiles.Length)
            {
                case 0:
                    Console.Error.WriteLine("No executable files found.  The manifest will not contain a command line.");
                    return "";
                case 1:
                    relativeCommandLine = executableFiles[0];
                    break;
                default:
                    // There's more than one exe, so see if the default file exists.
                    // Search the top-level directory for files with the default executable name and extension "exe",
                    // according to http://msdn.microsoft.com/en-us/library/wz42302f.aspx.
                    string[] defaultCandidates = Directory.GetFiles(path, defaultExecutableName + ".exe");
                    if (defaultCandidates.Length > 0)
                    {
                        // Return the first one that matches the default.
                        relativeCommandLine = defaultCandidates[0];
                        break;
                    }

                    // Otherwise, punt.
                    relativeCommandLine = executableFiles[0];
                    Console.Error.WriteLine(string.Format( 
                        "The directory contains {0} executable files.  Selecting {1} as default.",
                        executableFiles.Length, relativeCommandLine));
                    break;
            }

            // Verify that the file exists.
            if (!File.Exists(relativeCommandLine))
            {
                throw new ApplicationException(String.Format(
                    "The relative command line file {0} doesn't exist.", relativeCommandLine));
            }

            // Return just the file name.
            return Path.GetFileName(relativeCommandLine);
        }

        /// <summary>
        /// Gets the last directory in the given path.
        /// </summary>
        /// <param name="manifestPath">The manifest path.</param>
        /// <returns></returns>
        private static string LastDirectory(string manifestPath)
        {
            if ((manifestPath == null) || (manifestPath.Length == 0))
            {
                return "";
            }

            if (manifestPath[manifestPath.Length - 1] == Path.DirectorySeparatorChar)
            {
                return Path.GetFileName(
                    manifestPath.Substring( 0, manifestPath.Length - 1));
            }
            return Path.GetFileName(manifestPath);
        }

        /// <summary>
        /// Verify the manifest file produced by parsing it.
        /// (Syntactical analysis)
        /// If the manifest file is corrupt or incomplete, the xml reader
        /// will throw an error, which we will catch in Main.
        /// </summary>
        /// <param name="manifestFile"></param>
        private static void VerifyXML(String manifestFile)
        {
            try
            {
                Manifest manifest =
                    Manifest.ReadFromFile(manifestFile);
            }
            catch (Exception)
            {
                Console.Error.WriteLine("Error found when verifying manifest {0}.", manifestFile);
                throw;
            }
        }
    }
}
