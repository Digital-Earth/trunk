using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// A checksum singleton. 
    /// </summary>
    public static class ChecksumSingleton
    {
        private static ChecksumCache s_checksummer;

        /// <summary>
        /// Get the checksummer.
        /// </summary>
        public static ChecksumCache Checksummer
        {
            get
            {
                if (s_checksummer == null)
                {
                    s_checksummer = new ChecksumCache();
                }
                return s_checksummer;
            }
        }
    }
}
