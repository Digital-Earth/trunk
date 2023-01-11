using System;
using System.Linq;
using Microsoft.AspNet.Identity;
using MongoDB.AspNet.Identity;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public interface IIdentityDelegate
    {
        string FirstName { get; set; }
        string LastName { get; set; }
        string Email { get; set; }
        string UserName { get; set; }
        Guid? ResourceId { get; set; }
    }

    public class PyxisIdentityUser : IdentityUser, IIdentityDelegate, IPyxisIdentityUser, IUser
    {
        public bool? AcceptTerms { get; set; }
        public bool? PromotionConsent { get; set; }
        public bool? AccountConsent { get; set; }

        public string FirstName { get; set; }
        public string LastName { get; set; }
        public string Email { get; set; }
        public bool? EmailConfirmed { get; set; }
        public byte[] EmailConfirmedHash { get; set; }
        public string ExternalLoginProvider { get; set; }
        public string Country { get; set; }
        public string City { get; set; }
        public DateTime BirthDate { get; set; }
        public string ProfileName { get; set; }
        public Guid? ResourceId { get; set; }
        public string CrmId { get; set; }

        public string BusinessName { get; set; }
        public bool? CollectTax { get; set; }
        public string PayPalEmail { get; set; }

        public byte[] ResetPasswordHash { get; set; }
        public DateTime? ResetExpiration { get; set; }
        public DateTime? LastLogin { get; set; }
        
        public UserInfo UserInfo
        {
            get
            {
                if (!ResourceId.HasValue)
                {
                    throw new InvalidOperationException("Cannot create UserInfo from an incomplete Identity.");
                }
                return new UserInfo(ResourceId.Value, ProfileName);
            }
        }

        public bool IsInRole(params string[] roles)
        {
            return roles.Any(role => Roles.Any(r => role.Equals(r, StringComparison.InvariantCultureIgnoreCase)));
        }
    }

    public class PyxisIdentityAffiliate
    {
        public string FirstName { get; set; }
        public string LastName { get; set; }
        public string Email { get; set; }
        public string UserName { get; set; }
        public Guid? ResourceId { get; set; }
    }

    public static class PyxisIdentityRoles
    {
        // admin authorization plus setting site parameters 
        public const string SiteAdmin = "siteAdmin";
        // authorized to modify other Identity's Resources and see all Resources and Activities
        public const string Admin = "admin";
        // authorized to delegate gallery ownership of their created galleries
        public const string ChannelAdmin = "channelAdmin";
        // authorization of a normal Identity
        public const string Member = "member";
        // limited authorization to create Resources and Activities (e.g. can't create Users or accept License agreements)
        public const string ApiAccess = "apiAccess";
        // unconfirmed email users do not receive any authorization over an unauthenticated user except creating a User
        public const string Unconfirmed = "unconfirmed"; 
    }

    public static class PyxisIdentityRoleGroups
    {
        public const string ConfirmedAndNotApi = 
            PyxisIdentityRoles.Member + "," + 
            PyxisIdentityRoles.ChannelAdmin + "," +
            PyxisIdentityRoles.Admin + "," + 
            PyxisIdentityRoles.SiteAdmin;

        public const string NotApi = 
            PyxisIdentityRoles.Unconfirmed + "," +
            PyxisIdentityRoles.Member + "," +
            PyxisIdentityRoles.ChannelAdmin + "," +
            PyxisIdentityRoles.Admin + "," + 
            PyxisIdentityRoles.SiteAdmin;

        public const string AllAdmins =
            PyxisIdentityRoles.ChannelAdmin + "," +
            PyxisIdentityRoles.Admin + "," +
            PyxisIdentityRoles.SiteAdmin;

        public const string PyxisAdmins = 
            PyxisIdentityRoles.Admin + "," + 
            PyxisIdentityRoles.SiteAdmin;
    }
}