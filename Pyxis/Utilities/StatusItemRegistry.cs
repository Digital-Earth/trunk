/******************************************************************************
StatusItemRegistry.cs

begin      : January 28, 2008
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Maintains a registry of status items.  Multiple registries can exist 
    /// in the application.
    /// </summary>
    public class StatusItemRegistry
    {
        /// <summary>
        /// A list of status items registered with the registry.
        /// </summary>
        private List<StatusItem> m_items = new List<StatusItem>();

        /// <summary>
        /// Default Constructor.
        /// </summary>
        public StatusItemRegistry()
        {            
        }

        /// <summary>
        /// Register a StatusItem with the registry.
        /// </summary>
        /// <param name="item">
        /// The StatusItem to register.
        /// </param>
        /// <exception cref="ArgumentException">
        /// This is thrown when the StatusItem provided doesn't follow the 
        /// following rules: 
        /// 1. Has a different category than all other registry StatusItems 
        /// AND a unique name. 
        /// 2. Belongs to an existing category and has a different priority 
        /// than all other registry StatusItems of that category AND a unique 
        /// name.
        /// </exception>
        public void Register(StatusItem item)
        {
            foreach (StatusItem registryItem in m_items)
            {
                if ((registryItem.Category == item.Category) &&
                    (registryItem.Priority == item.Priority))
                {
                    throw new ArgumentException(string.Format(
                        "There already exists a StatusItem '{0}' of the same " 
                        + "category and priority in the registry.", 
                        registryItem.Name));
                }
                else if(registryItem.Name == item.Name)
                {
                    throw new ArgumentException(string.Format(
                        "There already exists a StatusItem with name '{0}' " + 
                        "in the registry.", item.Name));
                }
            }

            m_items.Add(item);
        }

        /// <summary>
        /// Unregister all status items from the registry.
        /// </summary>        
        public void UnregisterAll()
        {
            foreach (StatusItem item in m_items)
            {
                item.Dispose();
            }

            m_items.Clear();
        }

        /// <summary>
        /// Goes through all the status items in the registry and returns a 
        /// list of items, where each item in the list is the one with the 
        /// highest priority in its category.
        /// </summary>
        /// <returns>List of status items.</returns>
        public IList<StatusItem> GetStatusItems()
        {
            List<StatusItem> returnItems = new List<StatusItem>();
            
            Dictionary<string, SortedList<int, StatusItem>> itemList =
                new Dictionary<string, SortedList<int, StatusItem>>();

            foreach (StatusItem item in m_items)
            {
                if (itemList.ContainsKey(item.Category))
                {
                    itemList[item.Category].Add(item.Priority, item);
                }
                else
                {
                    SortedList<int, StatusItem> newList =
                        new SortedList<int, StatusItem>();
                    newList.Add(item.Priority, item);
                    itemList.Add(item.Category, newList);
                }
            }

            foreach (SortedList<int, StatusItem> item in itemList.Values)
            {
                returnItems.Add(item.Values[item.Values.Count - 1]);
            }

            return returnItems;
        }

        /// <summary>
        /// Searches for a StatusItem by its name.
        /// </summary>
        /// <param name="name">The name to search by.</param>
        /// <returns>The matching StatusItem or null.</returns>
        public StatusItem Find(string name)
        {
            foreach (StatusItem item in m_items)
            {
                if (item.Name == name)
                {
                    return item;
                }
            }

            return null;
        }

        /// <summary>
        /// Called by a StatusItem when its value or priority changes.
        /// </summary>
        public void Changed(StatusItem item)
        {
            // call the method that will raise the event
            OnStatusItemChanged(item);
        }

        /// <summary>
        /// Raises the event, providing the changed status item as an argument.
        /// </summary>
        /// <param name="item">The status item that has changed.</param>
        private void OnStatusItemChanged(StatusItem item)
        {
            StatusItemChangedEventArgs e = new StatusItemChangedEventArgs(item);

            m_StatusItemChanged.Invoke(this, e);
        }

        private EventHelper<StatusItemChangedEventArgs> m_StatusItemChanged = new EventHelper<StatusItemChangedEventArgs>();

        /// <summary>
        /// Event declaration for the event that is raised when a status item 
        /// changes.
        /// </summary>
        public event EventHandler<StatusItemChangedEventArgs> StatusItemChanged
        {
            add
            {
                m_StatusItemChanged.Add(value);
            }
            remove
            {
                m_StatusItemChanged.Remove(value);
            }
        }
    }

    /// <summary>
    /// The argument to pass when a StatusItemChanged event is raised.
    /// </summary>
    public class StatusItemChangedEventArgs : EventArgs
    {
        /// <summary>
        /// Initializes the class with the StatusItem whose text/value/
        /// priority has changed.
        /// </summary>
        /// <param name="item">
        /// The status item to provide as an event argument.
        /// </param>
        public StatusItemChangedEventArgs(StatusItem item)
        {
            m_item = item;
        }

        /// <summary>
        /// The StatusItem to provide as an argument.
        /// </summary>
        public StatusItem StatusItem
        {
            get
            {
                return m_item;
            }
        }
        private StatusItem m_item;
    }
}
