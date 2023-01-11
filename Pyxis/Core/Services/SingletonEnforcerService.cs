using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Services
{
    class SingletonEnforcerService : ServiceBase
    {
        private static object m_singletonServiceLock = new object();
        private static SingletonEnforcerService m_singletonService = null;

        protected override void StartService()
        {
            lock (m_singletonServiceLock)
            {
                if (m_singletonService == null)
                {
                    m_singletonService = this;
                }
                else
                {
                    throw new Exception("There can only one Pyxis.Core.Engine per process");
                }
            }
        }

        protected override void StopService()
        {
            lock (m_singletonServiceLock)
            {
                if (m_singletonService == this)
                {
                    m_singletonService = null;
                }
            }
        }
    }
}
