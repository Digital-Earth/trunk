using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;

namespace LicenseServer.App_Utilities
{
    public static class ResourceTypeResolver
    {
        private static Dictionary<Type, Pyxis.Contract.Publishing.ResourceType> s_typeMap = new Dictionary<Type, Pyxis.Contract.Publishing.ResourceType> {
            {typeof(GeoSource), Pyxis.Contract.Publishing.ResourceType.GeoSource},
            {typeof(MultiDomainGeoSource), Pyxis.Contract.Publishing.ResourceType.GeoSource},
            {typeof(User), Pyxis.Contract.Publishing.ResourceType.User},
            {typeof(Group), Pyxis.Contract.Publishing.ResourceType.Group},
            {typeof(License), Pyxis.Contract.Publishing.ResourceType.License},
            {typeof(Gallery), Pyxis.Contract.Publishing.ResourceType.Gallery},
            {typeof(File), Pyxis.Contract.Publishing.ResourceType.File},
            {typeof(Pipeline), Pyxis.Contract.Publishing.ResourceType.Pipeline},
            {typeof(Map), Pyxis.Contract.Publishing.ResourceType.Map},
            {typeof(Product), Pyxis.Contract.Publishing.ResourceType.Product}
        };

        public static Pyxis.Contract.Publishing.ResourceType? Resolve<T>() where T : Pyxis.Contract.Publishing.Resource
        {
            var currentType = typeof(T);
            Pyxis.Contract.Publishing.ResourceType resourceType;
            var contains = s_typeMap.TryGetValue(currentType, out resourceType);
            if (contains)
            {
                return s_typeMap[currentType];
            }
            return null;
        }
    }
}