using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class CrawlController : ApiController
    {
        public List<CrawlingTask> Get()
        {
            return CrawlingTaskDb.Tasks;
        }
        
        public async Task<string> Post()
        {
            var uri = new Uri(await this.Request.Content.ReadAsStringAsync());

            CrawlingTaskDb.StartTask(uri);

            return "Ok";
        }
    }
}
