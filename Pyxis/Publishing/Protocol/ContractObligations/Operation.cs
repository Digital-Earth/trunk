/******************************************************************************
Operation.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using Pyxis.Contract.Operations;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    public class Operation : IOperation
    {
        public Operation()
        {
            Parameters = new Dictionary<string, string>();
        }

        public OperationType OperationType { get; set; }

        public Dictionary<string, string> Parameters { get; set; }
    }
}
