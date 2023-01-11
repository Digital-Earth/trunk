/******************************************************************************
LsStatus.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.LicenseService;

namespace Pyxis.Contract.Publishing
{
    public class LsStatus : ILsStatus
    {
        public IList<IOperation> PipelinesRequests { get { return m_pipelinesRequests as IList<IOperation>; } set { m_pipelinesRequests = value as List<IOperation>; } }
        private List<IOperation> m_pipelinesRequests = new List<IOperation>();
    }

    public class Operation : IOperation
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public OperationType OperationType { get; set; }

        public Dictionary<string, string> Parameters { get; set; }
    }
}