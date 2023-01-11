/******************************************************************************
DataTransferred.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;
using NHibernate;

using NUnit.Framework;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public class DataTransferred
    {
        public virtual int id
        { get; set; }

        public virtual Report Report
        { get; set; }

        public virtual DateTime TimeStamp
        { get; set; }

        public virtual Node SendingNode
        { get; set; }

        public virtual Node ReceivingNode
        { get; set; }

        public virtual DataSet DataSet
        { get; set; }

        public virtual int BytesTransferred
        { get; set; }

        public virtual int Chunks
        { get; set; }

    }
}
