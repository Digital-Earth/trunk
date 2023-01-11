using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;

namespace Pyxis.Core.Test
{
    [SetUpFixture]
    public class EngineTestSetup
    {
        private Engine m_engine;

        [SetUp]
        public void SetupEngine()
        {
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
