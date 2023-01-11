/******************************************************************************
PipelineMockClient.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol;

namespace Pyxis.Publishing.Test
{
    internal class PipelineMockClient : IPipelineClient
    {
        public IPipelineMetaData GetPipelineMetaData(string procRef)
        {
            return new FakePipelineMetaData();
        }

        private class FakePipelineMetaData : IPipelineMetaData
        {
            public string ProcRef
            {
                get
                {
                    return "ProcRef";
                }
                set
                {
                    throw new NotImplementedException();
                }
            }

            public string Definition
            {
                get
                {
                    return "";//File.ReadAllText("FILE NAME TO READ DEFINITION FROM");
                }
                set
                {
                    throw new NotImplementedException();
                }
            }
        }
        public IList<IPipelineServerStatus> GetPipelineServerStatuses(string procRef)
        {
            throw new NotImplementedException();
        }


        public IEnumerable<IPipelineProcRef> GetAllPipelines()
        {
            throw new NotImplementedException();
        }

        public Contract.Publishing.Pipeline GetPipelineResource(string procRef)
        {
            throw new NotImplementedException();
        }

        public Contract.Publishing.Pipeline GetPipelineResource(Guid id, Guid version)
        {
            throw new NotImplementedException();
        }
    }
}
