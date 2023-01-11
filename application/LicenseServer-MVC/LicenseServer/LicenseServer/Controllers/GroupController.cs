using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Web.Http;
using System.Web.Http.Description;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Models.Mongo.Interface;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using Group = LicenseServer.Models.Mongo.Group;
using User = LicenseServer.Models.Mongo.User;
using Gallery = LicenseServer.Models.Mongo.Gallery;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Group")]
    public class GroupController : BaseResourceController<Group>
    {
        private static readonly Regex s_groupNameRegex = new Regex(@"[a-zA-Z0-9]{5,}");

        public GroupController() 
        { }

        // Inject for test
        public GroupController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/Group/Available?Name={name}
        [HttpGet]
        [Route("Available")]
        [Authorize(Roles = PyxisIdentityRoleGroups.AllAdmins)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public HttpResponseMessage Available(string name)
        {
            if (db.GetResourcesByName<Group>(name, MongoStringCompareOptions.CaseInsensitive)
                .FirstOrDefault(g => g.Metadata.User.Id == CurrentUserIdentity.ResourceId) == null)
            {
                return Request.CreateResponse(HttpStatusCode.OK);
            }
            return Request.CreateResponse(HttpStatusCode.Conflict);
        }

        // GET api/v1/Group/{id}/Expanded
        [HttpGet]
        [Route("{id}/Expanded")]
        [ResponseType(typeof(GroupExpandedDTO))]
        public HttpResponseMessage GetExpandedGroup(Guid id)
        {
            return GetExpandedGroup(id, ResultFormat.Basic);
        }

        // GET api/v1/Group/{id}/Expanded?Format=(Full|Basic|View}
        [HttpGet]
        [Route("{id}/Expanded")]
        [ResponseType(typeof(GroupExpandedDTO))]
        public HttpResponseMessage GetExpandedGroup(Guid id, ResultFormat format)
        {
            var groupResponse = base.Get(id);
            Group group;
            if (groupResponse.TryGetContentValue(out group))
            {
                var containedMembers = db.GetResourcesByIds(group.Members.Select(r => r.Id).ToList()).FormatResources(format);
                var groupExpandedDto = GroupExpandedFactory.Create(group, containedMembers);
                return Request.CreateResponse(HttpStatusCode.OK, groupExpandedDto);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }
        }

        // POST api/v1/Group
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(Group group)
        {
            if (group.Metadata == null || group.Metadata.Name == null || !s_groupNameRegex.IsMatch(group.Metadata.Name))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Group Metadata.Name must 5 or more alphanumeric characters");
            }
            if (db.GetResourcesByName<Group>(group.Metadata.Name, MongoStringCompareOptions.CaseInsensitive)
                .FirstOrDefault(g => g.Metadata.User.Id == CurrentUserIdentity.ResourceId) != null)
            {
                return Request.CreateResponse(HttpStatusCode.Conflict, "Group Metadata.Name is already taken, please choose a different name");
            }
            // Add Group owner as a Member by default
            UserInfo owner = null;
            if (group.Metadata.User != null && !CurrentUserIdentity.IsInPyxisAdminRole())
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Group Metadata.User cannot be specified");
            }
            if(group.Metadata.User != null && CurrentUserIdentity.IsInPyxisAdminRole())
            {
                var user = db.GetResourceById<User>(group.Metadata.User.Id);
                if (user == null)
                {
                    Request.CreateResponse(HttpStatusCode.BadRequest, "Specified User does not exist");
                }
                owner = new UserInfo(user);
                group.Metadata.User = owner;
            }
            else
            {
                owner = CurrentUserIdentity.UserInfo;
            }
            group.Members = new List<UserInfo> { owner };

            var response = Retry.Execute(() => base.Post(group));

            // Add the Group to the User's list of Groups
            var groupWithUser = response.Content.ReadAsAsync<Group>().Result;
            var publisher = Retry.Execute(() => db.GetResourceById<User>(groupWithUser.Metadata.User.Id));
            publisher.Groups.Add(new GroupInfo(groupWithUser));

            Retry.Execute(() => db.UpdateResource(publisher.Id, publisher.Version, new User { Groups = publisher.Groups}));

            return response;
        }

        // PUT api/v1/Group
        [Authorize(Roles = PyxisIdentityRoleGroups.NotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Put(Guid id, Guid version, Group group)
        {
            if (group.Metadata != null && group.Metadata.Name != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Group Metadata.Name cannot be changed");
            }
            if (group.Members != null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Use Member endpoints to add and remove members");
            }
            return base.Put(id, version, group);
        }

        // PUT api/v1/Group/{id}/Member?MemberId={memberId}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPut]
        [Route("{id}/Member")]
        public HttpResponseMessage AddGroupMember(Guid id, Guid memberId)
        {
            var group = db.GetResourceById<Group>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(group))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified Group");
            }
            var member = db.GetResourceById<User>(memberId);
            if (member == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified member does not exist");
            }
            if (group.Members.Any(m => m.Id == memberId))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The Group already contains the specified member");
            }

            group.Members.Add(new UserInfo(member));
            var response = base.Put(id, group.Version, new Group { Members = group.Members });

            return AddToUserGroups(Request, member, group, response);
        }

        // DELETE api/v1/Group/{id}/Member?MemberId={memberId}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpDelete]
        [Route("{id}/Member")]
        public HttpResponseMessage RemoveGroupMember(Guid id, Guid memberId)
        {
            var group = db.GetResourceById<Group>(id);
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(group))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to modify specified Group");
            }
            if (group.Metadata.User.Id == memberId)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The Group owner must be a member of the Group");
            }
            var member = db.GetResourceById<User>(memberId);
            if (member == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified member does not exist");
            }
            if (!group.Members.Any(m => m.Id == memberId))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The Group does not contain the specified member");
            }

            group.Members.RemoveAll(r => r.Id == memberId);
            var response = base.Put(id, group.Version, new Group { Members = group.Members });

            return RemoveFromUserGroups(Request, member, group, response);
        }

        // DELETE api/v1/Group/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Delete(Guid id)
        {
            var group = db.GetResourceById<Group>(id);
            if (group == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The specified group does not exist.");
            }
            if (!CurrentUserIdentity.IsInPyxisAdminRole() && !ValidateOwnership(group))
            {
                return Request.CreateResponse(HttpStatusCode.Unauthorized, "Not authorized to delete requested resource");
            }
            var attachedGalleries = db.GetResourcesByGroupIds<Gallery>(new List<Guid>{group.Id}).Select(g => g.Metadata.Name).ToList();
            if (attachedGalleries.Any())
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest,
                    String.Format("The group must be removed from the following galler{0} before it can be deleted: {1}",
                        attachedGalleries.Count() > 1 ? "ies" : "y",
                        String.Join(", ", attachedGalleries)
                        ));
            }
            if (group.Members.Any(m => m.Id != group.Metadata.User.Id))
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "The group must be empty (except for the owner) before it can be deleted.");
            }

            try
            {
                db.RemoveResource<Group>(id);
                var owner = Retry.Execute(() => db.GetResourceById<User>(group.Metadata.User.Id));
                return RemoveFromUserGroups(Request, owner, group);
            }
            catch (DataLayerException exception)
            {
                return exception.ToHttpResponse(Request);
            }
        }

        private HttpResponseMessage AddToUserGroups(HttpRequestMessage request, User user, Group group, HttpResponseMessage response = null)
        {
            var userUpdates = new User();
            var numGroups = user.Groups.Count;
            userUpdates.Groups = new List<GroupInfo>(user.Groups);
            userUpdates.Groups.Add(new GroupInfo(group));
            userUpdates.Groups = userUpdates.Groups.Distinct().ToList();
            if (numGroups != userUpdates.Groups.Count)
            {
                Retry.Execute(() => db.UpdateResource(user.Id, user.Version, userUpdates));
            }
            return response ?? request.CreateResponse(HttpStatusCode.OK);
        }

        private HttpResponseMessage RemoveFromUserGroups(HttpRequestMessage request, User user, Group group, HttpResponseMessage response = null)
        {
            var userUpdates = new User();
            userUpdates.Groups = new List<GroupInfo>(user.Groups);
            var numRemoved = userUpdates.Groups.RemoveAll(g => g.Id == group.Id);
            if (numRemoved > 0)
            {
                Retry.Execute(() => db.UpdateResource(user.Id, user.Version, userUpdates));
            }
            return response ?? request.CreateResponse(HttpStatusCode.OK);
        }
    }
}
