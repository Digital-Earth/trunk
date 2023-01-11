using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using PyxCrawler.Models;

namespace PyxCrawler.Crawling
{
    public interface ICrawler
    {
        void Crawl(CrawlingTask task, OnlineGeospatialEndpoint endpoint);
    }
}