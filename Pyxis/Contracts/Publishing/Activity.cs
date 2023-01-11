using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public enum ActivityType
    {
        Rating,
        Comment,
        Agreement
    }

    public class Activity
    {
        public Guid Id;
        [JsonConverter(typeof(StringEnumConverter))]
        public ActivityType Type { get; set; }
        public DateTime Created { get; set; }
        public UserInfo User { get; set; }

        // for deserializing from string
        public Activity()
        {
        }

        public Activity(ActivityType type, UserInfo user)
        {
            Id = Guid.NewGuid();
            Type = type;
            User = user;
            Created = DateTime.UtcNow;
        }

        public Activity(Activity activity)
        {
            Id = activity.Id;
            Type = activity.Type;
            User = activity.User;
            Created = DateTime.UtcNow;
        }
    }

    public class ResourceActivity : Activity
    {
        public Guid ResourceId { get; set; }

        // for deserializing from string
        public ResourceActivity()
        {
        }

        public ResourceActivity(Guid resourceId, ActivityType type, UserInfo user)
            : base(type, user)
        {
            ResourceId = resourceId;
        }

        public ResourceActivity(ResourceActivity resourceActivity)
            : base(resourceActivity)
        {
            ResourceId = resourceActivity.ResourceId;
        }
    }
}