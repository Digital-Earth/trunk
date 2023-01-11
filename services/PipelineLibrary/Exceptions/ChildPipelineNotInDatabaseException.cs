/******************************************************************************
ChildPipelineNotInDatabaseException.cs

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
    /// The exception to throw when a pipeline that does not exist in the 
    /// database is assigned as a child to a pipeline.
    /// </summary>
    public class ChildPipelineNotInDatabaseException : BaseException
    {
        public ChildPipelineNotInDatabaseException(string errorMessage) :
            base(errorMessage)
        {
        }
    }
}
