#region File Header
//
//      FILE:   WidgetList.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Collections;
using System.Drawing.Design;
using System.ComponentModel;
namespace Infralution.Controls
{

    /// <summary>
    /// Defines a list of <see cref="Widget">Widgets</see>.
    /// </summary>
    public class WidgetList : CollectionBase  
    {

        #region Public Interface

        /// <summary>
        /// Return the Widget at the given index.
        /// </summary>
        public Widget this[ int index ]  
        {
            get { return((Widget) List[index]); }
            set { List[index] = value; }
        }

        /// <summary>
        /// Add a Widget to the list.
        /// </summary>
        /// <param name="widget">The widget to add</param>
        /// <returns>The index at which the widget was added</returns>
        public int Add(Widget widget)  
        {
            return List.Add(widget);
        }

        /// <summary>
        /// Insert a Widget into the list at the given index.
        /// </summary>
        /// <param name="widget">The widget to insert</param>
        /// <param name="index">The index at which to insert the Widget</param>
        public void Insert(Widget widget, int index)  
        {
            List.Insert(index, widget);
        }

        /// <summary>
        /// Return the index of the given widget in the list
        /// </summary>
        /// <param name="value">The Widget to find the index of</param>
        /// <returns>The index of the Widget in the list or -1 if not found</returns>
        public int IndexOf(Widget value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given Widget from the list if present
        /// </summary>
        /// <param name="value">The Widget to remove</param>
        public void Remove(Widget value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given Widget
        /// </summary>
        /// <param name="value">The Widget to look for</param>
        /// <returns>True if the list contains the given Widget otherwise false</returns>
        public bool Contains(Widget value)  
        {
            return(List.Contains(value));
        }

        /// <summary>
        /// Create a copy of this list of widgets
        /// </summary>
        /// <returns>A new list containing the same widgets</returns>
        public WidgetList Clone()
        {
            WidgetList clone = new WidgetList();
            foreach (Widget widget in List)
            {
                clone.Add(widget);
            }
            return clone;
        }

        #endregion

        #region Local Methods


        /// <summary>
        /// Validates that the given object can be added to the list
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is Widget))
                throw new ArgumentException("value must be of type Widget", "value");
        }

        #endregion
    }

}


