using System.Collections.Generic;
using AutoMapper;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using System.Linq;

namespace LicenseServer.DTOs
{
    public class PipelineTaggedInfoFactory
    {
        static PipelineTaggedInfoFactory()
        {
            Mapper.CreateMap<Pyxis.Contract.Publishing.GeoSource, PipelineTaggedInfoDTO>();
            Mapper.CreateMap<Pyxis.Contract.Publishing.Metadata, PipelineTaggedInfoDTO>().
                ForMember(x => x.User, opt => opt.MapFrom(m => m.User.Name));
        }

        static public PipelineTaggedInfoDTO Create(Pyxis.Contract.Publishing.GeoSource geoSource)
        {
            if (geoSource != null)
            {
                var pipelineTaggedInfoDTO = Mapper.Map<PipelineTaggedInfoDTO>(geoSource);
                Mapper.Map(geoSource.Metadata, pipelineTaggedInfoDTO, typeof(Pyxis.Contract.Publishing.Metadata), typeof(PipelineTaggedInfoDTO));
                var externalImageUrl = geoSource.Metadata.ExternalUrls.FirstOrDefault(x => x.Type == Pyxis.Contract.Publishing.ExternalUrlType.Image);
                pipelineTaggedInfoDTO.ImageUrl = externalImageUrl == null ? null : externalImageUrl.Url;
                return pipelineTaggedInfoDTO;
            }
            return null;
        }
    }

    public class PipelineInfoBase
    {
        public string ProcRef { get; set; }
        public string State { get; set; }
        public long? DataSize { get; set; }
        public string User { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string Category { get; set; }
        public string ImageUrl { get; set; }
        public System.DateTime? Created { get; set; }
        public int? Usable { get; set; }
    }

    public class PipelineTaggedInfoNoDefinitionDTO : PipelineInfoBase
    {
        public List<string> Tags { get; set; }
    }

    public class PipelineTaggedInfoDTO : PipelineTaggedInfoNoDefinitionDTO
    {
        public string Definition { get; set; }
    }
    
    public class PipelineTaggedInfoNoDefinitionFactory
    {
        static PipelineTaggedInfoNoDefinitionFactory()
        {
            Mapper.CreateMap<Pyxis.Contract.Publishing.GeoSource, PipelineTaggedInfoNoDefinitionDTO>();
            Mapper.CreateMap<Pyxis.Contract.Publishing.Metadata, PipelineTaggedInfoNoDefinitionDTO>().
                ForMember(x => x.User, opt => opt.MapFrom(m => m.User.Name));
        }

        static public PipelineTaggedInfoNoDefinitionDTO Create(Pyxis.Contract.Publishing.GeoSource geoSource)
        {
            var pipelineTaggedInfoNoDefinitionDTO = Mapper.Map<PipelineTaggedInfoNoDefinitionDTO>(geoSource);
            Mapper.Map(geoSource.Metadata, pipelineTaggedInfoNoDefinitionDTO, typeof(Pyxis.Contract.Publishing.Metadata), typeof(PipelineTaggedInfoNoDefinitionDTO));
            var externalImageUrl = geoSource.Metadata.ExternalUrls.FirstOrDefault(x => x.Type == Pyxis.Contract.Publishing.ExternalUrlType.Image);
            pipelineTaggedInfoNoDefinitionDTO.ImageUrl = externalImageUrl == null ? null : externalImageUrl.Url;
            return pipelineTaggedInfoNoDefinitionDTO;
        }
    }
}