using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class Gallery : Pyxis.Contract.Publishing.Gallery, IMongoResource
    {
        // for deserializing from string
        public Gallery()
        {
        }

        public Gallery(List<LicenseReference> licenses, Metadata metadata, Guid version, List<GalleryResource> resources, UserInfo admin, List<GroupPermissionInfo> groups)
            : base(licenses, metadata, version, resources, admin, groups)
        {
        }

        public Gallery(Gallery basedOnGallery)
            : base(basedOnGallery)
        {
        }

        public Gallery(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, List<GalleryResource> resources, UserInfo admin, List<GroupPermissionInfo> groups)
            : base(id, licenses, metadata, version, resources, admin, groups)
        {
        }
    }
}