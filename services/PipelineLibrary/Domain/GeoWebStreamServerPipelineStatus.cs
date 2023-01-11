/******************************************************************************
GeoWebStreamServerPipelineStatus.cs

begin		: January 13, 2011
copyright	: (C) 211 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
//using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;

using Iesi.Collections.Generic;

using NHibernate;
using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// 
    /// </summary>
    public class GeoWebStreamServerPipelineStatus
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.  
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }

        public virtual string PipelineDefinition { get; set; }

        public virtual int PipelineId { get; set; }

        [System.ComponentModel.DefaultValue(false)]
        public virtual bool IsImported { get; set; }

        [System.ComponentModel.DefaultValue(false)]
        public virtual bool IsDownloaded { get; set; }

        [System.ComponentModel.DefaultValue(false)]
        public virtual bool IsPublished { get; set; }

        [System.ComponentModel.DefaultValue(false)]
        public virtual bool IsProcessed { get; set; }

        [System.ComponentModel.DefaultValue(0)]
        public virtual int MaxProcessedResolution { get; set; }

        public override string ToString()
        {
            return PipelineId +
                ((IsImported) ? ",Is Imported" : "") +
                ((IsDownloaded) ? ",Is Downloaded" : "") +
                ((IsPublished) ? ",Is Published" : "") +
                ((IsProcessed) ? ",Is Processed" : "Processed to Resolution" + MaxProcessedResolution);
        }
    }
}
