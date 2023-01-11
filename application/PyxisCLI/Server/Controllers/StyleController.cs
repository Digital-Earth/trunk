using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using Pyxis.Contract.Publishing;
using PyxisCLI.Server.Cache;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    [RoutePrefix("api/v1/Style")]
    public class StyleController : ApiController
    {
        /// <summary>
        /// post a style and get back a key (hash) representing that geometry so it can be used later on for other requests
        /// </summary>
        /// <param name="style">Style to generate has for</param>
        /// <returns>key - hash string value</returns>
        [HttpPost]
        [Route("")]
        [TimeTrace()]
        public string Post(Style style)
        {
            return StyleCacheSingleton.Add(style);
        }
    }
}
