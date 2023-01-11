/******************************************************************************
PipelineIdentity.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
//using System.Collections.Generic;
using System.Text;

using Iesi.Collections.Generic;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// All <see cref="Pipeline"/>s have a PipelineIdentity once it is 
    /// calculated.  Many pipelines can have the same identity.
    /// </summary>
    public class PipelineIdentity
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.  
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }

        /// <summary>
        /// The calculated XML identity.
        /// </summary>
        public virtual string XmlIdentity { get; set; }

        /// <summary>
        /// Indicates whether a pipeline's identity has reached its final state or not, i.e. 
        /// whether all processes that make up the pipeline have had their individual 
        /// checksums/identities calculated.
        /// </summary>
        [System.ComponentModel.DefaultValue(false)]
        public virtual bool IsStable { get; set; }

        // TODO[kabiraman]: Need to consider the performance issue of having 
        // pipelines refer to an identity and the identity in turn refering to 
        // pipelines; this would cause all pipelines of the same identity to 
        // be brought into memory.
        /// <summary>
        /// The pipelines that have this identity.
        /// </summary>
        protected internal virtual ISet<Pipeline> Pipelines { get; private set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="PipelineIdentity"/> class.
        /// </summary>
        public PipelineIdentity()
        {
            Pipelines = new HashedSet<Pipeline>();
        }
    }
}
