using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore
{
    public interface IValueColumn : IEnumerable<object>
    {
        object Get(int index);
        IEnumerable<int> Find(object Value);
        IEnumerable<int> Find(object Value, IEnumerable<int> at);
    }

    public class SimpleValueColumn : IValueColumn
    {
        private object[] m_values;

        public SimpleValueColumn(IEnumerable<object> values)
        {
            m_values = values.ToArray();
        }

        public object Get(int index)
        {
            return m_values[index];
        }

        public IEnumerable<int> Find(object value)
        {
            if (value == null)
            {
                for (int i = 0; i < m_values.Length; i++)
                {
                    if (m_values[i] == null)
                    {
                        yield return i;
                    }
                }
            }
            else
            {
                for (int i = 0; i < m_values.Length; i++)
                {
                    if (value.Equals(m_values[i]))
                    {
                        yield return i;
                    }
                }
            }
        }

        public IEnumerable<int> Find(object value, IEnumerable<int> at)
        {
            if (value == null)
            {
                foreach (var index in at)
                {
                    if (m_values[index] == null)
                    {
                        yield return index;
                    }
                }
            }
            else
            {
                foreach (var index in at)
                {
                    if (value.Equals(m_values[index]))
                    {
                        yield return index;
                    }
                }
            }
        }

        public IEnumerator<object> GetEnumerator()
        {
            return m_values.AsEnumerable<object>().GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_values.AsEnumerable<object>().GetEnumerator();
        }
    }

    public class LookupValueColumn : IValueColumn
    {
        private int[] m_values;
        private object[] m_lookup;

        public LookupValueColumn(IEnumerable<object> values)
        {
            m_lookup = values.Distinct().OrderBy(x => x).ToArray();
            m_values = values.Select(x => FindValue(x)).ToArray();
        }

        private int FindValue(object value)
        {
            return Array.BinarySearch(m_lookup, value);
        }

        public object Get(int index)
        {
            return m_lookup[m_values[index]];
        }

        public IEnumerable<int> Find(object value)
        {
            var index = FindValue(value);
            if (index < 0)
            {
                yield break;
            }
            for (int i = 0; i < m_values.Length; i++)
            {
                if (m_values[i] == index)
                {
                    yield return i;
                }
            }
        }

        public IEnumerable<int> Find(object value, IEnumerable<int> at)
        {
            var valueIndex = FindValue(value);
            if (valueIndex < 0)
            {
                yield break;
            }
            foreach (var index in at)
            {
                if (m_values[index] == valueIndex)
                {
                    yield return index;
                }
            }
        }

        public IEnumerator<object> GetEnumerator()
        {
            return m_values.Select(x => m_lookup[x]).GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_values.Select(x => m_lookup[x]).GetEnumerator();
        }
    }

    public class SparseValueColumn : IValueColumn
    {
        private SortedDictionary<int, object> m_values;
        private int m_size;

        public SparseValueColumn(IEnumerable<object> values)
        {
            m_values = new SortedDictionary<int, object>();
            var index = 0;
            foreach (var value in values)
            {
                if (value != null)
                {
                    m_values[index] = value;
                }
                index++;
            }
            m_size = index;
        }

        public object Get(int index)
        {
            object value;
            if (m_values.TryGetValue(index, out value))
            {
                return value;
            }
            return null;
        }

        public IEnumerable<int> Find(object value)
        {
            if (value == null)
            {
                var index = 0;
                foreach (var keyValue in m_values)
                {
                    while (index < keyValue.Key)
                    {
                        yield return index;
                        index++;
                    }
                    //skip keyValue entry as value != null
                    index++;
                }

                while (index < m_size)
                {
                    yield return index;
                    index++;
                }
            }
            else
            {
                foreach (var keyValue in m_values)
                {
                    if (keyValue.Value.Equals(value))
                    {
                        yield return keyValue.Key;
                    }
                }
            }
        }

        public IEnumerable<int> Find(object value, IEnumerable<int> at)
        {
            if (value == null)
            {
                foreach (var index in at)
                {
                    if (!m_values.ContainsKey(index))
                    {
                        yield return index;
                    }
                }
            }
            else
            {
                foreach (var index in m_values.Keys.Intersect(at))
                {
                    object atValue;
                    if (m_values.TryGetValue(index, out atValue) && atValue.Equals(value))
                    {
                        yield return index;
                    }
                }
            }
        }

        public IEnumerator<object> GetEnumerator()
        {
            var index = 0;
            foreach (var keyValue in m_values)
            {
                while (index < keyValue.Key)
                {
                    yield return null;
                    index++;
                }
                yield return keyValue.Value;
                index++;
            }

            while (index < m_size)
            {
                yield return null;
                index++;
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            var index = 0;
            foreach (var keyValue in m_values)
            {
                while (index < keyValue.Key)
                {
                    yield return null;
                    index++;
                }
                yield return keyValue.Value;
                index++;
            }

            while (index < m_size)
            {
                yield return null;
                index++;
            }
        }
    }
}
