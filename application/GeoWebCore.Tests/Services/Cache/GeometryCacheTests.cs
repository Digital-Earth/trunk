using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Services.Cache;
using Newtonsoft.Json;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO.GeoJson;

namespace GeoWebCore.Tests.Services.Cache
{
    [TestFixture]
    internal class GeometryCacheTests
    {
        /// <summary>
        /// Small geometries (in terms of size of json serialization) get the key that is the object it self. and we don't store it to disk
        /// </summary>
        [Test]
        public void ShortGeometryKeyIsSameAsValue()
        {
            var geometry = new Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry
            {
                Resource = new ResourceReference()
                {
                    Id = Guid.Parse("29dc91db-026b-4e3c-a876-a9432d7f6915"),
                    Version = Guid.Parse("d629d943-66a1-4106-8599-d9f50d38f554"),
                    Type = ResourceType.GeoSource
                },
                FeatureId = "0"
            };

            var key = GeometryCacheSingleton.Add(geometry);
            var value = JsonConvert.SerializeObject(geometry);

            Assert.AreEqual(key,value);
        }

        [Test]
        public void BigGeometryKeyIsHash()
        {
            var geometry = new PolygonGeometry()
            {
                Coordinates = JsonConvert.DeserializeObject<List<List<GeographicPosition>>>("[[[-118.92651144484887,38.39375386598281],[-119.0177664269288,38.39396830599719],[-119.22307735753816,38.394218837147875],[-119.49659160119501,38.322146225146994],[-119.63249163463698,38.160102143706546],[-119.76778075169429,37.99798749708073],[-119.83499211958373,37.88995961946599],[-119.85677826300598,37.78213323436043],[-119.85493219830057,37.54877606731116],[-119.85354382125989,37.36929957070343],[-119.85260111514555,37.24367092743824],[-119.82799418593447,36.93864326359543],[-119.71529186428815,36.77753776164501],[-119.51393029241686,36.59854422398313],[-119.2468054635619,36.47302380300443],[-119.06909002511479,36.418902582216575],[-118.73590035065406,36.399774202407635],[-118.15675929851211,36.485264312531626],[-117.9096879226547,36.60832029627889],[-117.22619795016536,37.11936552234878],[-117.08116702731506,37.45835579424282],[-116.97279203860114,38.01399420992781],[-116.95685530961083,38.48195489374781],[-117.33240603911443,38.97485713088307],[-117.55840057931381,39.158418198528835],[-117.78900475754622,39.19711919771471],[-117.97415528767901,39.19893385638794],[-118.27486207817702,39.20135088139438],[-118.55270544385,39.16690981660919],[-118.90000918407793,39.02390006660922],[-119.03843453499152,38.970118414643814],[-119.08452158261336,38.95216427132854],[-119.15369295823486,38.88014415046742],[-119.17673453515431,38.84412335555161],[-119.22270547227374,38.80812527261366],[-119.22270547227374,38.80812527261366],[-119.2456917665747,38.754102776327194],[-119.2456917665747,38.754102776327194],[-119.24568168527601,38.7721114022632]]]")
            };

            var key = GeometryCacheSingleton.Add(geometry);
            var value = JsonConvert.SerializeObject(geometry);

            Assert.AreNotEqual(key, value);
            Assert.IsTrue(key.StartsWith(geometry.Type.ToString()));
        }
    }
}
