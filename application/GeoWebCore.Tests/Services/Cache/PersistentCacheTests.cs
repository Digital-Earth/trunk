using System;
using GeoWebCore.Services.Cache;
using Moq;
using Newtonsoft.Json;
using NUnit.Framework;

namespace GeoWebCore.Tests.Services.Cache
{
    [TestFixture]
    internal class PersistentCacheTests
    {
        [Test]
        public void AddWithSmallValuesAreNotStored()
        {
            var storage = new Mock<ICacheStorage>();
            var cache = new PersistentCache<string>(storage.Object, 100, 20);

            cache.Add("hello");

            storage.Verify(s => s.Has("hello"),Times.Never);
        }

        [Test]
        public void AddWithBigValuesAreStored()
        {
            var storage = new Mock<ICacheStorage>();

            const string longString = "very very very very very very very very very long string (that is more than 20 chars)";
            const string hash = "87zCcZYCEhwBDOB3qkI9weKRHwWQiOBE8RTAjdfMfs8";

            storage.Setup(s => s.Has(It.IsAny<string>())).Returns(false);
            storage.Setup(s => s.Write(It.IsAny<string>(), It.IsAny<string>()));

            var cache = new PersistentCache<string>(storage.Object, 100, 20);

            cache.Add(longString);

            storage.Verify(s => s.Has(It.IsAny<string>()), Times.Once);
            storage.Verify(s => s.Write(It.IsAny<string>(), It.IsAny<string>()), Times.Once);
        }

        [Test]
        public void GetShortValuesWorkWithoutStorage()
        {
            var storage = new Mock<ICacheStorage>();

            storage.Setup(s => s.Has(It.IsAny<string>())).Returns(false);
            
            var cache = new PersistentCache<string>(storage.Object, 100, 20);

            var value = cache.Get("\"hello\"");

            storage.Verify(s => s.Has(It.IsAny<string>()), Times.Never);
            Assert.AreEqual(value,"hello");
        }

        [Test]
        public void GetBigValuesWillFailIfStroageHasNoValue()
        {
            var storage = new Mock<ICacheStorage>();

            storage.Setup(s => s.Has(It.IsAny<string>())).Returns(false);

            var cache = new PersistentCache<string>(storage.Object, 100, 20);

            const string hash = "87zCcZYCEhwBDOB3qkI9weKRHwWQiOBE8RTAjdfMfs8";

            Assert.Throws<Exception>(()=>cache.Get(hash));
            storage.Verify(s => s.Has(hash), Times.Once);
        }

        [Test]
        public void BigValuesAreStoredInMemoryCacheAndUseLocalStorageOnlyOnce()
        {
            var storage = new Mock<ICacheStorage>();

            const string longString = "very very very very very very very very very long string (that is more than 20 chars)";
            const string hash = "87zCcZYCEhwBDOB3qkI9weKRHwWQiOBE8RTAjdfMfs8";

            storage.Setup(s => s.Has(It.IsAny<string>())).Returns(false);
            storage.Setup(s => s.Has(hash)).Returns(true);
            storage.Setup(s => s.Read(hash)).Returns(() => JsonConvert.SerializeObject(longString));

            var cache = new PersistentCache<string>(storage.Object, 100, 20);

            var value = cache.Get(hash);

            Assert.AreEqual(value,longString);
            storage.Verify(s => s.Has(hash), Times.Once);
            storage.Verify(s => s.Read(hash), Times.Once);

            //try to get value second time will not cause new invokes to cache storage
            var value2 = cache.Get(hash);

            Assert.AreEqual(value2, longString);
            storage.Verify(s => s.Has(hash), Times.Once);
            storage.Verify(s => s.Read(hash), Times.Once);
        }
    }
}
