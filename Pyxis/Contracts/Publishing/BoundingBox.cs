using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace Pyxis.Contract.Publishing
{
    public class BoundingBoxCorner
    {
        public double X { get; set; }
        public double Y { get; set; }

        public BoundingBoxCorner()
        {
            
        }

        public BoundingBoxCorner(double x, double y)
        {
            X = x;
            Y = y;
        }
    }

    public class BoundingBox
    {
        public BoundingBoxCorner LowerLeft;
        public BoundingBoxCorner UpperRight;

        /// <summary>
        /// Spatial reference system as a string.
        /// If null, EPSG:4326 is assumed.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Srs { get; set; }

        public bool Intersects(BoundingBox other)
        {
            if (Srs != other.Srs)
            {
                throw new Exception("can't check intersetions for BoundingBox with different SRS");
            }

            return (LowerLeft.X < other.UpperRight.X) && (other.LowerLeft.X < UpperRight.X) &&
                   (LowerLeft.Y < other.UpperRight.Y) && (other.LowerLeft.Y < UpperRight.Y);
        }
    }
}
