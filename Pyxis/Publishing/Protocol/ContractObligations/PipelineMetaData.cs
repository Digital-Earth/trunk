/******************************************************************************
PipelineMetaData.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Services.LicenseService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    internal class PipelineMetaData : IPipelineMetaData
    {
        public string ProcRef { get; set; }

        public string Definition { get; set; }
    }
}
