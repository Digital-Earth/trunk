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
    public interface IReportRepository
    {
        void Add(Report report);
        void Update(Report report);
        void Remove(Report report);

        Report Get(int id);

        ICollection<Report> Get(Node sendingNode, string reportName);
        ICollection<Report> GetAll();
    }
}
