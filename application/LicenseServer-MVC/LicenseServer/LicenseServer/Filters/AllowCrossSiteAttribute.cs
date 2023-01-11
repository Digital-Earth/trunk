/******************************************************************************
AllowCrossSiteAttribute.cs

begin		: Aug. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http.Filters;

namespace LicenseServer.Filters
{
    public class AllowCrossSiteAttribute : ActionFilterAttribute, IActionFilter
    {   
        public override void OnActionExecuted(HttpActionExecutedContext actionExecutedContext)
        {
            if (actionExecutedContext.Response != null)
            {
                actionExecutedContext.Response.Headers.Add("Access-Control-Allow-Origin", "*");
                actionExecutedContext.Response.Headers.Add("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                actionExecutedContext.Response.Headers.Add("Access-Control-Allow-Headers", "X-Requested-With, X-Request, Accept, Access-Control-Allow-Origin, Content-Type");
            }

            base.OnActionExecuted(actionExecutedContext);
        }
    }
}