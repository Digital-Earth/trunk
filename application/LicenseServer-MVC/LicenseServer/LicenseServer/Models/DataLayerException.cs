using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;

namespace LicenseServer.Models
{
    public class DataLayerException : Exception
    {
        private readonly DataLayerExceptionType exceptionType;
        public DataLayerExceptionType ExceptionType { get { return exceptionType; } }

        public DataLayerException(string message, DataLayerExceptionType dataLayerExceptionType)
            : base(message)
        {
            exceptionType = dataLayerExceptionType;
        }

        public HttpResponseMessage ToHttpResponse(HttpRequestMessage request)
        {
            HttpResponseMessage response = null;
            switch(exceptionType)
            {
                case DataLayerExceptionType.BadRequest:
                    response = request.CreateResponse(HttpStatusCode.BadRequest, Message);
                    break;
                case DataLayerExceptionType.Conflict:
                    response = request.CreateResponse(HttpStatusCode.Conflict, Message);
                    break;
                case DataLayerExceptionType.InvalidUpdate:
                    response = request.CreateResponse(HttpStatusCode.Forbidden, Message);
                    break;
                case DataLayerExceptionType.NotFound:
                    response = request.CreateResponse(HttpStatusCode.NotFound, Message);
                    break;
                case DataLayerExceptionType.NotModified:
                    response = request.CreateResponse(HttpStatusCode.NotModified, Message);
                    break;
                default:
                    // HTTP internal server error code 500
                    throw this;
            }
            return response;
        }
    }

    public enum DataLayerExceptionType
    {
        BadRequest,
        Conflict,
        InvalidUpdate,
        NotFound,
        NotModified
    }
}