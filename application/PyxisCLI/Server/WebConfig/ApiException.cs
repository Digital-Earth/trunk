using System;
using PyxisCLI.Server.Models;

namespace PyxisCLI.Server.WebConfig
{
    /// <summary>
    /// ApiException wrapper for FailureResponse
    /// </summary>
    public class ApiException : Exception
    {
        /// <summary>
        /// The FailureResponse details
        /// </summary>
        public FailureResponse Failure { get; set; }


        /// <summary>
        /// Create a simple ApiException with a default FailureResponse using ErrorCode and description.
        /// </summary>
        /// <param name="errorCode">FailureResponse.ErrorCodes error code to use</param>
        /// <param name="description">Description of the error</param>
        public ApiException(FailureResponse.ErrorCodes errorCode, string description)
            : this(new FailureResponse(errorCode, description))
        {
        }

        /// <summary>
        /// Create ApiException with a given failure response
        /// </summary>
        /// <param name="failure">FailureResponse to use</param>
        public ApiException(FailureResponse failure)
            : base(failure.Description)
        {
            Failure = failure;
        }

        /// <summary>
        /// Create ApiException with a given failure respone and attach innerException for local debugging
        /// </summary>
        /// <param name="failure">FailureResponse to use</param>
        /// <param name="innerException">Exception to pass through the error page is debug setting are enabled.</param>
        public ApiException(FailureResponse failure, Exception innerException)
            : base(failure.Description, innerException)
        {
            Failure = failure;
        }
    }
}
