using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LicenseServer.DTOs;

namespace LicenseServer.Models.Mongo.Interface
{
    public interface IPipelines
    {
        IQueryable<PipelineTaggedInfoNoDefinitionDTO> GetPipelines();
        PipelineTaggedInfoDTO GetPipeline(string procRef);
        PublishedPipelineDetailsDTO GetPipelineDetails(string procRef);
    }
}
