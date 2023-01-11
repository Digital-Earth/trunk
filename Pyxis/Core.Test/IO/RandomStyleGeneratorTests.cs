using System;
using NUnit.Framework;
using Pyxis.Core.IO;

namespace Pyxis.Core.Test.IO
{
    [TestFixture]
    internal class RandomStyleGeneratorTests
    {
        [Test]
        public void RandomStyleIsSameForTheSameGuid()
        {
            var gtopo30Guid = Guid.Parse("8be6a2ec-5110-49cf-a295-1008f8e9a21b");

            var style1 = RandomStyleGenerator.Create(gtopo30Guid);
            var style2 = RandomStyleGenerator.Create(gtopo30Guid);

            Assert.IsTrue(style1.Icon.Color == style2.Icon.Color);
            Assert.AreEqual(style1.Icon.IconDataUrl, style2.Icon.IconDataUrl);
        }

        [Test]
        public void RandomStyleIsDifferentForDifferentGuid()
        {
            var guid1 = Guid.Parse("c5222ebc-750e-4afb-8df9-2ae45e08fb25");
            var guid2 = Guid.Parse("8be6a2ec-5110-49cf-a295-1008f8e9a21b");

            var style1 = RandomStyleGenerator.Create(guid1);
            var style2 = RandomStyleGenerator.Create(guid2);

            Assert.IsFalse(style1.Icon.Color == style2.Icon.Color);
            Assert.AreNotEqual(style1.Icon.IconDataUrl, style2.Icon.IconDataUrl);
        }
    }
}
