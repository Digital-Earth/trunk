using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Pyxis.Contract.Diagnostics
{
    public class Feedback
    {
        public string Name { get; set; }
        public string UserId { get; set; }
        public string Email { get; set; }
        public string Url { get; set; }
        public string Body { get; set; }

        public string Subject { get; set; }
        public List<FeedbackAttachment> Attachments { get; set; }
    }

    public class FeedbackAttachment
    {
        public string Name { get; set; }
        public string Base64Content { get; set; }
        public string MimeType { get; set; }
    }
}