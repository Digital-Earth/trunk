/******************************************************************************
IDataSetRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public interface IDataSetRepository
    {
        void Add(DataSet node);
        void Update(DataSet node);
        void Remove(DataSet node);

        DataSet GetById(int id);
        DataSet GetByGuid(Guid ident);

        ICollection<DataSet> GetAll();
    }
}
