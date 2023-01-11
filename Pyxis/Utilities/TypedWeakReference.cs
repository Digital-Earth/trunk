/******************************************************************************
TypedWeakReference.cs

begin      : March 5, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

namespace Pyxis.Utilities
{
    /// <summary>
    /// A more type-safe weak reference.
    /// </summary>
    /// <typeparam name="T">The target type.</typeparam>
    public class TypedWeakReference<T> where T : class
    {
        /// <summary>
        /// The underlying weak reference.
        /// </summary>
        private readonly System.WeakReference m_weakReference;

        /// <summary>
        /// The target of the reference.
        /// </summary>
        public T Target
        {
            get
            {
                return (T)m_weakReference.Target;
            }
        }

        /// <summary>
        /// A constructor for the reference.
        /// </summary>
        /// <param name="target">The target.</param>
        public TypedWeakReference(T target)
        {
            m_weakReference = new System.WeakReference(target);
        }

        /// <summary>
        /// An implicit conversion from the reference to the target.
        /// </summary>
        /// <param name="reference">The reference to convert from.</param>
        /// <returns>The target.</returns>
        public static implicit operator T(TypedWeakReference<T> reference)
        {
            if (null == reference)
            {
                return null;
            }
            return reference.Target;
        }

        /// <summary>
        /// An implicit conversion from an object to a reference.
        /// </summary>
        /// <param name="obj">The obj.</param>
        /// <returns></returns>
        public static implicit operator TypedWeakReference<T>(T obj)
        {
            return new TypedWeakReference<T>(obj);
        }
    }
}
