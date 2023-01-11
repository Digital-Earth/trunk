using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel.PeerResolvers;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.MultiDomain
{
    public class SimpleMultiDomainGeoSourceResolver : IMultiDomainGeoSourceResolver
    {
        public List<Domain> Domains { get; private set; }

        public delegate bool Resolver(Dictionary<string, object> domains, ref ResourceReference geoSource);

        private List<Resolver> Resolvers { get; set; }

        public SimpleMultiDomainGeoSourceResolver(List<Domain> domains)
        {
            Domains = domains;
            Resolvers = new List<Resolver>();
        }

        public void Register(Dictionary<string, object> domains, ResourceReference result)
        {
            if (domains == null)
            {
                throw new ArgumentNullException("domains");
            }

            if (domains.Count == 0)
            {
                throw new ArgumentException("No domains were provided.","domains");
            }

            //push resolver to the front
            Register((Dictionary<string, object> compareAgainst, ref ResourceReference outGeoSource) =>
            {
                if (domains.All(d => compareAgainst.ContainsKey(d.Key) && d.Value.Equals(compareAgainst[d.Key])))
                {
                    outGeoSource = result;
                    return true;
                }
                return false;
            });
        }

        public void Register(Resolver resolver)
        {
            if (resolver == null)
            {
                throw new ArgumentNullException("resolver");
            }

            Resolvers.Insert(0, resolver);
        }

        public ResourceReference Resolve(Dictionary<string, object> domains)
        {
            ResourceReference result = null;

            if (Resolvers.Any(resolver => resolver(domains, ref result)))
            {
                return result;
            }

            throw new Exception("Failed resolve GeoSource for " + JsonConvert.SerializeObject(domains));
        }
    }
}
