using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo.Interface
{
    public enum MongoStringCompareOptions
    {
        CaseSensitive,
        CaseInsensitive
    }

    public interface IResources
    {
        IQueryable<Resource> GetResources();
        IQueryable<Resource> GetResources(List<ResourceType> types);
        IQueryable<T> GetResources<T>() where T : Resource;
        IQueryable<Resource> GetResourcesByIds(List<Guid> ids);
        IQueryable<T> GetResourcesByIds<T>(List<Guid> ids) where T : Resource;
        IQueryable<Resource> GetResourcesByName(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive);
        IQueryable<T> GetResourcesByName<T>(string name, MongoStringCompareOptions options = MongoStringCompareOptions.CaseSensitive) where T : Resource;
        IQueryable<Resource> GetResourcesByName(Guid userId, string name);
        IQueryable<T> GetResourcesByName<T>(Guid userId, string name) where T : Resource;
        IQueryable<Resource> SearchResources(string search);
        IQueryable<Resource> SearchResources(string search, List<ResourceType> types);
        IQueryable<ExternalData> SearchExternal(string search, List<double> center, Envelope bbox, int skip, int top);
        IQueryable<string> SuggestTerms(string search, List<ResourceType> types);
        IQueryable<string> SuggestCompletions(string search, List<ResourceType> types);
        IQueryable<string> SuggestMatches(string search, List<ResourceType> types);
        IQueryable<T> SearchResources<T>(string search) where T : Resource;
        IQueryable<Resource> SearchGallery(Guid id, string search);
        Resource GetResourceById(Guid id);
        T GetResourceById<T>(Guid id) where T : Resource;
        IQueryable<Resource> GetResourceVersionsById(Guid id);
        IQueryable<T> GetResourceVersionsById<T>(Guid id) where T : Resource;
        Resource GetResourceByIdAndVersion(Guid id, Guid version);
        T GetResourceByIdAndVersion<T>(Guid id, Guid version) where T : Resource;
        IQueryable<Resource> GetResourcesByUserId(Guid id);
        IQueryable<T> GetResourcesByUserId<T>(Guid id) where T : Resource;
        long GetStorageByUserId(Guid id);
        IQueryable<Resource> GetResourcesByGroupIds(List<Guid> groupIds);
        IQueryable<T> GetResourcesByGroupIds<T>(List<Guid> groupIds) where T : Resource;
        IQueryable<ResourceGrouping> GetResourceGroupings(string field);
        IQueryable<ResourceGrouping> GetResourceGroupings<T>(string field) where T : Resource;
        IQueryable<ResourceGrouping> GetResourceGroupings(string search, string field);
        IQueryable<ResourceGrouping> GetResourceGroupings(string search, List<ResourceType> types, string field);
        IQueryable<ResourceGrouping> GetResourceGroupings<T>(string search, string field) where T : Resource;
        IQueryable<ResourceGrouping> GetGalleryGroupings(Guid id, string field);
        IQueryable<ResourceGrouping> GetGalleryGroupings(Guid id, string search, string field);
        Resource GetPipelineByProcRef(string procRef); // string representing legacy process reference "{Guid}[int]"
        Resource GetPipelineByIdAndVersion(Guid id, Guid version); // Resource.Id, Resource.Version
        IQueryable<Resource> GetUpdates(List<Guid> ids, DateTime lastUpdated);
        IQueryable<T> GetUpdates<T>(List<Guid> ids, DateTime lastUpdated) where T : Resource;
        void InsertResource<T>(T resource) where T : Resource;
        void UpdateResource<T>(Guid id, Guid version, T resource) where T : Resource;
        void RemoveResource<T>(Guid id) where T : Resource;
    }
}
