using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Publishing
{
    public class Comment : ResourceActivity
    {
        public Guid? ReplyTo { get; set; }
        public string Message { get; set; }

        // for deserializing from string
        public Comment()
        {
        }

        public Comment(Guid resourceId, string message, UserInfo user, Guid? replyTo)
            :base(resourceId, ActivityType.Comment, user)
        {
            ReplyTo = replyTo;
            Message = message;
        }

        public Comment(Comment comment)
            : base(comment)
        {
            ReplyTo = comment.ReplyTo;
            Message = comment.Message;
        }
    }

    public class IndividualComment
    {
        public Guid ResourceId { get; set; }
        public Guid? ReplyTo { get; set; }
        public string Message { get; set; }
        public Guid Id;
        public DateTime Created { get; set; }
        public UserInfo User { get; set; }

        // for deserializing from string
        public IndividualComment()
        {
        }

        public IndividualComment(Comment comment)
        {
            ResourceId = comment.ResourceId;
            ReplyTo = comment.ReplyTo;
            Message = comment.Message;
            Id = comment.Id;
            Created = comment.Created;
            User = new UserInfo(comment.User);
        }

        public IndividualComment(IndividualComment comment)
        {
            ResourceId = comment.ResourceId;
            ReplyTo = comment.ReplyTo;
            Message = comment.Message;
            Id = comment.Id;
            Created = comment.Created;
            User = new UserInfo(comment.User);
        }
    }

    public class AggregateComment
    {
        public IndividualComment Comment { get; set; }
        public LinkedList<AggregateComment> Replies { get; set; }

        // for deserializing from string
        public AggregateComment()
        {
        }

        public AggregateComment(Comment comment)
        {
            Comment = new IndividualComment(comment);
            Replies = new LinkedList<AggregateComment>();
        }

        public AggregateComment(AggregateComment aggregateComment)
        {
            Comment = new IndividualComment(aggregateComment.Comment);
            Replies = new LinkedList<AggregateComment>(Replies);
        }

        public AggregateComment FindComment(Guid id)
        {
            AggregateComment comment = null;
            if (id == Comment.Id)
            {
                comment = this;
            }
            else
            {
                foreach (var currentComment in Replies)
                {
                    var foundComment = currentComment.FindComment(id);
                    if (foundComment != null)
                    {
                        return foundComment;
                    }
                }
            }
            return comment;
        }
    }
}