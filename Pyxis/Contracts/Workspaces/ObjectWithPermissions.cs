using System.Collections.Generic;
using System.Security.Principal;
using System.Threading;
using Newtonsoft.Json;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// Generic base class to add authentication and permissions attributes to an object
    /// </summary>
    public class ObjectWithPermissions
    {
        [JsonProperty("owner", NullValueHandling = NullValueHandling.Ignore)]
        public OnwerDetails Owner { get; set; }

        [JsonProperty("permissions", NullValueHandling = NullValueHandling.Ignore)]
        public Dictionary<string, List<string>> Permissions { get; set; }


        public bool CanAccess()
        {
            if (Thread.CurrentPrincipal != null)
            {
                return CanAccess(Thread.CurrentPrincipal);
            }
            return CanAccess(null);
        }

        public bool CanAccess(IPrincipal user)
        {            
            return CanDoAction("access", user);
        }

        public bool CanDoAction(string action)
        {
            if (Thread.CurrentPrincipal != null)
            {
                return CanDoAction(action, Thread.CurrentPrincipal);
            }
            return CanDoAction(action, null);
        }

        public bool CanDoAction(string action, IPrincipal user)
        {
            //onwer can do anything
            if (Owner != null)
            {
                if (user != null && Owner.Id == user.Identity.Name)
                {
                    return true;
                }
            }
            else
            {
                return true;
            }

            if (Permissions == null || !Permissions.ContainsKey(action))
            {
                return false;
            }
           

            foreach (var group in Permissions[action])
            {
                if (@group == "everyone")
                {
                    return true;
                }
                if (user != null && @group == user.Identity.Name)
                {
                    return true;
                }
            }

            return false;
        }
    }
}