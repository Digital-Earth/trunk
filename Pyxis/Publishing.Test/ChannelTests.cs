using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Moq;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing;
using Pyxis.Publishing.Protocol;

namespace Pyxis.Publishing.Test
{
    [TestFixture]
    class ChannelTests
    {
        [Test]
        public void TestChannelGeoSourceFirst()
        {
            var resourceMock = new Mock<IResourcesClient<GeoSource>>();
            var lsMock = new Mock<ILicenseServerClient>();
            lsMock.Setup(x=>x.GeoSources).Returns(resourceMock.Object);

            ResourceFilter filter = null;

            resourceMock.Setup(x => x.GetResources(It.IsAny<ResourceFilter>()))
                .Callback((ResourceFilter f) => { filter = f; })
                .Returns(new List<GeoSource>() { new GeoSource() { Id = Guid.NewGuid() } });

            var channel = new Channel(lsMock.Object);

            var geoSource = channel.GeoSources.First();

            resourceMock.Verify(x=> x.GetResources(It.IsAny<ResourceFilter>()), Times.AtLeastOnce());

            Assert.AreEqual(0, filter.Skip);
            Assert.AreEqual(1, filter.Top);
            Assert.IsNull(filter.Search);
            Assert.IsNull(filter.Filter);
            Assert.IsNull(filter.Select);
            Assert.IsNotNull(geoSource);
            Assert.IsNotNull(geoSource.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource);
        }

        [Test]
        public void TestChannelGeoSourceSearch()
        {
            var resourceMock = new Mock<IResourcesClient<GeoSource>>();
            var lsMock = new Mock<ILicenseServerClient>();
            lsMock.Setup(x => x.GeoSources).Returns(resourceMock.Object);

            ResourceFilter filter1 = null;
            ResourceFilter filter2 = null;

            var GeoSource1 = new GeoSource() { Id = Guid.NewGuid() };
            var GeoSource2 = new GeoSource() { Id = Guid.NewGuid() };

            resourceMock.Setup(x => x.GetResources(It.Is<ResourceFilter>(f=>f.Skip == 0)))
                .Callback((ResourceFilter f) => { filter1 = f; })
                .Returns(new List<GeoSource>() { GeoSource1, GeoSource2 });

            resourceMock.Setup(x => x.GetResources(It.Is<ResourceFilter>(f => f.Skip != 0)))
                .Callback((ResourceFilter f) => { filter2 = f; })
                .Returns(new List<GeoSource>() {});

            var channel = new Channel(lsMock.Object);


            var geoSources = channel.GeoSources.Search("Hello");

            Assert.AreEqual(2, geoSources.Count());

            resourceMock.Verify(x => x.GetResources(It.IsAny<ResourceFilter>()), Times.Exactly(2));

            Assert.AreEqual(0, filter1.Skip);
            Assert.Greater(filter1.Top, 1);
            Assert.AreEqual("Hello",filter1.Search);

            Assert.AreEqual(2, filter2.Skip);
            Assert.Greater(filter2.Top, 1);
            Assert.AreEqual("Hello", filter2.Search);
        }
    }
}
