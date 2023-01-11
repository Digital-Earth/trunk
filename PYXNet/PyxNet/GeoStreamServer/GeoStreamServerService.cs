/******************************************************************************
GeoStreamServerService.cs

begin      : November 1, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    /// <summary>
    /// Implementation of a GeoStreamServerService.
    /// </summary>
    public class GeoStreamServerService : Service.ServiceBase
    {
        #region ServiceIds

        /// <summary>
        /// The globally known identity of a logging service.
        /// </summary>
        public static readonly Service.ServiceId GeoStreamServerServiceId =
            new PyxNet.Service.ServiceId(
                new Guid("{78C4840E-141A-4a57-A693-917A8CCD17D3}"));

        #endregion ServiceIds

        #region Construction

        /// <summary>
        /// Constructor.  Creates a server attached to the given stack.
        /// This can block for a while.
        /// </summary>
        /// <param name="stack"></param>
        public GeoStreamServerService(Stack stack) :
            base(stack, GeoStreamServerServiceId)
        {
            if (null == stack)
            {
                throw new ArgumentNullException("stack");
            }

            Tracer.DebugWriteLine("Constructing on stack {0}.", stack.ToString());

            // Check to see if there is a valid certificate in the stack's 
            //  certificate repository.  If there is, then we are authorized,
            //  and the stack will automatically publish that certificate (so
            //  other nodes can find us.)
            Tracer.DebugWriteLine("Checking validity of certificate.");
            if (!Certificate.Valid)
            {
                InvalidOperationException exception = new InvalidOperationException(
                    "This node is not authorized to act as a logging server.");
                Tracer.WriteLine("The certificate is invalid.  Throwing exception: {0}.", exception.ToString());
                throw exception;
            }
            Tracer.DebugWriteLine("The certificate is valid.  Construction complete.");
        }

        #endregion Construction
    }
}