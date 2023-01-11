using System.Collections.Generic;
using Newtonsoft.Json;

namespace PyxisCLI.Server.Models
{    
    /// <summary>
    /// Generic Failure Response   
    /// </summary>
    //TODO: consider moving this class to Pyxis.Contract
    public class FailureResponse
    {
        /// <summary>
        /// List of well known error codes.
        /// </summary>
        public enum ErrorCodes
        {
            /// <summary>
            /// Bad Request
            /// </summary>
            BadRequest   = 10000,

            /// <summary>
            /// Resource not found
            /// </summary>
            NotFound     = 20000,

            /// <summary>
            /// Unauthorized request
            /// </summary>
            Unauthorized = 30000,

            /// <summary>
            /// Internal server error
            /// </summary>
            ServerError  = 40000,
        }

        /// <summary>
        /// Creates a Failure Response with a given error code and description
        /// </summary>
        /// <param name="errorCode">ErrorCode to use.</param>
        /// <param name="description">Description of the error.</param>
        public FailureResponse(ErrorCodes errorCode, string description)
        {
            ErrorCode = errorCode;
            Description = description;
        }    

        /// <summary>
        /// Code to represent the error.
        /// </summary>
        public ErrorCodes ErrorCode { get; set; }

        /// <summary>
        /// Description of the error.
        /// </summary>
        public string Description { get; set; }

        /// <summary>
        /// Required Information that can help resolve the error
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Dictionary<string, RequiredInformationItem> RequiredInformation { get; set; }


        /// <summary>
        /// Stack trace for local requests only in debug mode
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string StackTrace { get; set; }

        /// <summary>
        /// Add a Required information to the failure response
        /// </summary>
        /// <param name="fieldName">Field name for the required information.</param>
        /// <param name="description">Description of how this information can resolve the error.</param>
        /// <param name="possibleValues">List of possible values that can used to resolve the error.</param>
        /// <returns>this FailureResponse object</returns>
        public FailureResponse AddRequiredInformation(string fieldName, string description, IEnumerable<object> possibleValues = null)
        {
            if (RequiredInformation == null)
            {
                RequiredInformation = new Dictionary<string, RequiredInformationItem>();
            }

            RequiredInformation[fieldName] = new RequiredInformationItem()
            {
                Description = description
            };

            if (possibleValues != null)
            {
                RequiredInformation[fieldName].PossibleValues = new List<object>(possibleValues);
            }
            return this;
        }

        /// <summary>
        /// Represent a required information item.
        /// </summary>
        public class RequiredInformationItem
        {
            /// <summary>
            /// Description of how this information can resolve the error.
            /// </summary>
            public string Description { get; set; }

            /// <summary>
            /// List of possible values that can used to resolve the error.
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public List<object> PossibleValues { get; set; }
        }
    }
}
