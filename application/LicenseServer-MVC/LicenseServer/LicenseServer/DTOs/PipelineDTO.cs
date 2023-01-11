/******************************************************************************
PipelineDTO.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models;
using Pyxis.Contract.Services.LicenseService;

namespace LicenseServer.DTOs
{
    public class PipelineDTO : IPipelineMetaData
    {
        public string ProcRef { get; set; }
        public string User { get; set; }
        public string Definition { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string Category { get; set; }
        public long? DataSize { get; set; }
        public string ImageUrl { get; set; }
        public List<string> Tags { get; set; }

        public Guid Guid { get; set; }
    }
    
    public class PublishedResponse : IPublishedResponse
    {
        public string Url { get; set; }
    }
}