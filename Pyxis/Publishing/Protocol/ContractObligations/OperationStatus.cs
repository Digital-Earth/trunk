/******************************************************************************
OperationStatus.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using Pyxis.Contract.Operations;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    public class OperationStatus : IOperationStatus
    {
        public OperationStatus()
        {
            Operation = new Operation();
            Guid = System.Guid.NewGuid().ToString();
        }

        public IOperation Operation { get; set; }

        public string Description { get; set; }

        public DateTime EndTime { get; set; }

        public string Guid { get; set; }

        public float? Progress { get; set; }

        public DateTime StartTime { get; set; }

        public virtual OperationStatusCode StatusCode { get; set; }
    }
}
