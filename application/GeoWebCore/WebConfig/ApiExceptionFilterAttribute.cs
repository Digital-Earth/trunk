using GeoWebCore.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Http.Filters;
using System.Net.Http;
using System.Net;
using System.Web.Http;
using System.Diagnostics;

namespace GeoWebCore.WebConfig
{
    class ApiExceptionFilterAttribute : ExceptionFilterAttribute
    {
        public override void OnException(HttpActionExecutedContext actionExecutedContext)
        {
            //check if we have valid exception
            if (actionExecutedContext != null && actionExecutedContext.Exception != null)
            {
                var reportException = true;

                if (actionExecutedContext.Exception is ApiException)
                {
                    //the current exception is ApiException - we can use it to generate custom error page
                    var apiException = actionExecutedContext.Exception as ApiException;

                    CreateCustomErrorResponse(actionExecutedContext, apiException);                
                }
                else if (actionExecutedContext.Exception is HttpResponseException)
                {
                    var response = (actionExecutedContext.Exception as HttpResponseException).Response;

                    if (response != null)
                    {
                        //401..404 are errors we don't need to be notified about. at least not for now.
                        switch (response.StatusCode)
                        {
                            case HttpStatusCode.Unauthorized:
                            case HttpStatusCode.PaymentRequired:
                            case HttpStatusCode.Forbidden:
                            case HttpStatusCode.NotFound:
                                reportException = false;
                                break;
                        }
                    }
                }
                //to deal with generic exception (not ApiException or HttpResponseException exceptions)
                else
                {
                    var innerExpection = actionExecutedContext.Exception;
                    if (innerExpection != null && actionExecutedContext.Exception is AggregateException)
                    {
                        var aggragateException = innerExpection as AggregateException;
                        if (aggragateException.InnerExceptions.Count == 1)
                        {
                            innerExpection = aggragateException.InnerExceptions[0];
                        }
                    }

                    //create a ServerError ApiException
                    var genericErrorMessage = new ApiException(
                        new FailureResponse(
                            FailureResponse.ErrorCodes.ServerError,
                            innerExpection.Message)
                        , innerExpection);

                    CreateCustomErrorResponse(actionExecutedContext, genericErrorMessage);
                }

                //if we have RavenClient installed - try to send this exception to the server.
                if (reportException && !actionExecutedContext.Request.IsLocal() && Program.RavenClient != null)
                {
                    var data = new SharpRaven.Data.SentryEvent(actionExecutedContext.Exception);
                    data.Extra = actionExecutedContext.ActionContext.ActionArguments;

                    //add request info
                    data.Tags["url"] = actionExecutedContext.Request.RequestUri.ToString();
                    data.Tags["method"] = actionExecutedContext.Request.Method.ToString();
                    data.Tags["exception"] = actionExecutedContext.Exception.GetType().Name;

                    //add geoSource if found
                    if (actionExecutedContext.ActionContext.ActionArguments.ContainsKey("geoSource"))
                    {
                        data.Tags["geoSource"] = actionExecutedContext.ActionContext.ActionArguments["geoSource"].ToString();
                    }

                    Program.RavenClient.CaptureAsync(data);
                }
            }
        }

        /// <summary>
        /// Create A CustomErrorResponse from ApiException
        /// </summary>
        /// <param name="actionExecutedContext">HttpActionExecutedContext context.</param>
        /// <param name="apiException">ApiException with error details.</param>
        private static void CreateCustomErrorResponse(HttpActionExecutedContext actionExecutedContext, ApiException apiException)
        {
            var httpStatusCode = ExtractHttpStatusCode(apiException);

            if (actionExecutedContext.Request.IsLocal() && Debugger.IsAttached)
            {
                //collect all exception strack trace and add it for local debuging
                string completeStackTrace = "";

                var exception = apiException as Exception;

                do 
                {
                    //add the current text at the top of the stack strace.
                    completeStackTrace = String.Format("<<{0} : {1}>>\n{2}\n{3}", exception.GetType().Name, exception.Message , exception.StackTrace, completeStackTrace);
                    //get next inner exception until we done    
                    exception = exception.InnerException;
                }
                while(exception != null);

                apiException.Failure.StackTrace = completeStackTrace;
            }

            var response = actionExecutedContext.Request.CreateResponse(httpStatusCode, apiException.Failure);

            response.Headers.CacheControl = new System.Net.Http.Headers.CacheControlHeaderValue
            {
                NoCache = true,
                NoStore = true
            };

            actionExecutedContext.Response = response;
        }

        /// <summary>
        /// Return a matching HttpStatusCode based oon ApiException error codes
        /// </summary>
        /// <param name="apiException">ApiException to use</param>
        /// <returns>HttpStatusCode matchin gthe error: 400,401,404 and 500</returns>
        private static HttpStatusCode ExtractHttpStatusCode(ApiException apiException)
        {
            var httpStatusCode = HttpStatusCode.BadRequest; //400
            if (apiException.Failure.ErrorCode >= FailureResponse.ErrorCodes.ServerError)
            {
                httpStatusCode = HttpStatusCode.InternalServerError; //500
            }
            else if (apiException.Failure.ErrorCode >= FailureResponse.ErrorCodes.Unauthorized)
            {
                httpStatusCode = HttpStatusCode.Unauthorized; //401
            }
            else if (apiException.Failure.ErrorCode >= FailureResponse.ErrorCodes.NotFound)
            {
                httpStatusCode = HttpStatusCode.NotFound; //404
            }
            return httpStatusCode;
        }
    }
}
