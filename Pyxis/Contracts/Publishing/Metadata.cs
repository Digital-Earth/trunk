using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public class SimpleMetadata
    {
        public string Name { get; set; }
        public string Description { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> Tags { get; set; }
        
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> SystemTags { get; set; }
        
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<ExternalUrl> ExternalUrls { get; set; }

        public SimpleMetadata()
        {
        }

        public SimpleMetadata(SimpleMetadata other)
        {
            Name = other.Name;
            Description = other.Description;

            if (other.Tags != null)
            {
                Tags = new List<string>(other.Tags);
            }

            if (other.SystemTags != null)
            {
                SystemTags = new List<string>(other.SystemTags);
            }

            if (other.ExternalUrls != null)
            {
                ExternalUrls = other.ExternalUrls.Select(x => new ExternalUrl() { Type = x.Type, Url = x.Url }).ToList();
            }
        }
    }

    public class ImmutableMetadata : SimpleMetadata
    {
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public UserInfo User { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<Provider> Providers { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Category { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore, DefaultValueHandling = DefaultValueHandling.Ignore)]
        public DateTime Created { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore, DefaultValueHandling = DefaultValueHandling.Ignore)]
        public DateTime Updated { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Guid? BasedOnVersion { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public VisibilityType? Visibility { get; set; }
    }

    public class Metadata : ImmutableMetadata
    {
        public LinkedList<AggregateComment> Comments { get; set; }
        public AggregateRatings Ratings { get; set; }
        
        // for deserializing from string
        public Metadata()
        {
        }

        public Metadata(string name, string description, UserInfo user, List<Provider> providers, string category, List<string> tags, List<string> systemTags,
            DateTime created, DateTime updated, Guid? basedOnVersion, List<ExternalUrl> externalUrls, VisibilityType? visibility, LinkedList<AggregateComment> comments, AggregateRatings ratings)
        {
            Name = name;
            Description = description;
            User = user;
            Providers = new List<Provider>(providers ?? new List<Provider>());
            Category = category;
            Tags = new List<string>(tags ?? new List<string>());
            SystemTags = new List<string>(systemTags ?? new List<string>());
            Created = created;
            Updated = updated;
            BasedOnVersion = basedOnVersion;
            ExternalUrls = (externalUrls ?? new List<ExternalUrl>()).Select(x => new ExternalUrl { Type = x.Type, Url = x.Url }).ToList(); ;
            Visibility = visibility;
            Comments = new LinkedList<AggregateComment>(comments ?? new LinkedList<AggregateComment>());
            Ratings = new AggregateRatings(ratings ?? new AggregateRatings());
        }

        public Metadata(Metadata metadata)
            : this(metadata.Name, metadata.Description, metadata.User, metadata.Providers, metadata.Category, metadata.Tags, metadata.SystemTags,
                metadata.Created, metadata.Updated, metadata.BasedOnVersion, metadata.ExternalUrls, metadata.Visibility, metadata.Comments, metadata.Ratings)
        {
        }
        
        // Generate a copy with new timestamps and BasedOnVersion set to the BasedOnResource's Version
        public Metadata(Resource BasedOnResource)
        {
            Name = BasedOnResource.Metadata.Name;
            Description = BasedOnResource.Metadata.Description;
            User = BasedOnResource.Metadata.User;
            Providers = new List<Provider>(BasedOnResource.Metadata.Providers ?? new List<Provider>());
            Category = BasedOnResource.Metadata.Category;
            Tags = new List<string>(BasedOnResource.Metadata.Tags ?? new List<string>());
            SystemTags = new List<string>(BasedOnResource.Metadata.SystemTags ?? new List<string>() );
            Created = BasedOnResource.Metadata.Created;
            Updated = DateTime.UtcNow;
            BasedOnVersion = BasedOnResource.Version;
            ExternalUrls = (BasedOnResource.Metadata.ExternalUrls ?? new List<ExternalUrl>() ).Select(x => new ExternalUrl { Type = x.Type, Url = x.Url }).ToList();
            Visibility = BasedOnResource.Metadata.Visibility;
            Comments = new LinkedList<AggregateComment>(BasedOnResource.Metadata.Comments ?? new LinkedList<AggregateComment>());
            Ratings = new AggregateRatings(BasedOnResource.Metadata.Ratings ?? new AggregateRatings() );
        }
    }

    public class Provider
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public ResourceType Type { get; set; }
        public Guid Id { get; set; }
        public string Name { get; set; }
    }

    public enum ExternalUrlType
    {
        Reference,
        Icon,
        Image,
        Video
    }

    public class ExternalUrl
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public ExternalUrlType Type { get; set; }
        public string Url { get; set; }
    }

    public class UserInfo
    {
        public Guid Id { get; set; }
        public string Name { get; set; }

        public UserInfo()
        {
        }

        public UserInfo(Guid id, string name)
        {
            Id = id;
            Name = name;
        }

        public UserInfo(User user)
        {
            Id = user.Id;
            Name = user.Metadata.Name;
        }

        public UserInfo(UserInfo userInfo)
        {
            Id = userInfo.Id;
            Name = userInfo.Name;
        }
    }

    public enum VisibilityType
    {
        Public,
        Private,
        NonDiscoverable
    }
}