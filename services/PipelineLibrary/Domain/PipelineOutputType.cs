/******************************************************************************
PipelineOutput.cs

begin		: October 2, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// A <see cref="Pipeline"/> may or may not have OutputTypes.  A pipeline can 
    /// have many OutputTypes, for e.g. a coverage pipeline outputs a coverage 
    /// and a feature collection.
    /// </summary>
    public class PipelineOutputType
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.  
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }        

        /// <summary>
        /// Gets or sets the type of the output.
        /// </summary>
        public virtual Guid OutputType { get; set; }
    }
}
