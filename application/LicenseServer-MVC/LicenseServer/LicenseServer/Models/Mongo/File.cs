using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class File : Pyxis.Contract.Publishing.File, IMongoResource
    {
        // for deserializing from string
        public File()
        {
        }

        public File(List<LicenseReference> licenses, Metadata metadata, Guid version, string fileStamp, string mimeType, long? size)
            : base(licenses, metadata, version, fileStamp, mimeType, size)
        {
        }

        public File(File basedOnFile)
            : base(basedOnFile)
        {
        }

        public File(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string fileStamp, string mimeType, long? size)
            : base(id, licenses, metadata, version, fileStamp, mimeType, size)
        {
        }
    }
}