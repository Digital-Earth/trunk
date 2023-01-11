/******************************************************************************
ObservableObject.cs

begin      : May 23, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// EventArgs for a Changed event.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class ChangedEventArgs<T> : EventArgs
    {
        /// <summary>The NewValue.</summary>
        public T NewValue { get; private set;}

        /// <summary>
        /// Gets or sets the old value.
        /// </summary>
        /// <value>The old value.</value>
        public T OldValue { get; private set; }

        internal ChangedEventArgs(T theOldValue, T theNewValue)
        {
            NewValue = theNewValue;
            OldValue = theOldValue;
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

                T oldValue = m_value;
                m_value = value;
                this.OnChanged(this, oldValue, value);
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

        private EventHelper<ChangedEventArgs<T>> m_Changed = new EventHelper<ChangedEventArgs<T>>();
        /// <summary> Event handler for Changed. </summary>
        public event EventHandler<ChangedEventArgs<T>> Changed
        {
            add
            {
                m_Changed.Add(value);
            }
            remove
            {
                m_Changed.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Changed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theNewValue"></param>
        private void OnChanged(object sender, T theOldValue, T theNewValue)
        {
            ChangedEventArgs<T> args = 
                new ChangedEventArgs<T>(theOldValue, theNewValue);
                
            m_Changed.Invoke( sender, args);
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

                string oldValue = m_value;
                m_value = value;
                this.OnChanged(this, oldValue, value);
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

        private EventHelper<ChangedEventArgs<string>> m_Changed = new EventHelper<ChangedEventArgs<string>>();

        /// <summary> Event handler for Changed. </summary>
        public event EventHandler<ChangedEventArgs<string>> Changed
        {
            add
            {
                m_Changed.Add(value);
            }
            remove
            {
                m_Changed.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Changed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theNewValue"></param>
        private void OnChanged(object sender, string theOldValue, string theNewValue)
        {
            ChangedEventArgs<string> args =
                new ChangedEventArgs<string>(theOldValue, theNewValue);
            m_Changed.Invoke(sender, args);
        }

        #endregion
    }

    public static class ObservableObjectExtensions
    {
        public static void ValidatedTransition<T>(this ObservableObject<T> state, T sourceState, Dictionary<T, List<T>> stateTransitions) where T : struct
        {
            if (!stateTransitions[state.Value].Contains(sourceState))
            {
                System.Diagnostics.Trace.WriteLine(String.Format("Attempt to transition to state {0} from invalid state {1}. Allowed states are {2}",
                                                                 state.Value, sourceState, String.Join(", ", stateTransitions[state.Value])));
            }
        }
    }
}
