using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Services;
using GeoWebCore.Services.Cluster;
using NUnit.Framework;

namespace GeoWebCore.Tests.Services
{
    [TestFixture]
    internal class HashRingTests
    {
        [Test]
        public void EmptyHashRingReturnNull()
        {
            var hashRing = new HashRing(10);

            Assert.IsNullOrEmpty(hashRing.GetEndpoint("hello"));
        }

        [Test]
        public void HashRingCreateStopPointsForEveryNode()
        {
            var hashRing = new HashRing(100);
            hashRing.SetEndpoints(new List<string> { "http://localhost:8000", "http://localhost:8001", "http://localhost:8002" , "http://localhost:8003" , "http://localhost:8004" , "http://localhost:8005" });

            var pointsPerNode = hashRing.GetStopPoints().GroupBy(t => t.Value).ToDictionary(t=>t.Key,t=>t.Count());

            foreach (var keyValue in pointsPerNode)
            {
                Assert.AreEqual(100, keyValue.Value);
            }
        }

        [Test]
        public void GetEndPointsReturnBasedNextFoundEndpoint()
        {
            var hashRing = new HashRing(16);
            hashRing.SetEndpoints(new List<string> { "http://localhost:8000", "http://localhost:8001", "http://localhost:8002", "http://localhost:8003", "http://localhost:8004", "http://localhost:8005" });

            foreach (var stopPoint in hashRing.GetStopPoints())
            {
                var hash = stopPoint.Key;
                Assert.AreEqual(stopPoint.Value, hashRing.GetEndpoint(hash - 1));
                Assert.AreEqual(stopPoint.Value, hashRing.GetEndpoint(hash));                
            }

            var firstStop = hashRing.GetStopPoints().First();
            var lastStop = hashRing.GetStopPoints().Last();

            Assert.AreEqual(firstStop.Value,hashRing.GetEndpoint(lastStop.Key + 1));

        }

    }
}
