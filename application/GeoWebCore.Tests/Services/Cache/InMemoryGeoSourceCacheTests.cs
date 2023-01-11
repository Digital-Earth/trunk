using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using GeoWebCore.Services.Cache;
using NUnit.Framework;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.Tests.Services.Cache
{
    [TestFixture]
    internal class InMemoryGeoSourceCacheTests
    {
        private Guid m_guid1 = Guid.NewGuid();
        private Guid m_guid2 = Guid.NewGuid();

        private class SimpleResolver
        {
            public int CallCount = 0;
            public int ResolvedCount = 0;
            private readonly Dictionary<Guid,GeoSource> Entries = new Dictionary<Guid, GeoSource>();

            public GeoSource Resolve(Guid id)
            {
                CallCount++;
                if (!Entries.ContainsKey(id))
                {
                    return null;
                }
                ResolvedCount++;
                return Entries[id];
            }

            public SimpleResolver(params Guid[] guids)
            {
                foreach (var guid in guids)
                {
                    Entries[guid] = new GeoSource() { Id = guid };
                }
            }
        }

        private static GeoSource ResolveNothing(Guid id)
        {
            return null;
        }

        private static GeoSource ResolveWithError(Guid id)
        {
            throw new NotImplementedException();
        }

        [Test]
        public void GeoSourceCacheReturnNullIfFailedToResolve()
        {
            var cache = new InMemoryGeoSourceCache();
            cache.AssignResolver(ResolveNothing);

            Assert.IsNull(cache.GetGeoSource(Guid.NewGuid()));
        }

        [Test]
        public void GeoSourceCacheReturnNullIfResolveThrowException()
        {
            var cache = new InMemoryGeoSourceCache();
            cache.AssignResolver(ResolveWithError);

            Assert.IsNull(cache.GetGeoSource(Guid.NewGuid()));
        }

        [Test]
        public void ResolversGetCalledInOrderWithExceptions()
        {
            var cache = new InMemoryGeoSourceCache();

            var resolver1 = new SimpleResolver(m_guid1);

            cache.AssignResolver(ResolveWithError);
            cache.AssignResolver(resolver1.Resolve);

            //guid1 will be resolved by resolver1 only
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);
        }

        [Test]
        public void ResolversGetCalledInOrder()
        {
            var cache = new InMemoryGeoSourceCache();

            var resolver1 = new SimpleResolver(m_guid1);
            var resolver2 = new SimpleResolver(m_guid2);
            
            cache.AssignResolver(resolver1.Resolve);
            cache.AssignResolver(resolver2.Resolve);

            //guid1 will be resolved by resolver1 only
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);
            Assert.AreEqual(resolver2.CallCount, 0);

            //gui21 will be resolved by resolver1 (and fail) and then by resolved2
            Assert.IsNotNull(cache.GetGeoSource(m_guid2));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 1);
            Assert.AreEqual(resolver2.CallCount, 1);
            Assert.AreEqual(resolver2.ResolvedCount, 1);
        }

        [Test]
        public void CacheKeepEntriesForTimeToLive()
        {
            var cache = new InMemoryGeoSourceCache(TimeSpan.FromSeconds(0.1), TimeSpan.FromSeconds(0.1));
            var resolver1 = new SimpleResolver(m_guid1);

            cache.AssignResolver(resolver1.Resolve);

            //guid1 will be resolved by resolver1 only
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            //second call will not invoke resolver as it in cache
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            //sleep until entry will be invalidated
            Thread.Sleep(TimeSpan.FromSeconds(0.2));

            //guid1 will be resolved by resolver1 only
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 2);
        }

        [Test]
        public void FailedEntriesHaveDifferentTimeToLive()
        {
            var cache = new InMemoryGeoSourceCache(TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(0.1));
            var resolver1 = new SimpleResolver(m_guid1);

            cache.AssignResolver(resolver1.Resolve);

            //guild1 will work, guid2 will fail
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.IsNull(cache.GetGeoSource(m_guid2));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            //sleep until entry will be invalidated
            Thread.Sleep(TimeSpan.FromSeconds(0.2));

            //guid1 still in cahce
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            //guid2 is no longer in cache and will invoke resolve again
            Assert.IsNull(cache.GetGeoSource(m_guid2));
            Assert.AreEqual(resolver1.CallCount, 3);
            Assert.AreEqual(resolver1.ResolvedCount, 1);
        }

        [Test]
        public void InvalidateGeoSourceRemoveEntryFromCache()
        {
            var cache = new InMemoryGeoSourceCache();
            var resolver1 = new SimpleResolver(m_guid1);

            cache.AssignResolver(resolver1.Resolve);

            //invoke first resolve
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            Assert.IsTrue(cache.InvalidateGeoSource(m_guid1));

            //invoke second resolve
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 2);
        }

        [Test]
        public void InvalidateWithNewEntryWillNotRemoveEntry()
        {
            var cache = new InMemoryGeoSourceCache();
            var resolver1 = new SimpleResolver(m_guid1);

            cache.AssignResolver(resolver1.Resolve);

            //invoke first resolve
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            Assert.IsFalse(cache.InvalidateGeoSource(m_guid1, TimeSpan.FromMinutes(1)));

            //entry should not be reomved
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 1);
            Assert.AreEqual(resolver1.ResolvedCount, 1);

            //let entry get old
            Thread.Sleep(TimeSpan.FromMilliseconds(10));
            Assert.IsTrue(cache.InvalidateGeoSource(m_guid1, TimeSpan.FromMilliseconds(1)));

            //invoke second resolve
            Assert.IsNotNull(cache.GetGeoSource(m_guid1));
            Assert.AreEqual(resolver1.CallCount, 2);
            Assert.AreEqual(resolver1.ResolvedCount, 2);
        }
    }
}
