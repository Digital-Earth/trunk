using System.Collections.Generic;
using System.Security.Principal;
using Newtonsoft.Json;

namespace Pyxis.Contract.Workspaces
{
    public class Workspace : ObjectWithPermissions
    {
        [JsonProperty("endpoints")]
        public Dictionary<string, Endpoint> Endpoints;

        [JsonProperty("imports")]
        public Dictionary<string, IImport> Imports;

        [JsonProperty("globes")]
        public Dictionary<string, GlobeTemplate> Globes;

        public static Workspace CreateEmpty()
        {
            return new Workspace
            {
                Endpoints = new Dictionary<string, Endpoint>(),
                Imports = new Dictionary<string, IImport>(),
                Globes = new Dictionary<string, GlobeTemplate>()
            };
        }

        /// <summary>
        /// This function should be used to remove the Permission details and Owner Id when sending those workspaces over the network
        /// </summary>
        /// <param name="user">User requesting the workspace, can be null</param>
        /// <returns>a shallow copy of the workspace without permissions details that not visible to user</returns>
        public Workspace CreateCopyWithoutSensativeDetails(IPrincipal user)
        {
            var clone = new Workspace
            {
                Endpoints = Endpoints,
                Imports = Imports,
                Globes = Globes,
            };

            //no onwer nor pemrissions - we done
            if (Owner == null)
            {
                return clone;
            }

            //this is the owner, it menas it have full access
            if (user != null && user.Identity.Name == Owner.Id)
            {
                clone.Owner = Owner;
                clone.Permissions = Permissions;
            }
            else
            {
                //not the current user, just return the name
                clone.Owner = new OnwerDetails() { Name = Owner.Name };    
            }

            return clone;
        }
    }
}
