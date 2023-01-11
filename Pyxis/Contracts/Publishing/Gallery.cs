using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Publishing
{
    public class GalleryResource
    {
        public Guid ResourceId { get; set; }
        public bool Featured { get; set; }
    }

    public class Gallery : Resource
    {
        public List<GalleryResource> Resources { get; set; }
        public UserInfo Admin { get; set; }
        public List<GroupPermissionInfo> Groups { get; set; } 

        // for deserializing from string
        public Gallery()
        {
        }

        public Gallery(List<LicenseReference> licenses, Metadata metadata, Guid version, List<GalleryResource> resources, UserInfo admin, List<GroupPermissionInfo> groups)
            : base(ResourceType.Gallery, licenses, metadata, version)
        {
            Resources = resources;
            Admin = admin == null ? null : new UserInfo(admin);
            Groups = new List<GroupPermissionInfo>(groups);
        }

        public Gallery(Gallery basedOnGallery)
            : base(basedOnGallery)
        {
            Resources = basedOnGallery.Resources;
            Admin = basedOnGallery.Admin == null ? null : new UserInfo(basedOnGallery.Admin);
            Groups = new List<GroupPermissionInfo>(basedOnGallery.Groups);
        }

        public Gallery(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, List<GalleryResource> resources, UserInfo admin, List<GroupPermissionInfo> groups)
            : this(licenses, metadata, version, resources, admin, groups)
        {
            Id = id;
        }
    }
}