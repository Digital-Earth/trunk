using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class User : Pyxis.Contract.Publishing.User, IMongoResource
    {
        // for deserializing from string
        public User()
        {
        }

        public User(List<LicenseReference> licenses, Metadata metadata, Guid version, UserState? state, UserType? userType, List<Guid> subscribed, bool? seller, List<Guid> galleries, List<GroupInfo> groups)
            : base(licenses, metadata, version, state, userType, subscribed, seller, galleries, groups)
        {
        }

        public User(User basedOnUser)
            : base(basedOnUser)
        {
        }

        public User(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, UserState? state, UserType? userType, List<Guid> subscribed, bool? seller, List<Guid> galleries, List<GroupInfo> groups)
            : base(id, licenses, metadata, version, state, userType, subscribed, seller, galleries, groups)
        {
        }
    }
}