using System;
using NUnit.Framework;
using PyxNet.DLM;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class GeoSourcePermissionFactTests
    {
        [Test]
        public void TestPermissionLimitationNullSerialization()
        {
            var limitation = new GeoSourcePermissionFact.PermissionLimitations
            {
                Area = null,
                Resolution = null,
                Time = null,
                Watermark = null
            };

            var message = new Message();
            message.Append((Int32)0); // fill the first 4 bytes reserved for message type
            limitation.ToMessage(message);
            var reader = new MessageReader(message);
            var deserializedLimitations = new GeoSourcePermissionFact.PermissionLimitations(reader);

            Assert.AreEqual(deserializedLimitations.Area, limitation.Area);
            Assert.AreEqual(deserializedLimitations.Resolution, limitation.Resolution);
            Assert.AreEqual(deserializedLimitations.Time, limitation.Time);
            Assert.AreEqual(deserializedLimitations.Watermark, limitation.Watermark);
        }

        [Test]
        public void TestPermissionLimitationNonNullSerialization()
        {
            var limitation = CreatePermissionLimitations();

            var message = new Message();
            message.Append((Int32)0); // fill the first 4 bytes reserved for message type
            limitation.ToMessage(message);
            var reader = new MessageReader(message);
            var deserializedLimitations = new GeoSourcePermissionFact.PermissionLimitations(reader);

            Assert.AreEqual(deserializedLimitations.Area, limitation.Area);
            Assert.AreEqual(deserializedLimitations.Resolution, limitation.Resolution);
            Assert.AreEqual(deserializedLimitations.Time, limitation.Time);
            Assert.AreEqual(deserializedLimitations.Watermark, limitation.Watermark);
        }

        [Test]
        public void TestGeoSourcePermissionFactSerialization()
        {
            var geoSourcePermissionFact = CreateGeoSourcePermissionFact();

            var message  = geoSourcePermissionFact.ToMessage();

            var deserializedGeoSourcePermissionFact = new GeoSourcePermissionFact();
            deserializedGeoSourcePermissionFact.FromMessage(message);

            Assert.AreEqual(deserializedGeoSourcePermissionFact.AuthorizedNode, geoSourcePermissionFact.AuthorizedNode);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.ResourceId, geoSourcePermissionFact.ResourceId);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.Limitations, geoSourcePermissionFact.Limitations);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.ProcRef, geoSourcePermissionFact.ProcRef);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.SerializedGeometry, String.Empty);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.UserResourceId, geoSourcePermissionFact.UserResourceId);
        }

        [Test]
        public void TestGeoSourcePermissionFactSerializationWithLimitations()
        {
            var geoSourcePermissionFact = CreateGeoSourcePermissionFact();
            geoSourcePermissionFact.Limitations = CreatePermissionLimitations();

            var message = geoSourcePermissionFact.ToMessage();

            var deserializedGeoSourcePermissionFact = new GeoSourcePermissionFact();
            deserializedGeoSourcePermissionFact.FromMessage(message);

            Assert.AreEqual(deserializedGeoSourcePermissionFact.AuthorizedNode, geoSourcePermissionFact.AuthorizedNode);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.ResourceId, geoSourcePermissionFact.ResourceId);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.Limitations.Area, geoSourcePermissionFact.Limitations.Area);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.Limitations.Resolution, geoSourcePermissionFact.Limitations.Resolution);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.Limitations.Time, geoSourcePermissionFact.Limitations.Time);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.Limitations.Watermark, geoSourcePermissionFact.Limitations.Watermark);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.ProcRef, geoSourcePermissionFact.ProcRef);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.SerializedGeometry, String.Empty);
            Assert.AreEqual(deserializedGeoSourcePermissionFact.UserResourceId, geoSourcePermissionFact.UserResourceId);
        }

        private static GeoSourcePermissionFact CreateGeoSourcePermissionFact()
        {
            var resourceId = new ResourceId(Guid.Empty);
            var userResourceId = new ResourceId(new Guid(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
            var publicKey = new PublicKey(new byte[4]);
            var userId = new UserId(publicKey);
            var identity = new Guid(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2);
            var authorizedNode = new NodeId(identity);
            authorizedNode.UserId = userId;
            var procRef = "{1}[1]";
            var geoSourcePermissionFact = new GeoSourcePermissionFact(resourceId, userResourceId, authorizedNode, procRef, null,
                null);
            return geoSourcePermissionFact;
        }

        private static GeoSourcePermissionFact.PermissionLimitations CreatePermissionLimitations()
        {
            var limitation = new GeoSourcePermissionFact.PermissionLimitations
            {
                Area = "testArea",
                Resolution = 10,
                Time = new TimeSpan(1, 1, 1, 1),
                Watermark = "testWatermark"
            };
            return limitation;
        }
    }
}