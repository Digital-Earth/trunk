using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.DTOs;
using LicenseServer.Models.Mongo.Interface;
using MongoDB.Driver;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public interface IMongoDBEntities : IAuthorizable, IResources, IPipelines, IActivities, IParameters, IGwsses, IIdentities
    {
    }
}