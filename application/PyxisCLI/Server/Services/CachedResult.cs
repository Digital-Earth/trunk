using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace PyxisCLI.Server.Services
{
    /// <summary>
    /// Basic items for CachedItem
    /// </summary>
    internal interface ICachedResult
    {
        /// <summary>
        /// Return true if value is been generated
        /// </summary>
        bool InProgress { get; }

        /// <summary>
        /// Return true if value has been generated
        /// </summary>
        bool HasValue { get; }

        /// <summary>
        /// Invalidate the current stored value
        /// </summary>
        void Invalidate();

        /// <summary>
        /// Mark depented item to be invalidated if this item been invalidated.
        /// </summary>
        /// <param name="item">dependent item</param>
        void AddDependentItem(ICachedResult item);
    }

    /// <summary>
    /// Helper class around Task object to store a task and validate the output for faster recalculations
    /// </summary>
    /// <typeparam name="T">Result type of the Task</typeparam>
    internal class CachedResult<T> : ICachedResult
    {
        /// <summary>
        /// Simple validator that check if the result is not null
        /// </summary>
        /// <param name="result">result of the task</param>
        /// <returns>return true if result is not null</returns>
        public static bool ValidateNotNull(T result)
        {
            return result != null;
        }

        private readonly Func<Task<T>> m_generator;
        private readonly Func<T, bool> m_validator;
        private readonly object m_lock = new object();
        private Task<T> m_task;

        private List<ICachedResult> m_dependentItems;

        /// <summary>
        /// Create a Cached result of type T.
        /// </summary>
        /// <param name="generator">Generator function, this function return a Task object to wait around</param>
        /// <param name="validator">Optional validator, if validator is provided, the CachedResult will only store the values that passed validation</param>
        public CachedResult(Func<Task<T>> generator, Func<T, bool> validator = null)
        {
            m_generator = generator;
            m_validator = validator;
        }

        /// <summary>
        /// Created cached result that store only results values that are not null
        /// </summary>
        /// <param name="generator">Generator function, this function return a value</param>
        /// <returns>CachedResult object to fetch values from</returns>
        public static CachedResult<T> CreateNotNull(Func<T> generator)
        {
            return new CachedResult<T>(() => Task<T>.Factory.StartNew(generator), ValidateNotNull);
        }

        /// <summary>
        /// Created cached result that store only results values that are not null 
        /// </summary>
        /// <param name="generator">Generator function, this function return a Task object to wait around</param>
        /// <returns>CachedResult object to fetch values from</returns>
        public static CachedResult<T> CreateNotNullAsync(Func<Task<T>> generator)
        {
            return new CachedResult<T>(generator, ValidateNotNull);
        }

        /// <summary>
        /// Get the Task for generating the value. this function will invoke value generation if needed or return cached value.
        /// </summary>
        /// <returns>Value generator task that can be awaited for</returns>
        public Task<T> GetTask()
        {
            lock (m_lock)
            {
                if (m_task == null)
                {
                    m_task = m_generator().ContinueWith<T>(ValidateState);
                }
            }
            return m_task;
        }

        /// <summary>
        /// Try to fetch a value from CachedResult. this function will invoke value generation if needed or return cached value.
        /// </summary>
        /// <returns>Value generator or an exception if value generation failed</returns>
        public T GetValue()
        {
            return GetTask().Result;
        }

        public bool InProgress
        {
            get
            {
                lock (m_lock)
                {
                    return m_task != null && !m_task.IsCompleted;
                }
            }
        }

        /// <summary>
        /// Return true if task has value generated
        /// </summary>
        public bool HasValue
        {
            get
            {
                lock (m_lock)
                {
                    return m_task != null && m_task.IsCompleted;
                }
            }
        }

        /// <summary>
        /// Add a depent Cached Item that will invalidated when this item is been invalidated
        /// </summary>
        /// <param name="dependent"></param>
        public void AddDependentItem(ICachedResult dependent)
        {
            lock (m_lock)
            {
                if (m_dependentItems == null)
                {
                    m_dependentItems = new List<ICachedResult>();
                }
                m_dependentItems.Add(dependent);
            }
        }

        /// <summary>
        /// Invalidate the cached value
        /// </summary>
        public void Invalidate()
        {
            lock (m_lock)
            {
                if (m_task == null)
                {
                    return;
                }

                m_task = null;
                if (m_dependentItems != null)
                {
                    m_dependentItems.ForEach(item => item.Invalidate());
                }
            }
        }

        private T ValidateState(Task<T> task)
        {
            var validResult = true;
            Exception errorException = null;

            try
            {
                if (task.IsFaulted)
                {
                    errorException = task.Exception;
                    validResult = false;
                }
                else if (m_validator != null && !m_validator(task.Result))
                {
                    validResult = false;
                }
            }
            catch (Exception e)
            {
                errorException = e;
                validResult = false;
            }

            if (validResult)
            {
                return task.Result;
            }
            
            //task result are not valid, thorw an error and delete state
            lock (m_lock)
            {
                //validate that m_task pointing to this task
                if (m_task != null && Task.CurrentId == m_task.Id)
                {
                    m_task = null;
                }
            }
            throw new Exception("CachedResult generation failed", errorException);
        }

        /// <summary>
        /// Mark this CachedResult to depend on root CachedResult. invalidate the root cached item will cause invalidation of all dependent children
        /// </summary>
        /// <param name="root">Root cached item to be depent on</param>
        /// <returns>this object</returns>
        public CachedResult<T> DependsOn(ICachedResult root)
        {
            root.AddDependentItem(this);
            return this;
        }
    }
}