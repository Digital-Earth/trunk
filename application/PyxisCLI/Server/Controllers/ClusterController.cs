using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Provide Information about the cluster
    /// </summary>
    [RoutePrefix("api/v1/Cluster")]
    public class ClusterController : ApiController
    {
        [Route("Health")]
        [HttpGet]
        public string Health()
        {
            return "Ok";
        }
    }
}
