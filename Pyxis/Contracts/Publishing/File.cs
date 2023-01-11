using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Publishing
{
    public class File : Resource
    {
        public string FileStamp { get; set; }
        public string MimeType { get; set; }
        public long? Size { get; set; }

        // for deserializing from string
        public File()
        {
        }

        public File(List<LicenseReference> licenses, Metadata metadata, Guid version, string fileStamp, string mimeType, long? size)
            : base(ResourceType.File, licenses, metadata, version)
        {
            FileStamp = fileStamp;
            MimeType = mimeType;
            Size = size;
        }

        public File(File basedOnFile)
            : base(basedOnFile)
        {
            FileStamp = basedOnFile.FileStamp;
            MimeType = basedOnFile.MimeType;
            Size = basedOnFile.Size;
        }

        public File(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string fileStamp, string mimeType, long? size)
            : this(licenses, metadata, version, fileStamp, mimeType, size)
        {
            Id = id;
        }
    }
}
