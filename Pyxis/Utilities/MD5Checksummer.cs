using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    public class MD5Checksummer : IChecksum 
    {
        #region IChecksum Members

        public string GetCheckSum(string data)
        {
            throw new NotImplementedException();
        }

        public string GetFileCheckSum(string file)
        {
            FileHash.FileHashGenerator fileHashGenerator =
                new FileHash.FileHashGenerator(file);

            return FileHash.FileHashGenerator.MD5ToHex(fileHashGenerator.GenerateMD5_deprecated());
        }

        public string FindFileMatchingChecksum(string checksum)
        {
            // Not implemented.  This class has no cache.  (Should never be called!)
            return "";
        }
        #endregion
    }
}
