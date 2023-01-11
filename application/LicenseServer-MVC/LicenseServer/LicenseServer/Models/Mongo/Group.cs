using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class Group : Pyxis.Contract.Publishing.Group, IMongoResource
    {
        // for deserializing from string
        public Group()
        {
        }

        public Group(List<LicenseReference> licenses, Metadata metadata, Guid version, List<UserInfo> members)
            : base(licenses, metadata, version, members)
        {
        }

        public Group(Group basedOnGroup)
            : base(basedOnGroup)
        {
        }

        public Group(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, List<UserInfo> members)
            : base(id, licenses, metadata, version, members)
        {
        }
    }
}