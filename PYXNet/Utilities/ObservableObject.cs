/******************************************************************************
ObservableObject.cs

begin      : May 23, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
{
    /// <summary>
    /// EventArgs for a Changed event.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class ChangedEventArgs<T> : EventArgs
    {
        /// <summary>The NewValue.</summary>
        public T NewValue
        {
            get 
            { 
                return m_newValue; 
            }
            set 
            { 
                m_newValue = value; 
            }
        }
        private T m_newValue;

        internal ChangedEventArgs(T theNewValue)
        {
            m_newValue = theNewValue;
        }
    }
    
    /// <summary>
    /// Generic wrapper provides a value that sends a Changed event whenever
    /// it is changed.
    /// </summary>
    /// <typeparam name="T">
    /// This must be a value type; state changes of a non-value type 
    /// would not be detected by this observer, as it only intercepts 
    /// the setter.
    /// </typeparam>
    public class ObservableObject<T> where T : struct
    {
        /// <summary>
        /// Gets or sets the value.
        /// </summary>
        /// <value>The value.</value>
        public T Value
        {
            get { return m_value; }
            set
            {
                if (m_value.Equals(value))
                {
                    return;
                }

                // TODO: Consider adding Changing event as well....
                //this.OnChanging( this, value);

                m_value = value;
                this.OnChanged(this, value);
            }
        }
        private T m_value;

        /// <summary>
        /// Initializes a new instance of the <see cref="ObservableObject&lt;T&gt;"/> class.
        /// </summary>
        /// <param name="initialValue">The initial value.</param>
        public ObservableObject(T initialValue)
        {
            this.m_value = initialValue;
        }
        
        #region Changed Event

        /// <summary> Event handler for Changed. </summary>
        public event EventHandler<ChangedEventArgs<T> > Changed;

        /// <summary>
        /// Raises the Changed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theNewValue"></param>
        private void OnChanged(object sender, T theNewValue)
        {
            EventHandler<ChangedEventArgs<T> > handler = Changed;
            if (handler != null)
            {
                ChangedEventArgs<T> args = 
                    new ChangedEventArgs<T>(theNewValue);
                handler(sender, args);
            }
        }

        #endregion
    }

    /// <summary>
    /// A wrapper for a String value that sends a Changed event whenever
    /// it is changed.
    /// </summary>
    public class ObservableString
    {
        /// <summary>
        /// Gets or sets the value.
        /// </summary>
        /// <value>The value.</value>
        public string Value
        {
            get { return m_value; }
            set
            {
                if (m_value.Equals(value))
                {
                    return;
                }

                // TODO: Consider adding Changing event as well....
                //this.OnChanging( this, value);

                m_value = value;
                this.OnChanged(this, value);
            }
        }
        private string m_value;

        /// <summary>
        /// Initializes a new instance of the <see cref="ObservableString&lt;T&gt;"/> class.
        /// </summary>
        /// <param name="initialValue">The initial value.</param>
        public ObservableString(string initialValue)
        {
            this.m_value = initialValue;
        }

        #region Changed Event

        /// <summary> Event handler for Changed. </summary>
        public event EventHandler<ChangedEventArgs<string>> Changed;

        /// <summary>
        /// Raises the Changed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theNewValue"></param>
        private void OnChanged(object sender, string theNewValue)
        {
            EventHandler<ChangedEventArgs<string>> handler = Changed;
            if (handler != null)
            {
                ChangedEventArgs<string> args =
                    new ChangedEventArgs<string>(theNewValue);
                handler(sender, args);
            }
        }

        #endregion
    }

    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for ObservableObject
        /// </summary>
        [TestFixture]
        public class ObservableObjectTests
        {
            [Test]
            public void Integer()
            {
                bool callbackHappened = false;

                ObservableObject<int> myObject = new ObservableObject<int>(13);
                myObject.Changed +=
                    delegate(object sender, ChangedEventArgs<int> e)
                    {
                        Assert.AreEqual(42, e.NewValue);
                        callbackHappened = true;
                    };
                myObject.Value = 42;
                Assert.IsTrue(callbackHappened);
            }
        }
    }
}
