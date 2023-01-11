using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    public interface IChecksum
    {
        /// <summary>
        /// Calculate a checksum on the contents of a string then santitize
        /// the checksum by converting to a base 64 string to ensure that only
        /// UTF characters remain in the checksum string.
        /// </summary>
        /// <param name="data">The data that will be checksumed.</param>
        /// <returns>Returns the sanitized checksum, as a base 64 string. </returns>
        String GetCheckSum(String data);

        /// <summary>
        /// Calculate a checksum on the contents of a file then santitize
        /// the checksum by converting to a base 64 string to ensure that only
        /// UTF characters remain in the checksum string.
        /// </summary>
        /// <param name="data">The name of the file that will be checksumed.</param>
        /// <returns>Returns the sanitized checksum, as a base 64 string. </returns>
        String GetFileCheckSum(String file);

        /// <summary>
        /// Finds the file matching the given checksum.
        /// </summary>
        /// <param name="checksum">The checksum.</param>
        /// <returns>The matching file's path, or empty string if no match found.</returns>
        string FindFileMatchingChecksum(string checksum);
    }
}
