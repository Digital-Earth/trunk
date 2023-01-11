using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    public class TemporaryObject<T>
    {
        private T m_value;
        private DateTime m_updated;
        private TimeSpan m_timeToLive;

        public TemporaryObject(T value, TimeSpan timeToLive)
        {
            m_value = value;
            m_updated = DateTime.Now;
            m_timeToLive = timeToLive;
        }

        public T Value
        {
            get
            {
                return m_value;
            }
            set
            {
                m_value = value;
                m_updated = DateTime.Now;
            }
        }

        public TimeSpan Age
        {
            get
            {
                return DateTime.Now - m_updated;
            }
        }

        public TimeSpan TimetoLive
        {
            get
            {
                return m_timeToLive;
            }
            set
            {
                m_timeToLive = value;
            }
        }

        public bool Expired
        {
            get
            {
                return DateTime.Now - m_updated > m_timeToLive;
            }
        }

        public bool TryGetValue(out T value)
        {
            if (Expired)
            {
                value = default(T);
                return false;
            }
            else
            {
                value = m_value;
                return true;
            }
        }
    }
}
