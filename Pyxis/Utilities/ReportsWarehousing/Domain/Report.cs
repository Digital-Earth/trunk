/******************************************************************************
Report.cs

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
    /// <summary>
    /// Each report is saved and normalized so that we can utilize cascading
    /// deletes to remove a report's data when necessary.
    /// </summary>
    public class Report
    {
        public virtual int id
        { get; set; }

        public virtual DateTime TimeStamp
        { get; set; }

        public virtual Node SendingNode
        { get; set; }

        public virtual string Name
        { get; set; }

        public virtual DateTime Processed
        { get; set; }

        #region support for cascading delete 

        //--
        //-- reverse links to nodes that reference the report id.
        //-- hibernate uses these routines to do the cascading delete
        //-- of a report and all its data.
        //--

        public virtual Iesi.Collections.Generic.ISet<MatchedQuery> MatchedQuery
        { get; set; }

        public virtual Iesi.Collections.Generic.ISet<ReceivedQuery> ReceivedQuery
        { get; set; }

        public virtual Iesi.Collections.Generic.ISet<DataTransferred> DataTransferred
        { get; set; }

        #endregion
    }
}
