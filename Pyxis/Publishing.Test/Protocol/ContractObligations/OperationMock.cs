/******************************************************************************
OperationMock.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using Pyxis.Contract.Operations;

namespace Pyxis.Publishing.Test.Protocol
{
    internal class OperationMock : IOperation
    {
        public OperationType OperationType
        {
            get
            {
                return OperationType.Publish;
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        public Dictionary<string, string> Parameters
        {
            get
            {
                var param = new Dictionary<string, string>();
                param.Add("ProcRef", "ProcRef");
                return param;
            }
            set
            {
                throw new NotImplementedException();
            }
        }
    }
}
