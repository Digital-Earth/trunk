using System.Collections.Generic;
using System.Linq;
using LicenseServer.Models.Mongo;

namespace LicenseServer.DTOs
{
    public class GeoSourceExpandedFactory
    {
        static public GeoSourceExpandedDTO Create(MultiDomainGeoSource geoSource, User publisher)
        {
            return new GeoSourceExpandedDTO { GeoSource = geoSource, User = publisher };
        }
    }

    public class GeoSourceExpandedDTO
    {
        public MultiDomainGeoSource GeoSource { get; set; }
        public User User { get; set; }
    }
}