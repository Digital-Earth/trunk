/******************************************************************************
IReportRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public interface IReceivedQueryRepository
    {
        void Add(ReceivedQuery query);
        void Update(ReceivedQuery query);
        void Remove(ReceivedQuery query);

        ReceivedQuery GetById(int id);

        ICollection<ReceivedQuery> GetAll();
    }
}
