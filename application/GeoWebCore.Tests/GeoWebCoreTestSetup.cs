using NUnit.Framework;
using Pyxis.Core;

namespace GeoWebCore.Tests
{
    [SetUpFixture]
    public class GeoWebCoreTestSetup
    {
        private Engine m_engine;

        [SetUp]
        public void SetupEngine()
        {
            //TODO: to export GeoWebSetup code here.
            var config = EngineConfig.WorldViewDefault;

            m_engine = Engine.Create(config);
            m_engine.Start();
        }

        [TearDown]
        public void DestroyEngine()
        {
            m_engine.Stop();
        }
    }
}
