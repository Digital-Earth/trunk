using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using AutoMapper;
using LicenseServer.Models.Mongo;

namespace LicenseServer.Extensions
{
    public class UserProfileFactory
    {
        static UserProfileFactory()
        {
            Mapper.CreateMap<User, Pyxis.Contract.Publishing.UserProfile>();
            Mapper.CreateMap<PyxisIdentityUser, Pyxis.Contract.Publishing.UserProfile>()
                .ForMember(x => x.Id, opt => opt.Ignore())
                .ForMember(x => x.Type, opt => opt.Ignore());
        }

        static public Pyxis.Contract.Publishing.UserProfile Create(User user, PyxisIdentityUser identity)
        {
            var userProfileDTO = Mapper.Map<Pyxis.Contract.Publishing.UserProfile>(user);
            Mapper.Map(identity, userProfileDTO, typeof(PyxisIdentityUser), typeof(Pyxis.Contract.Publishing.UserProfile));
            return userProfileDTO;
        }
    }

    public static class UserProfileExtensions
    {
        static UserProfileExtensions()
        {
            Mapper.CreateMap<Pyxis.Contract.Publishing.UserProfile, User>()
                .ForAllMembers(x => x.Condition(p => !p.IsSourceValueNull));
            Mapper.CreateMap<Pyxis.Contract.Publishing.UserProfile, PyxisIdentityUser>()
                .ForMember(x => x.ProfileName, opt => opt.MapFrom(s => s.Metadata.Name))
                .ForMember(x => x.ResourceId, opt => opt.MapFrom(s => s.Id));
        }

        public static User ExtractUser(this Pyxis.Contract.Publishing.UserProfile userProfile)
        {
            return Mapper.Map<User>(userProfile);
        }

        public static PyxisIdentityUser ExtractIdentity(this Pyxis.Contract.Publishing.UserProfile userProfile)
        {
            return Mapper.Map<PyxisIdentityUser>(userProfile);
        }
    }
}