using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using AutoMapper;
using Pyxis.Contract.Publishing;
using PyxNet;
using PyxNet.DLM;
using PyxNet.Service;

namespace LicenseServer.Extensions.PyxNet
{
    public static class NodeIdExtensions
    {
        static NodeIdExtensions()
        {
            Mapper.CreateMap<PyxNetNodeId.PyxNetPublicKey, PublicKey>();
            Mapper.CreateMap<PyxNetNodeId, NodeId>();

            Mapper.CreateMap<PublicKey, PyxNetNodeId.PyxNetPublicKey>();
            Mapper.CreateMap<NodeId, PyxNetNodeId>()
                .ForMember(x => x.Id, opt => opt.MapFrom(n => n.Identity));
        }

        public static NodeId ToNodeId(this PyxNetNodeId pyxNetNodeId)
        {
            return Mapper.Map<NodeId>(pyxNetNodeId);
        }

        public static PyxNetNodeId ToPyxNetNodeId(this NodeId nodeId)
        {
            return Mapper.Map<PyxNetNodeId>(nodeId);
        }
    }

    public static class LicenseLimitationExtensions
    {
        static LicenseLimitationExtensions()
        {
            Mapper.CreateMap<LicenseTrialLimitations, GeoSourcePermissionFact.PermissionLimitations>();
        }

        public static GeoSourcePermissionFact.PermissionLimitations ToPermissionLimitations(this LicenseTrialLimitations limitations)
        {
            return Mapper.Map<GeoSourcePermissionFact.PermissionLimitations>(limitations);
        }
    }
}