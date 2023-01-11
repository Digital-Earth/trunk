/******************************************************************************
StatusItem.cs

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
    /// The base class for all status items.
    /// </summary>
    public class StatusItem
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="registry">
        /// The registry the StatusItem will be registered with.
        /// </param>
        /// <param name="name">The name.</param>
        /// <param name="category">The category.</param>
        public StatusItem(StatusItemRegistry registry, 
            string name, 
            string category)
        {
            m_registry = registry;
            m_name = name;
            m_category = category;
        }

        /// <summary>
        /// Called explicitly by registry before unregistering a status item 
        /// to clean up any resources.
        /// </summary>
        public virtual void Dispose()
        {
        }

        /// <summary>
        /// The item's name (this must be unique among all registered 
        /// StatusItems), which is used as a key by the client when its value is updated.
        /// </summary>
        public string Name
        {
            get
            {
                return m_name;
            }
        }
        private string m_name;

        /// <summary>
        /// The item's category.  Multiple status items can belong to the same 
        /// category, but only the item with the highest priority in its 
        /// category will be displayed (not yet implemented and the algorithm 
        /// may change).
        /// </summary>
        public string Category
        {
            get
            {
                return m_category;
            }
        }
        private string m_category;

        /// <summary>
        /// The item's priority.  The higher the priority, the more important 
        /// the status item is within its category.  When the item's priority 
        /// changes, it informs the registry it is registered with.
        /// </summary>
        public int Priority
        {
            get
            {
                return m_priority;
            }
            set
            {
                m_priority = value;
                OnChanged();
            }
        }
        private int m_priority;

        /// <summary>
        /// The item's current text.  When the item's text changes, it 
        /// informs the registry it is registered with.
        /// </summary>        
        public string Text
        {
            get
            {
                return m_text;
            }
            set
            {
                if (m_text != value)
                {
                    m_text = value;
                    OnChanged();
                }
            }
        }
        protected string m_text;

        private void OnChanged()
        {
            m_registry.Changed(this);
        }

        /// <summary>
        /// The registry to register the status item with.
        /// </summary>
        /// TODO: Turn this into an event-driven system, and don't store registry here.
        protected StatusItemRegistry Registry
        {
            get
            {
                return m_registry;
            }
        }
        private StatusItemRegistry m_registry;

        /// <summary>
        /// This is used to calculate the fixed width (using 
        /// Graphics.MeasureString()) of the StatusItem when it is drawn on 
        /// the StatusBar.
        /// </summary>
        public string LongestTextPossible
        {
            get
            {
                return m_longestTextPossible;
            }
            set
            {
                m_longestTextPossible = value;
            }
        }
        private string m_longestTextPossible;

        /// <summary>
        /// The ToolTip to display.
        /// </summary>
        public string ToolTip
        {
            get
            {
                return m_toolTip;
            }
            set
            {
                m_toolTip = value;
                OnChanged();
            }
        }
        private string m_toolTip = string.Empty;
    }

    /// <summary>
    /// Status Items must implement this interface if they want themselves to 
    /// be displayed as progress bars.
    /// </summary>
    public interface IProgressStatusItem
    {
        /// <summary>
        /// The current progress value.  Implementation code must inform the 
        /// registry of changes to this value for it to be reflected in the 
        /// client.
        /// </summary>
        int Value
        {
            get;
            set;
        }

        /// <summary>
        /// The minimum progress value (usually 0).
        /// </summary>
        int MinValue
        {
            get;
        }

        /// <summary>
        /// The maximum progress value (usually 100).
        /// </summary>
        int MaxValue
        {
            get;
        }
    }

    /// <summary>
    /// The command context for StatusItems.
    /// </summary>
    public interface IStatusItemCommandContext : ICommandContext
    {
        /// <summary>
        /// The name/key of the StatusItem.
        /// </summary>
        string StatusItemName
        {
            get;
        }

        /// <summary>
        /// The registry the StatusItem is registered with.
        /// </summary>
        StatusItemRegistry Registry
        {
            get;
        }
    }

    /// <summary>
    /// The command context for StatusItems.
    /// </summary>
    public class StatusItemCommandContext : IStatusItemCommandContext
    {
        public StatusItemCommandContext(
            string itemName,
            StatusItemRegistry registry)
        {
            m_statusItemName = itemName;
            m_registry = registry;
        }

        #region IStatusItemCommandContext Members

        public string StatusItemName
        {
            get { return m_statusItemName; }
        }
        private string m_statusItemName;

        public StatusItemRegistry Registry
        {
            get
            {
                return m_registry;
            }
        }
        private StatusItemRegistry m_registry;

        #endregion IStatusItemCommandContext Members
    }
}
