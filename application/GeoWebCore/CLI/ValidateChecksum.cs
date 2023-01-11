using Pyxis.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.CLI
{
    /// <summary>
    /// Provide CLI (command line interface) for validate all checksum for user downloaded files
    /// </summary>
    static class ValidateChecksum
    {
        public static bool Run()
        {
            foreach (var file in System.IO.Directory.EnumerateFiles(Program.RunInformation.GalleryFilesRoot, "*.*", System.IO.SearchOption.AllDirectories))
            {
                Console.WriteLine("{0} = {1}", System.IO.Path.GetFileName(file), ChecksumSingleton.Checksummer.getFileCheckSum_synch(file));
            }

            ChecksumSingleton.Checksummer.WriteChecksumCache();

            return true;
        }
    }
}
