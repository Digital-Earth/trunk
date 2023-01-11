/******************************************************************************
IDataTransferredRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public interface IDataTransferredRepository
    {
        void Add(DataTransferred query);
        void Update(DataTransferred query);
        void Remove(DataTransferred query);

        DataTransferred GetById(int id);

        ICollection<DataTransferred> GetAll();
    }
}
