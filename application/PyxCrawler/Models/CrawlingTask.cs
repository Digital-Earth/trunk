using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace PyxCrawler.Models
{
    public class CrawlingTask
    {
        public Uri Uri { get; set; }
        public string Service { get; set; }
        public double Progress { get; set; }
        public string Status { get; set; }
    }
}