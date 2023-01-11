/******************************************************************************
ThreadSafeInt.cs

begin      : Auguest 15, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    public class ThreadSafeInt
    {
        private int m_value = 0;

        /// <summary>
        /// Initializes a new instance of the <see cref="ThreadSafeInt"/> class, initialized to 0.
        /// </summary>
        public ThreadSafeInt()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ThreadSafeInt"/> class.
        /// </summary>
        /// <param name="initialValue">The initial value.</param>
        public ThreadSafeInt(int initialValue)
        {
            m_value = initialValue;
        }

        /// <summary>
        /// Gets or sets the value.
        /// </summary>
        /// <value>The value.</value>
        public int Value
        {
            get { return m_value; }
            set
            {
                System.Threading.Interlocked.Exchange(ref m_value, value);
            }
        }

        /// <summary>
        /// Returns a <see cref="T:System.String"/> that represents the value of this <see cref="T:ThreadSafeInt"/>.
        /// </summary>
        /// <returns>
        /// A <see cref="T:System.String"/> that represents the current <see cref="T:ThreadSafeInt"/>.
        /// </returns>
        public override string ToString()
        {
            return Value.ToString();
        }

        /// <summary>
        /// Decrements the specified value.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        public static ThreadSafeInt operator --(ThreadSafeInt value)
        {
            System.Threading.Interlocked.Decrement(ref value.m_value);
            return value;
        }

        /// <summary>
        /// Increments the specified value.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        public static ThreadSafeInt operator ++(ThreadSafeInt value)
        {
            System.Threading.Interlocked.Increment(ref value.m_value);
            return value;
        }

        /// <summary>
        /// Implements the operator +.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="increment">The increment.</param>
        /// <returns>The result of the operator.</returns>
        public static ThreadSafeInt operator +(ThreadSafeInt value, int increment)
        {
            System.Threading.Interlocked.Add(ref value.m_value, increment);
            return value;
        }

        /// <summary>
        /// Implements the operator -.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="decrement">The decrement.</param>
        /// <returns>The result of the operator.</returns>
        public static ThreadSafeInt operator -(ThreadSafeInt value, int decrement)
        {
            System.Threading.Interlocked.Add(ref value.m_value, -decrement);
            return value;
        }

        /// <summary>
        /// Converts the specified value to an integer.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        public static implicit operator int(ThreadSafeInt value)
        {
            return value.Value;
        }

        /// <summary>
        /// Converts an int to a new ThreadSafeInt.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        public static implicit operator ThreadSafeInt(int value)
        {
            return new ThreadSafeInt(value);
        }
    }
}
