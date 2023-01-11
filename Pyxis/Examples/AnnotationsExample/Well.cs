using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Core.IO.Reflection;

namespace AnnotationsExample
{
    public class Well
    {
        [FeatureId]
        public Guid Id { get; set; }

        public string Status { get; set; }

        public string Operator { get; set; }

        public float Depth { get; set; }

        public float Production { get; set; }

        public Pyxis.Core.IO.GeoJson.PointGeometry Point { get; set; }
    }


    static class WellGenerator
    {
        public static Well CreateRandomWell()
        {
            return new Well()
            {
                Id = Guid.NewGuid(),
                Operator = GetRandomOperator(),
                Status = GetRandomStatus(),
                Depth = GetRandomDepth(),
                Production = GetRandomProduction(),
                Point = GetRandomPoint()
            };
        }


        static Random random = new Random();

        static string[] operators = new string[] { "Suncor Energy Inc.", "Imperial Oil Ltd.", "Encana Corporation", "Husky Energy Inc.", "Cenovus Energy Inc.", "Talisman Energy Inc.", "Canadian Natural Resources Ltd.", "Crescent Point Energy Corp." };
        static string[] status = new string[] { "Idle", "Drilling", "Pumping", "Maintenance" };

        private static string GetRandomOperator()
        {
            return operators[random.Next(operators.Length)];
        }

        private static string GetRandomStatus()
        {
            return status[random.Next(status.Length)];
        }

        private static float GetRandomProduction()
        {
            return GetRandomNumber(100, 4000);
        }

        private static float GetRandomDepth() 
        {
            return GetRandomNumber(100,4000);
        }

        private static float GetRandomNumber(float min,float max) 
        {
            return (float)random.NextDouble()*(max-min)+min;
        }
        
        private static Pyxis.Core.IO.GeoJson.PointGeometry GetRandomPoint()
        {
            return new Pyxis.Core.IO.GeoJson.PointGeometry()
            {
                Coordinates = Pyxis.Core.IO.GeoJson.GeographicPosition.FromWgs84LatLon(GetRandomNumber(48.998036f, 59.985094f), GetRandomNumber(-120.004238f, -109.995693f))
            };
        }
    }
}
