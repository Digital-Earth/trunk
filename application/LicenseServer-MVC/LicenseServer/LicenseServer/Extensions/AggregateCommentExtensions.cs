using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Extensions
{
    public static class AggregateCommentExtensions
    {
        static public bool Insert(this LinkedList<AggregateComment> comments, Comment comment)
        {
            if (comments == null)
            {
                comments = new LinkedList<AggregateComment>();
            }

            if (comment.ReplyTo == null)
            {
                // newest comment at the top
                comments.AddFirst(new AggregateComment(comment));
            }
            else
            {
                AggregateComment foundComment = null;
                foreach (var existingComment in comments)
                {
                    foundComment = existingComment.FindComment(comment.ReplyTo.Value);
                    if (foundComment != null)
                    {
                        break;
                    }
                }
                if (foundComment == null)
                {
                    return false;
                }
                // newest reply at the end
                foundComment.Replies.AddLast(new AggregateComment(comment));
            }
            return true;
        }
    }
}