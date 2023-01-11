/******************************************************************************
ErrorDeserializingGeometryException.cs

begin		: October 20, 2010
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Exceptions
{
    /// <summary>
    /// The exception to throw when the deserialization of a pipeline's geometry fails.
    /// </summary>
    public class ErrorDeserializingGeometryException : BaseException
    {
        public ErrorDeserializingGeometryException(string errorMessage) :
            base(errorMessage)
        {
        }
    }
}
