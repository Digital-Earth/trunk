using System;

namespace Pyxis.Contract.Publishing
{
    public class Rating : ResourceActivity
    {
        public sbyte Value { get; set; }

        // for deserializing from string
        public Rating()
        {
        }

        public Rating(Guid resourceId, UserInfo user, sbyte value)
            : base(resourceId, ActivityType.Rating, user)
        {
            Value = value;
        }

        public Rating(Rating rating)
            : base(rating)
        {
            Value = rating.Value;
        }
    }

    public class AggregateRatings
    {
        public int Likes { get; set; }
        public int Dislikes { get; set; }
        
        // for deserializing from string
        public AggregateRatings()
        {
        }

        public AggregateRatings(AggregateRatings ratings)
        {
            Likes = ratings.Likes;
            Dislikes = ratings.Dislikes;
        }
    }
}