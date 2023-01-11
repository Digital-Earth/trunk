using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;

namespace LicenseServer.Extensions
{
    public static class PyxisIdentityUserExtensions
    {
        public static bool IsInAdminRole(this PyxisIdentityUser identity)
        {
            return identity.IsInRole(PyxisIdentityRoles.ChannelAdmin, PyxisIdentityRoles.Admin, PyxisIdentityRoles.SiteAdmin);
        }

        public static bool IsInChannelAdminRole(this PyxisIdentityUser identity)
        {
            return identity.IsInRole(PyxisIdentityRoles.ChannelAdmin);
        }

        public static bool IsInPyxisAdminRole(this PyxisIdentityUser identity)
        {
            return identity.IsInRole(PyxisIdentityRoles.Admin, PyxisIdentityRoles.SiteAdmin);
        }
    }
}