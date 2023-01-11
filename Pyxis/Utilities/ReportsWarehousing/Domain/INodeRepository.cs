/******************************************************************************
INodeRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public interface INodeRepository
    {
        void Add(Node node);
        void Update(Node node);
        void Remove(Node node);

        Node GetById(int id);
        Node GetByGuid(Guid ident);

        ICollection<Node> GetAll();
    }
}
