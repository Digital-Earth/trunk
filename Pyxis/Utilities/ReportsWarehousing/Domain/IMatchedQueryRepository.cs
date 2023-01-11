/******************************************************************************
IMatchedQueryRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public interface IMatchedQueryRepository
    {
        void Add(MatchedQuery query);
        void Update(MatchedQuery query);
        void Remove(MatchedQuery query);

        MatchedQuery GetById(int id);

        ICollection<MatchedQuery> GetAll();
    }
}
