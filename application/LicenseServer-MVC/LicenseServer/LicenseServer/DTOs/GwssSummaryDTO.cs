/******************************************************************************
GwssSummaryDTO.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace LicenseServer.DTOs
{
    public class GwssSummaryDTO
    {
        public GwssSummaryDTO()
        {
            PipelineStatuses = new List<PipelineServerStatusDTO>();
        }

        public string NodeID { get; set; }
        public string Name { get; set; }
        public double AvailableDiskSpaceMB { get; set; }
        public DateTime LastHeard { get; set; }
        public List<PipelineServerStatusDTO> PipelineStatuses { get; set; }
    }
}