using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace LicenseServer.Models.Mongo
{
    /// <summary>
    /// Allow specification of a format of resources to respond to a request with
    /// </summary>
    public enum ResultFormat
    {
        /// <summary>
        /// All Resource fields
        /// </summary>
        Full,
        /// <summary>
        /// Common fields only (Resource base class)
        /// </summary>
        Basic,
        /// <summary>
        /// Optimize for views (excludes Pipeline definitions)
        /// </summary>
        View
    }
}