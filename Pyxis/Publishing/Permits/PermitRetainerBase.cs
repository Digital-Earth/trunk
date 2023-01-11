using Pyxis.Contract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing.Permits
{
    /// <summary>
    /// helper class for implementing PermitRetainers by implementing 2 abstract function: DoGetPermit and DoReleasePermit.
    /// This class would take care of thread sync and validating the permit has not expired.
    /// </summary>
    /// <typeparam name="T">IPermit class for this PermitRetainer to retain</typeparam>
    internal abstract class PermitRetainerBase<T> : IPermitRetainer<T> where T : class, IPermit
    {
        private object m_lock = new object();
        private T m_currentPermit = null;

        public T GetPermit()
        {
            lock (m_lock)
            {
                if (m_currentPermit == null || m_currentPermit.Expires.AddMinutes(1) > DateTime.UtcNow)
                {
                    m_currentPermit = DoGetPermit();
                }
                return m_currentPermit;
            }
        }

        public void ReleasePermit()
        {
            lock (m_lock)
            {
                if (m_currentPermit != null)
                {
                    DoReleasePermit(m_currentPermit);
                    m_currentPermit = null;
                }
            }
        }

        public void Dispose()
        {
            ReleasePermit();
        }

        abstract protected T DoGetPermit();
        abstract protected void DoReleasePermit(T permit);
    }
}
