using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Publishing.Protocol;

namespace Pyxis.Publishing
{
    public class QueryableResources<T> : IEnumerable<T> where T : Pyxis.Contract.Publishing.Resource
    {
        private IResourcesClient<T> m_context;
        private ResourceFilter m_filter;

        public QueryableResources(IResourcesClient<T> context)
        {
            m_context = context;
            m_filter = new ResourceFilter()
            {
                Top = 0,
                Skip = 0,
                Search = null,
                Filter = null,
                Select = null
            };
        }

        private QueryableResources(IResourcesClient<T> context, ResourceFilter filter)
        {
            m_context = context;
            m_filter = filter;
        }

        public QueryableResources<T> Search(string search)
        {
            var filter = m_filter.Clone();
            filter.Search = search;
            return new QueryableResources<T>(m_context, filter);
        }

        public QueryableResources<T> Filter(string filter)
        {
            var newFilter = m_filter.Clone();
            newFilter.Filter = filter;
            return new QueryableResources<T>(m_context, newFilter);
        }

        public QueryableResources<T> Select(string select)
        {
            var filter = m_filter.Clone();
            filter.Select = select;
            return new QueryableResources<T>(m_context, filter);
        }

        public QueryableResources<T> Skip(int skip)
        {
            var filter = m_filter.Clone();
            filter.Skip += skip;
            return new QueryableResources<T>(m_context, filter);
        }

        public T First()
        {
            var filter = m_filter.Clone();
            filter.Top = 1;
            return (new QueryableResources<T>(m_context, filter)).First<T>();
        }

        public QueryableResources<T> Take(int amount)
        {
            var filter = m_filter.Clone();
            filter.Top = amount;
            return new QueryableResources<T>(m_context, filter);
        }

        public T GetById(Guid id)
        {
            return m_context.GetResourceById(id);
        }

        public IEnumerable<T> GetVersionsOfId(Guid id)
        {
            return m_context.GetResourceVersions(id);
        }

        public T GetByIdAndVersion(Guid id,Guid version)
        {
            return m_context.GetResourceByIdAndVerison(id, version);
        }

        private IEnumerable<T> ExecuteQuery()
        {
            var filter = m_filter.Clone();
            if (filter.Top == 0)
            {
                filter.Top = 50;
            }

            var count = 1;
            while (count > 0)
            {
                count = 0;
                foreach (var resource in m_context.GetResources(filter))
                {
                    count++;
                    yield return resource;
                }
                filter = filter.NextPage(count);
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            return ExecuteQuery().GetEnumerator();
        }
        
        IEnumerator IEnumerable.GetEnumerator()
        {
            return ExecuteQuery().GetEnumerator();
        }
    }
}
