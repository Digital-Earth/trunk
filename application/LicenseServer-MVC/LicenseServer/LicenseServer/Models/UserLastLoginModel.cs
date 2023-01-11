using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models
{
    public class UserLastLoginModel
    {
        public string Name { get; set; }

        public string Email { get; set; }

        public DateTime? LastLogin { get; set; }

        public Guid? ResourceId { get; set; }

        public string LoginMethod { get; set; }

        public UserLastLoginModel()
        {
            
        }

        public UserLastLoginModel(PyxisIdentityUser user)
        {
            Name = user.UserName;
            Email = user.Email;
            LoginMethod = user.ExternalLoginProvider ?? "Password";
            ResourceId = user.ResourceId;
            LastLogin = user.LastLogin;
        }
    }
}