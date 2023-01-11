using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.IO.DataDiscovery
{
    public static class UriExtensions
    {
        public static bool IsScheme(this Uri uri, params string[] schemas)
        {
            return schemas.Any(scheme => uri.Scheme == scheme);
        }

        public static bool IsPath(this Uri uri, string path)
        {
            return IsPath(uri, path.Split(new[] {'/'}, StringSplitOptions.RemoveEmptyEntries));
        }

        public static bool IsPath(this Uri uri, string[] pathSegments)
        {
            var segements = uri.AbsolutePath.Split(new[] { '/' }, StringSplitOptions.RemoveEmptyEntries);
            var currentSegment = 0;
            var lookupSegment = 0;
            var recursiveLookup = false;
            while (currentSegment < segements.Length && lookupSegment < pathSegments.Length)
            {
                if (pathSegments[lookupSegment] == "**")
                {
                    lookupSegment++;
                    recursiveLookup = true;
                }
                else if (pathSegments[lookupSegment] == "*")
                {
                    currentSegment++;
                    lookupSegment++;
                }
                else if (pathSegments[lookupSegment].Split('|').Any((segement) => segement.Equals(segements[currentSegment], StringComparison.InvariantCultureIgnoreCase)))
                {
                    currentSegment++;
                    lookupSegment++;
                    recursiveLookup = false;
                }
                else if (recursiveLookup)
                {
                    currentSegment++;
                }
                else
                {
                    //proabebly a failure
                    break;
                }
            }
            //edge case for patterns that ends with "**" and there are no segments on the path left (zero case)
            if (lookupSegment == pathSegments.Length - 1 && pathSegments[lookupSegment] == "**" && currentSegment == segements.Length)
            {
                return true;
            }
            return lookupSegment == pathSegments.Length && (currentSegment == segements.Length || recursiveLookup);
        }
    }
}
