using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public enum UserState
    {
        Active,
        Banned,
        Expired
    }

    public enum UserType
    {
        Beta,
        Member,
        Super
    }

    public class User : Resource
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public UserState? State { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public UserType? UserType { get; set; }
        public List<Guid> Subscribed { get; set; }
        public bool? Seller { get; set; }
        public List<Guid> Galleries { get; set; }
        public List<GroupInfo> Groups { get; set; }

        // for deserializing from string
        public User()
        {
        }

        public User(List<LicenseReference> licenses, Metadata metadata, Guid version, UserState? state, UserType? userType, List<Guid> subscribed, bool? seller, List<Guid> galleries, List<GroupInfo> groups)
            : base(ResourceType.User, licenses, metadata, version)
        {
            State = state;
            UserType = userType;
            Subscribed = subscribed;
            Seller = seller;
            Galleries = galleries;
            Groups = groups;
        }

        public User(User basedOnUser)
            : base(basedOnUser)
        {
            State = basedOnUser.State;
            UserType = basedOnUser.UserType;
            Subscribed = basedOnUser.Subscribed;
            Seller = basedOnUser.Seller;
            Galleries = basedOnUser.Galleries;
            Groups = basedOnUser.Groups;
        }

        public User(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, UserState? state, UserType? userType, List<Guid> subscribed, bool? seller, List<Guid> galleries, List<GroupInfo> groups)
            : this(licenses, metadata, version, state, userType, subscribed, seller, galleries, groups)
        {
            Id = id;
        }
    }
}