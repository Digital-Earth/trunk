/******************************************************************************
PublishedPipelineDTO.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.App_Utilities;
using LicenseServer.Models;
using Pyxis.Contract.Publishing;

namespace LicenseServer.DTOs
{
    public class PublishedPipeline
    {
        public string ProcRef { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public long DataSize { get; set; }
    }

    public class PublishedPipelineSummaryDTO : PublishedPipeline
    {
        public int PublishingServers { get; set; }
        public Dictionary<string, int> Statuses { get; set; }
    }

    public class PublishedPipelineDetailsDTO : PublishedPipeline
    {
        public List<PipelineServerStatusDTO> PublishingServerStatus { get; set; }
    }
}