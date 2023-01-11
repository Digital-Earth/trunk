/******************************************************************************
PipelineMetadata.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// <see cref="Pipeline"/>s may or may not have Metadata.  A pipeline can 
    /// have many metadata name/value pairs.
    /// </summary>
    public class PipelineMetadata
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.  
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }

        public virtual string Name { get; set; }

        public virtual string Value { get; set; }        
    }
}
