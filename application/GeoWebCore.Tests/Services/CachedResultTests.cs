using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Services;
using NUnit.Framework;

namespace GeoWebCore.Tests.Services
{
    [TestFixture]
    internal class CachedResultTests
    {
        [Test]
        public void CachedResultCacheValidResult()
        {
            var counter = 0;
            var cachedResult = CachedResult<string>.CreateNotNull(() =>
            {
                counter++;
                return "generated";
            });

            Assert.AreEqual(0,counter);
            Assert.IsFalse(cachedResult.HasValue);

            var result = cachedResult.GetValue();

            Assert.AreEqual(1, counter);
            Assert.AreEqual("generated", result);
            Assert.IsTrue(cachedResult.HasValue);

            var result2 = cachedResult.GetValue();

            Assert.AreEqual(1, counter);
            Assert.AreEqual(result,result2);
        }

        [Test]
        public void CachedResultValidateValue()
        {
            var cachedResult = CachedResult<string>.CreateNotNull(() => null);
            Assert.Throws<AggregateException>(() => cachedResult.GetValue());
            Assert.IsFalse(cachedResult.HasValue);
            Assert.IsFalse(cachedResult.InProgress);
        }

        [Test]
        public void CachedResultAcceptCustomValidation()
        {
            var output = "dog";
            var cachedResult = new CachedResult<string>(() => Task.FromResult(output),value => value != "cat");

            Assert.AreEqual("dog", cachedResult.GetValue());
            Assert.IsTrue(cachedResult.HasValue);
            Assert.IsFalse(cachedResult.InProgress);

            cachedResult.Invalidate();

            output = "cat";
            Assert.Throws<AggregateException>(() => cachedResult.GetValue());
            Assert.IsFalse(cachedResult.HasValue);
            Assert.IsFalse(cachedResult.InProgress);
        }

        [Test]
        public void CachedResultHandleFaultyValidator()
        {
            var cachedResult = new CachedResult<string>(() => Task.FromResult("hi"), value =>
            {
                throw new Exception("faulty handler");
            });

            Assert.Throws<AggregateException>(() => cachedResult.GetValue());
            Assert.IsFalse(cachedResult.HasValue);
            Assert.IsFalse(cachedResult.InProgress);
        }

        [Test]
        public void InvalidateResetTheCachedValue()
        {
            var cachedResult = CachedResult<string>.CreateNotNull(() => "hello");

            Assert.IsFalse(cachedResult.HasValue);
            Assert.IsFalse(cachedResult.InProgress);
            Assert.AreEqual("hello",cachedResult.GetValue());
            Assert.IsTrue(cachedResult.HasValue);
            cachedResult.Invalidate();
            Assert.IsFalse(cachedResult.HasValue);
        }

        [Test]
        public void InvalidateResetDependentItems()
        {
            var cachedResult = CachedResult<string>.CreateNotNull(() => "hello");
            var dependResult = CachedResult<string>.CreateNotNull(() => cachedResult.GetValue() + " world").DependsOn(cachedResult);

            Assert.AreEqual("hello world", dependResult.GetValue());
            Assert.IsTrue(cachedResult.HasValue);
            Assert.IsTrue(dependResult.HasValue);

            cachedResult.Invalidate();

            Assert.IsFalse(cachedResult.HasValue);
            Assert.IsFalse(dependResult.HasValue);
        }

        [Test]
        public void InProgressIsTureWhileGeneratingValue()
        {
            CachedResult<string> cachedResult = null;

            cachedResult = CachedResult<string>.CreateNotNull(() =>
            {
                Assert.IsTrue(cachedResult.InProgress);
                return "generated";
            });

            //before calling generator function
            Assert.IsFalse(cachedResult.InProgress);

            //generate value
            Assert.AreEqual("generated",cachedResult.GetTask().Result);

            //after
            Assert.IsFalse(cachedResult.InProgress);
        }
    }
}
