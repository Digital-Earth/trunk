/******************************************************************************
BaseException.cs

begin		: October 7, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Exceptions
{
    /// <summary>
    /// Base exception for all exceptions thrown in the PipelineLibrary assembly.
    /// </summary>
    public abstract class BaseException : System.Exception
    {
        public BaseException(string errorMessage) :
            base(errorMessage)
        {
        }
    }
}
