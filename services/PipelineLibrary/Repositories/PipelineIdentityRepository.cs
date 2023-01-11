/******************************************************************************
PipelineIdentityRepository.cs

begin		: September 30, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate;

namespace Pyxis.Services.PipelineLibrary.Repositories
{
    /// <summary>
    /// 
    /// </summary>
    internal class PipelineIdentityRepository 
    {
        internal void Add(
            Domain.PipelineIdentity pipelineIdentity, 
            ISession session)
        {
            session.Save(pipelineIdentity);
        }

        internal void Update(
            Domain.PipelineIdentity pipelineIdentity, 
            ISession session)
        {
            session.Update(pipelineIdentity);
        }

        internal Domain.PipelineIdentity GetByIdentity(
            string identity, 
            ISession session)
        {
            Domain.PipelineIdentity pipelineIdentity = session
                .CreateCriteria(typeof(Pyxis.Services.PipelineLibrary.Domain.PipelineIdentity))
                    .Add(NHibernate.Criterion.Expression.Eq("XmlIdentity", identity))
                    .UniqueResult<Domain.PipelineIdentity>();

            return pipelineIdentity;
        }
    }
}
