using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using Pyxis.Contract.Workspaces;
using PyxisCLI.Server.Cluster;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.Workspaces;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// AuthorizedApiController is a base class to be used by Contorllers that need to protect access to workspaces.
    /// 
    /// AuthorizedApiController have [Authorize] attribute it on to block anonymous requiests. use [AllowAnonymous] attribute
    /// on specific actions to enable anonymous requests.
    /// 
    /// AuthorizedApiController define the following:
    /// 
    /// 1. Workspaces property that is already been authorized based on the ApiController.User.
    /// 
    /// 2. HttpClusterProxy property to forward request to other nodes in the cluster using the right authorization token for this request
    /// </summary>
    [Authorize]
    public class AuthorizedApiController : ApiController
    {
        private readonly object m_workspacesLock = new object();
        private ILocalWorkspaces m_workspaces;

        public ILocalWorkspaces Workspaces
        {
            get
            {
                lock (m_workspacesLock)
                {
                    if (m_workspaces == null)
                    {
                        m_workspaces = Program.Workspaces.AuthorizedAs(User, (message) =>
                        {
                            throw new HttpResponseException(HttpStatusCode.Unauthorized);
                        });
                    }
                }
                return m_workspaces;
            }
        }

        public HttpClusterProxy HttpClusterProxy
        {
            get
            {
                if (Request.Headers.Authorization != null)
                {
                    return new HttpClusterProxy(Program.Cluster, Request.Headers.Authorization.Parameter);
                }
                else
                {
                    return new HttpClusterProxy(Program.Cluster);
                }
            }
        }
    }
}
