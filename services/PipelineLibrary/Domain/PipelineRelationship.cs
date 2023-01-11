/******************************************************************************
PipelineRelationship.cs

begin		: October 6, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// Maintains child-parent relationship between <see cref="Pipeline"/>s.
    /// </summary>
    public class PipelineRelationship
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.  
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }

        /// <summary>
        /// The Parent pipeline.
        /// </summary>        
        protected internal virtual Pipeline Parent { get; set; }

        /// <summary>
        /// The database Id of the Pipeline that is the child of this Pipeline.
        /// </summary>
        public virtual int ChildPipelineId { get; set; }
    }
}
