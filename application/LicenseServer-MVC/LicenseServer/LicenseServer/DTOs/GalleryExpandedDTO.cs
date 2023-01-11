using System.Collections.Generic;
using System.Linq;
using LicenseServer.Models.Mongo;

namespace LicenseServer.DTOs
{
    public class GalleryExpandedFactory
    {
        static public GalleryExpandedDTO Create(Gallery gallery, IQueryable<Pyxis.Contract.Publishing.Resource> containedResources)
        {
            var unorderedResources = containedResources.ToList();
            var orderedResources = new List<Pyxis.Contract.Publishing.Resource>();
            foreach (var id in gallery.Resources.Select(r => r.ResourceId))
            {
                var resource = unorderedResources.FirstOrDefault(r => r.Id == id);
                if (resource != null)
                {
                    orderedResources.Add(resource);
                }
            }
            return new GalleryExpandedDTO { Gallery = gallery, Resources = orderedResources };
        }
    }

    public class GalleryExpandedDTO
    {
        public Gallery Gallery { get; set; }
        public List<Pyxis.Contract.Publishing.Resource> Resources { get; set; }
    }
}