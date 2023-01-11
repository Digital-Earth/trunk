using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;

namespace LicenseServer.DTOs
{
    public class GroupExpandedFactory
    {
        static public GroupExpandedDTO Create(Group group, IQueryable<Pyxis.Contract.Publishing.Resource> containedMembers)
        {
            return new GroupExpandedDTO { Group = group, Members = containedMembers.ToList() };
        }
    }

    public class GroupExpandedDTO
    {
        public Group Group { get; set; }
        public List<Pyxis.Contract.Publishing.Resource> Members { get; set; }
    }
}