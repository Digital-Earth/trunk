using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Publishing
{
    public enum GroupPermission
    {
        View,
        Publish
    }

    public class GroupInfo
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public UserInfo Channel { get; set; }

        public GroupInfo()
        {
        }

        public GroupInfo(Guid id, string name, UserInfo channel)
        {
            Id = id;
            Name = name;
            Channel = new UserInfo(channel);
        }

        public GroupInfo(Group group)
        {
            Id = group.Id;
            Name = group.Metadata.Name;
            Channel = new UserInfo(group.Metadata.User);
        }

        public GroupInfo(GroupInfo groupInfo)
        {
            Id = groupInfo.Id;
            Name = groupInfo.Name;
            Channel = new UserInfo(groupInfo.Channel);
        }
    }

    public class GroupPermissionInfo : GroupInfo
    {
        public GroupPermission Permission { get; set; }

        public GroupPermissionInfo()
        {
        }

        public GroupPermissionInfo(Guid id, string name, UserInfo channel, GroupPermission permission)
            : base(id, name, channel)
        {
            Permission = permission;
        }

        public GroupPermissionInfo(GroupPermissionInfo groupPermissionInfo)
            :base(groupPermissionInfo)
        {
            Permission = groupPermissionInfo.Permission;
        }

        public GroupPermissionInfo(Group group, GroupPermission groupPermission)
            : base(group)
        {
            Permission = groupPermission;
        }
    }

    public class Group : Resource
    {
        public List<UserInfo> Members { get; set; }

        // for deserializing from string
        public Group()
        {
        }

        public Group(List<LicenseReference> licenses, Metadata metadata, Guid version, List<UserInfo> members)
            : base(ResourceType.Group, licenses, metadata, version)
        {
            Members = new List<UserInfo>(members);
        }

        public Group(Group basedOnGroup)
            : base(basedOnGroup)
        {
            Members = new List<UserInfo>(basedOnGroup.Members);
        }

        public Group(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, List<UserInfo> members)
            : this(licenses, metadata, version, members)
        {
            Id = id;
        }
    }
}
