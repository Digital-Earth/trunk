using System;
using System.Linq;
using System.Net.Http;
using System.Web.Http.Filters;

namespace PyxisCLI.Server.WebConfig
{
    [AttributeUsage(AttributeTargets.Method,AllowMultiple=false)]
    class TimeTraceAttribute : ActionFilterAttribute
    {
        private static readonly string s_propName = "TimeTrace.Timestamp";

        public string Format { get; set; }

        public TimeTraceAttribute()
        {
            //some default attributes
            Format = "id,geoSource,key";
        }

        public TimeTraceAttribute(string format)
        {
            Format = format;
        }

        public override void OnActionExecuting(System.Web.Http.Controllers.HttpActionContext actionContext)
        {
            base.OnActionExecuting(actionContext);

            actionContext.Request.Properties[s_propName] = DateTime.Now;
        }

        public override void OnActionExecuted(HttpActionExecutedContext actionExecutedContext)
        {
            base.OnActionExecuted(actionExecutedContext);

            var startTime = (DateTime)actionExecutedContext.Request.Properties[s_propName];

            var actionContext = actionExecutedContext.ActionContext;

            var query = actionExecutedContext.Request.GetQueryNameValuePairs().ToDictionary(x=>x.Key,x=>(object)x.Value);

            var line = String.Format("{0} : {1} : {2}/{3}", 
                startTime.ToString("hh:mm:ss"),
                actionExecutedContext.Request.Method,
                actionContext.ActionDescriptor.ControllerDescriptor.ControllerName,
                actionContext.ActionDescriptor.ActionName);

            foreach(var name in Format.Split(',')) {
                object value;
                if (actionContext.ActionArguments.TryGetValue(name,out value) ||
                    query.TryGetValue(name,out value) ||
                    actionContext.Request.Properties.TryGetValue(name, out value))
                {
                    line += String.Format(" : {0}={1}", name, value);
                }
            }

            var error = false;
            if (actionExecutedContext.Response != null)
            {
                line += String.Format(" : {0} : {1}[msec]", (int)actionExecutedContext.Response.StatusCode, (int)(DateTime.Now - startTime).TotalMilliseconds);
                error = !actionExecutedContext.Response.IsSuccessStatusCode;
            }
            else if (actionExecutedContext.Exception != null)
            {
                var exception = actionExecutedContext.Exception;
                if (exception is AggregateException)
                {
                    var aggregateException = (AggregateException)exception;

                    if (aggregateException.InnerExceptions.Count == 1)
                    {
                        exception = aggregateException.InnerExceptions[0];
                    }
                    else
                    {
                        foreach (var innerException in aggregateException.InnerExceptions)
                        {
                            Console.WriteLine("Error: {0}\n{1}", innerException.Message, innerException.StackTrace);
                        }
                    }
                }

                error = true;
                line += String.Format(" : {0} : {1}[msec]", exception.Message, (int)(DateTime.Now - startTime).TotalMilliseconds);
            }
            else
            {
                error = true;
                line += String.Format(" : {0} : {1}[msec]", "?", (int)(DateTime.Now - startTime).TotalMilliseconds);
            }

            if (error)
            {
                Console.ForegroundColor = ConsoleColor.Red;
            }
            Console.WriteLine(line);
            Console.ResetColor();
        }
    }
}
