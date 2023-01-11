using System;
using System.Linq;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    [TestFixture]
    public class GlobalTypeFinderTests
    {
        [AttributeUsage(AttributeTargets.Class)]
        public class FakeGlobalAttribute : Attribute
        {
        }

        [FakeGlobal]
        private class FakeSettingsProvider1 { }

        [FakeGlobal]
        private class FakeSettingsProvider2 { }

        interface IFakeGlobalInterface
        {
        }

        private class FakeSettingsProvider3 : IFakeGlobalInterface
        {
        }

        private class FakeSettingsProvider4 : IFakeGlobalInterface
        {
        }


        // Find the FakeSettingsProviders
        [Test]
        public void TestFindAllAttributes()
        {
            var fakeSettingsProviders = GlobalTypeFinder.Attributes<FakeGlobalAttribute>.FindAll();

            Assert.AreEqual(2, fakeSettingsProviders.Count());
        }
        
           
        // Find the FakeSettingsProviders
        [Test]
        public void TestFindAllInterfaces()
        {
            var fakeInterfaceImplementors = GlobalTypeFinder.Interfaces<IFakeGlobalInterface>.FindAll();

            Assert.AreEqual(2, fakeInterfaceImplementors.Count());
        }
    }
}