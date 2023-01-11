/******************************************************************************
PipelineClient.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol.ContractObligations;
using Newtonsoft.Json;
using RestSharp;
using Pyxis.Publishing.Permits;
using Pyxis.Contract;

namespace Pyxis.Publishing.Protocol
{
    public class PipelineClient : RestPublishingClient, IPipelineClient
    {
        protected IPermitRetainer<AccessToken> TokenRetainer { get; set; }
        protected AccessToken Token { get { return TokenRetainer!=null?TokenRetainer.GetPermit():null; } }

        public PipelineClient(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public PipelineClient(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public PipelineClient(string licenseServerUrl, IPermitRetainer<AccessToken> tokenRetainer)
            : base(licenseServerUrl)
        {
            TokenRetainer = tokenRetainer;
        }

        public PipelineClient(string licenseServerUrl, string licenseServerRestPrefix, IPermitRetainer<AccessToken> tokenRetainer)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
            TokenRetainer = tokenRetainer;
        }

        public PipelineClient(PipelineClient basedOn, IPermitRetainer<AccessToken> tokenRetainer)
            : base(basedOn)
        {
            TokenRetainer = tokenRetainer;
        }

        public IPipelineMetaData GetPipelineMetaData(string procRef)
        {
            RestRequest request = new RestRequest(RestUrlBuilder.Pipeline(procRef), Method.GET);
            if (Token != null)
            {
                Token.Authenticate(request);
            }
            request.RequestFormat = DataFormat.Json;
            request.AddBody(procRef);
            var response = m_client.Execute<PipelineMetaData>(request);

            if (response.ResponseStatus == ResponseStatus.Completed)
            {
                return response.Data;
            }
            Debug.WriteLine("Failed to get pipeline meta data from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage);
            return null;
        }

        public IList<IPipelineServerStatus> GetPipelineServerStatuses(string procRef)
        {
            RestRequest request = new RestRequest(RestUrlBuilder.UserPipelineDetails(procRef), Method.GET);
            if (Token != null)
            {
                Token.Authenticate(request);
            }
            request.RequestFormat = DataFormat.Json;
            var response = m_client.Execute<PipelinePublishingServerStatusesDTO>(request);

            if (response.ResponseStatus == ResponseStatus.Completed)
            {
                var pipelineServerStatuses = new List<IPipelineServerStatus>();

                if (response.Data.PublishingServerStatus != null)
                {
                    pipelineServerStatuses.AddRange(response.Data.GetIPipelineServerStatuses());
                }
                return pipelineServerStatuses;
            }
            Debug.WriteLine("Failed to get pipeline server statuses from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage);
            return null;
        }

        public IEnumerable<IPipelineProcRef> GetAllPipelines()
        {
            RestRequest request = new RestRequest(RestUrlBuilder.AllPipelines(), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            var response = m_client.Execute<List<PipelineProcRef>>(request);
            if (response.ResponseStatus == ResponseStatus.Completed)
            {
                var result = new List<IPipelineProcRef>();
                result.AddRange(response.Data);
                return result;
            }
            Debug.WriteLine("Failed to get all pipelines from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage);
            return null;
        }

        public Pyxis.Contract.Publishing.Pipeline GetPipelineResource(string procRef)
        {
            RestRequest request = new RestRequest(RestUrlBuilder.GetPipelineResourceByProcRef(procRef), Method.GET);
            if (Token != null)
            {
                Token.Authenticate(request);
            }
            var response = m_client.Execute(request);
            var pipeline = DeserializePipeline(response.Content);

            if (response.StatusCode == HttpStatusCode.OK && pipeline != null)
            {
                return pipeline;
            }
            Debug.WriteLine("Failed to get pipeline Resource from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage);
            return null;
        }

        public Pyxis.Contract.Publishing.Pipeline GetPipelineResource(Guid id, Guid version)
        {
            RestRequest request = new RestRequest(RestUrlBuilder.GetPipelineResourceByIdAndVersion(id, version), Method.GET);
            if (Token != null)
            {
                Token.Authenticate(request);
            }
            var response = m_client.Execute(request);
            var pipeline = DeserializePipeline(response.Content);

            if (response.StatusCode == HttpStatusCode.OK && pipeline != null)
            {
                return pipeline;
            }
            Debug.WriteLine("Failed to get pipeline Resource from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage);
            return null;
        }

        private Pyxis.Contract.Publishing.Pipeline DeserializePipeline(string pipelineString)
        {
            var pipeline = JsonConvert.DeserializeObject<Pyxis.Contract.Publishing.Pipeline>(pipelineString);
            if (pipeline == null)
            {
                return null;
            }
            if (pipeline.Type == Contract.Publishing.ResourceType.GeoSource)
            {
                pipeline = JsonConvert.DeserializeObject<Pyxis.Contract.Publishing.GeoSource>(pipelineString);
            }
            else if (pipeline.Type == Contract.Publishing.ResourceType.Map)
            {
                pipeline = JsonConvert.DeserializeObject<Pyxis.Contract.Publishing.Map>(pipelineString);
            }
            return pipeline;
        }

        private class PipelinePublishingServerStatusesDTO
        {
            public PipelinePublishingServerStatusesDTO()
            {
                PublishingServerStatus = new List<PipelineServerStatusDTO>();
            }

            public List<PipelineServerStatusDTO> PublishingServerStatus { get; set; }

            public string ProcRef { get; set; }

            public string Name { get; set; }

            public string Description { get; set; }

            public long DataSize { get; set; }

            public List<IPipelineServerStatus> GetIPipelineServerStatuses()
            {
                var pipelineServerStatuses = new List<IPipelineServerStatus>();
                pipelineServerStatuses.AddRange(PublishingServerStatus.Select(x => x.GetIPublishingServerStatus()));
                return pipelineServerStatuses;
            }
        }

        // The following is to get around deserializing to interface objects
        private class PipelineServerStatusDTO
        {
            public string ProcRef { get; set; }

            public string NodeId { get; set; }

            public string Status { get; set; }

            public OperationStatusDTO OperationStatus { get; set; }

            public IPipelineServerStatus GetIPublishingServerStatus()
            {
                return new PipelineServerStatus()
                {
                    ProcRef = ProcRef,
                    NodeId = NodeId,
                    Status = Status,
                    OperationStatus = OperationStatus != null ? OperationStatus.GetIOperationStatus() : null
                };
            }
        }

        private class OperationStatusDTO
        {
            public OperationStatusDTO()
            {
                Operation = new Operation();
                Guid = System.Guid.NewGuid().ToString();
            }

            public Operation Operation { get; set; }

            public string Description { get; set; }

            public DateTime EndTime { get; set; }

            public string Guid { get; set; }

            public float? Progress { get; set; }

            public DateTime StartTime { get; set; }

            public OperationStatusCode StatusCode { get; set; }

            public IOperationStatus GetIOperationStatus()
            {
                return new OperationStatus()
                {
                    Description = Description,
                    EndTime = EndTime,
                    Guid = Guid,
                    Progress = Progress,
                    StartTime = StartTime,
                    StatusCode = StatusCode,
                    Operation = Operation
                };
            }
        }

        private class PipelineProcRef : IPipelineProcRef
        {

            public string ProcRef { get; set; }

            public IPipelineProcRef GetIPipelineProcRef()
            {
                return this;
            }
        }
    }

    public class AuthenticatedPipelineClient : PipelineClient
    {
        public AuthenticatedPipelineClient(string licenseServerUrl, IPermitRetainer<AccessToken> tokenRetainer)
            : base(licenseServerUrl, tokenRetainer)
        {            
        }

        public AuthenticatedPipelineClient(string licenseServerUrl, string licenseServerRestPrefix, IPermitRetainer<AccessToken> tokenRetainer)
            : base(licenseServerUrl, licenseServerRestPrefix, tokenRetainer)
        {   
        }

        public T PostResource<T>(T resource) where T : Pyxis.Contract.Publishing.Resource
        {
            var request = new RestRequest(RestUrlBuilder.PostResource<T>(), Method.POST);
            var requestBody = JsonConvert.SerializeObject(resource);
            request.AddParameter("application/json", requestBody, ParameterType.RequestBody);
            request.RequestFormat = DataFormat.Json;
            Token.Authenticate(request);
            var response = m_client.Execute(request);

            if (response.StatusCode != HttpStatusCode.Created)
            {
                throw new Exception(String.IsNullOrEmpty(response.ErrorMessage) ? response.Content : response.ErrorMessage);
            }

            return JsonConvert.DeserializeObject<T>(response.Content);
        }

        public void PutResource<T>(Guid id, Guid version, T resourceUpdates) where T : Pyxis.Contract.Publishing.Resource
        {
            var request = new RestRequest(RestUrlBuilder.PutResource<T>(id, version), Method.PUT);
            var requestBody = JsonConvert.SerializeObject(resourceUpdates);
            request.AddParameter("application/json", requestBody, ParameterType.RequestBody);
            request.RequestFormat = DataFormat.Json;
            Token.Authenticate(request);
            var response = m_client.Execute(request);

            if (response.StatusCode != HttpStatusCode.OK)
            {
                throw new Exception(String.IsNullOrEmpty(response.ErrorMessage) ? response.Content : response.ErrorMessage);
            }
        }

        public void DeleteResource<T>(Guid id) where T : Pyxis.Contract.Publishing.Resource
        {
            var request = new RestRequest(RestUrlBuilder.GetResourceById<T>(id), Method.DELETE);
            Token.Authenticate(request);
            var response = m_client.Execute(request);

            if (response.StatusCode != HttpStatusCode.OK)
            {
                throw new Exception(String.IsNullOrEmpty(response.ErrorMessage) ? response.Content : response.ErrorMessage);
            }
        }
    }
}
