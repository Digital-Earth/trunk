using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public enum ResourceType
    {
        GeoSource,
        User,
        Group,
        License,
        Atlas,
        Gallery,
        Map,
        File,
        Pipeline,
        Product,
        Theme = 10 // specify number stops it from being optimized away as unused 
    }

    public class ResourceGrouping
    {
        public string Name { get; set; }
        public int Count { get; set; }
    }

    public class VersionedId
    {
        public Guid Id { get; set; }
        public Guid Version { get; set; }

        public static VersionedId FromResource(Resource resource)
        {
            return new VersionedId()
            {
                Id = resource.Id,
                Version = resource.Version
            };
        }

        public bool Equals(VersionedId other)
        {
            return Id == other.Id &&
                Version == other.Version;
        }

        public override bool Equals(object obj)
        {
            if (obj is VersionedId)
            {
                var other = obj as VersionedId;
                return Equals(other);
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return Id.GetHashCode() ^ Version.GetHashCode();
        }

        public static bool operator ==(VersionedId a, VersionedId b)
        {
            if (a == null)
            {
                return b == null;
            }
            return a.Equals(b);
        }

        public static bool operator !=(VersionedId a, VersionedId b)
        {
            return !(a == b);
        }

        public override string ToString()
        {
            return String.Format("{1}:{2}", Id, Version);
        }
    }

    public class ResourceReference
    {
        public Guid Id { get; set; }
        public Guid Version { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public ResourceType Type { get; set; }

        public ResourceReference Clone()
        {
            return new ResourceReference()
            {
                Type = Type,
                Id = Id,
                Version = Version
            };
        }

        public static ResourceReference FromResource(Resource resource)
        {
            return new ResourceReference()
            {
                Type = resource.Type,
                Id = resource.Id,
                Version = resource.Version
            };
        }

        public bool Equals(ResourceReference other)
        {
            return Type == other.Type &&
                Id == other.Id &&
                Version == other.Version;
        }

        public override bool Equals(object obj)
        {
            if (obj is ResourceReference)
            {
                var other = obj as ResourceReference;
                return Equals(other);
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return Id.GetHashCode() ^ Version.GetHashCode() ^ Type.GetHashCode();
        }

        public static bool operator ==(ResourceReference a, ResourceReference b)
        {
            if (ReferenceEquals(a, b))
            {
                return true;
            }
            if ((object)a == null || (object)b == null)
            {
                return false;
            }
            return a.Equals(b);
        }

        public static bool operator !=(ResourceReference a, ResourceReference b)
        {
            return !(a == b);
        }

        public override string ToString()
        {
            return String.Format("{0}:{1}:{2}", Type, Id, Version);
        }
    }

    public class Resource
    {
        public Guid Id { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public ResourceType Type { get; set; }
        public List<LicenseReference> Licenses { get; set; }
        public Metadata Metadata { get; set; }
        public Guid Version { get; set; }

        // for deserializing from string
        public Resource()
        {
        }

        public Resource(ResourceType type, List<LicenseReference> licenses, Metadata metadata, Guid version)
        {
            Id = Guid.NewGuid();
            Type = type;
            Licenses = new List<LicenseReference>(licenses);
            Metadata = metadata;
            Version = version;
        }

        // Generates a new Version
        public Resource(Resource basedOnResource)
        {
            Id = basedOnResource.Id;
            Type = basedOnResource.Type;
            Licenses = new List<LicenseReference>(basedOnResource.Licenses);
            Metadata = new Metadata(basedOnResource);
            Version = Guid.NewGuid();
        }

        // For unit testing
        public Resource(Guid id, ResourceType type, List<LicenseReference> licenses, Metadata metadata, Guid version)
        {
            Id = id;
            Type = type;
            Licenses = new List<LicenseReference>(licenses);
            Metadata = metadata;
            Version = version;
        }
    }
}