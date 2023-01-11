#region File Header
//
//      FILE:   ColumnList.cs.
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
using System.Collections.Generic;
using System.Drawing.Design;
using System.ComponentModel;
using System.Xml;
using System.Xml.Serialization;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a list of <see cref="Column">Columns</see> for a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>
    /// The list supports XML serialization via the <see cref="WriteXml"/> and <see cref="ReadXml"/> methods
    /// to allow user customized attributes to be saved and restored.
    /// </remarks>
    /// <seealso href="XtraColumns.html">Using Columns</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="Column"/>
    public class ColumnList : BindingCollectionBase  
    {

        #region Member Variables
        
        /// <summary>
        /// The tree that this column list is associated with
        /// </summary>
        private VirtualTree _tree;

        #endregion

        #region Public Interface

        /// <summary>
        /// Return the Virtual Tree this collection is associated with.
        /// </summary>
        public VirtualTree Tree
        {
            get { return _tree; }
        }

        /// <summary>
        /// Return the Column at the given index.
        /// </summary>
        public Column this[ int index ]  
        {
            get  
            {
                return((Column) List[index]);
            }
            set  
            {
                List[index] = value;
            }
        }

        /// <summary>
        /// Return the Column with the given name.
        /// </summary>
        public Column this[ string name ]  
        {
            get  
            {
                foreach (Column column in List)
                {
                    if (column.Name == name)
                        return column;
                }
                return null;
            }
        }

        /// <summary>
        /// Add an existing Column to the list.
        /// </summary>
        /// <param name="value">The Column to add</param>
        /// <returns>The index at which the Column was added</returns>
        public int Add(Column value)  
        {
            return(List.Add(value));
        }

        /// <summary>
        /// Insert a column into the list at the given index.
        /// </summary>
        /// <param name="index">The index at which to insert the Column</param>
        /// <param name="column">The column to insert</param>
        public void Insert(int index, Column column)  
        {
            List.Insert(index, column);
        }

        /// <summary>
        /// Set the index of the given column within the list
        /// </summary>
        /// <param name="column">The Column to set the index for</param>
        /// <param name="newIndex">The new index for the column</param>
        public void SetIndexOf(Column column, int newIndex)
        {
            int index = IndexOf(column);
            if (index < 0) throw new ArgumentException("The column does not belong to this list", "column");
            if (newIndex < 0 || newIndex > Count - 1) throw new ArgumentOutOfRangeException("newIndex");
            if (index != newIndex)
            {
                SuspendChangeNotification();
                RemoveAt(index);
                List.Insert(newIndex, column);
                ResumeChangeNotification(false);
                OnListChanged(new ListChangedEventArgs(ListChangedType.ItemMoved, newIndex, index));
            }
        }

        /// <summary>
        /// Return an array of the visible columns
        /// </summary>
        /// <remarks>
        /// The list excludes the <see cref="NS.VirtualTree.PrefixColumn">PrefixColumn</see> (if any). 
        /// </remarks>
        /// <returns>A list of the visible columns</returns>
        public SimpleColumnList GetVisibleColumns()
        {
            SimpleColumnList columns = new SimpleColumnList();
            foreach (Column column in List)
            {
                if (column.Visible && !column.PrefixColumn)
                {
                    columns.Add(column);
                }
            }
            return columns;
        }

        /// <summary>
        /// Return an array of the visible scrollable (ie non-header) columns
        /// </summary>
        /// <remarks>
        /// The list excludes <see cref="NS.VirtualTree.PrefixColumn">PrefixColumn</see> (if any) and 
        /// <see cref="Column.Pinned">Pinned</see> Columns
        /// </remarks>
        /// <returns>A list of the visible scrollable columns</returns>
        public SimpleColumnList GetVisibleScrollableColumns()
        {
            SimpleColumnList columns = new SimpleColumnList();
            foreach (Column column in List)
            {
                if (column.Visible && !column.PrefixColumn && !column.Pinned)
                {
                    columns.Add(column);
                }
            }
            return columns;
        }

        /// <summary>
        /// Return an array of the visible pinned columns
        /// </summary>
        /// <returns>A list of the visible pinned columns</returns>
        public SimpleColumnList GetVisiblePinnedColumns()
        {
            SimpleColumnList columns = new SimpleColumnList();
            foreach (Column column in List)
            {
                if (column.Visible && !column.PrefixColumn && column.Pinned)
                {
                    columns.Add(column);
                }
            }
            return columns;
        }

        /// <summary>
        /// Return the index of the given Column in the list
        /// </summary>
        /// <param name="value">The Column to find the index of</param>
        /// <returns>The index of the Column in the list or -1 if not found</returns>
        public int IndexOf(Column value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given Column from the list if present
        /// </summary>
        /// <param name="value">The Column to remove</param>
        public void Remove(Column value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given Column
        /// </summary>
        /// <param name="value">The Column to look for</param>
        /// <returns>True if the list contains the given Column otherwise false</returns>
        public bool Contains(Column value)  
        {
            return(List.Contains(value));
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Initialise a new instance of a column list
        /// </summary>
        /// <param name="tree">The tree that this list is associated with</param>
        internal ColumnList(VirtualTree tree)
        {
            _tree = tree;
        }

        /// <summary>
        /// Attaches the <see cref="Column"/> to the <see cref="VirtualTree"/>
        /// </summary>
        /// <param name="index"></param>
        /// <param name="value"></param>
        protected override void OnInsertComplete(int index, Object value)  
        {
            Column column = (Column)value;
            column.SetTree(_tree);
            column.PropertyChanged += new PropertyChangedEventHandler(OnColumnChanged);
            base.OnInsertComplete(index, value);
        }

        /// <summary>
        /// Override to detach from the column Changed event
        /// </summary>
        protected override void OnClear()
        {
            foreach (Column column in List)
            {
                column.SetTree(null);
                column.PropertyChanged -= new PropertyChangedEventHandler(OnColumnChanged);
            }
            base.OnClear ();
        }

        /// <summary>
        /// Override to detach from the column Changed event
        /// </summary>
        protected override void OnRemoveComplete(int index, object value)
        {
            Column column = value as Column;
            column.SetTree(null);
            column.PropertyChanged -= new PropertyChangedEventHandler(OnColumnChanged);
            base.OnRemoveComplete (index, value);
        }

        /// <summary>
        /// Validates that the given object can be added to the list
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is Column))
                throw new ArgumentException("value must be of type Column", "value");
        }

        /// <summary>
        /// Handle a change to one of the columns contained by the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnColumnChanged(object sender, PropertyChangedEventArgs e)
        {
            int index = List.IndexOf(sender);
            if (index >= 0)
            {
                OnListChanged(new ListChangedEventArgs(ListChangedType.ItemChanged, index, index));
            }
        }

        #endregion

        #region XML Serialization

        /// <summary>
        /// Write the end-user customizable attributes of the columns (width, positions etc) 
        /// as XML
        /// </summary>
        /// <param name="writer">The writer to write the attributes to</param>
        /// <param name="elementName">The name of the enclosing element tag</param>
        public virtual void WriteXml(XmlWriter writer, string elementName)
        {
            writer.WriteStartElement(elementName);
            foreach (Column column in List)
            {
                writer.WriteStartElement("Column");
                writer.WriteAttributeString("name", column.Name);
                column.WriteXml(writer);
                writer.WriteEndElement();
            }
            writer.WriteEndElement();
        }

        /// <summary>
        /// Read the end-user customizable attributes of the columns (width, positions etc) 
        /// from XML
        /// </summary>
        /// <param name="reader">The reader to read the attributes from</param>
        /// <remarks>
        /// This method reads the attributes of columns that are currently in the collection from
        /// the given XML reader.   It will not create columns if they don't already exist in
        /// the collection. 
        /// </remarks>
        public virtual void ReadXml(XmlReader reader)
        {
            Tree.SuspendLayout();
            try
            {
                // save the original indexes of columns
                //
                Dictionary<Column, int> originalIndex = new Dictionary<Column, int>();
                foreach (Column column in this)
                {
                    originalIndex[column] = IndexOf(column);
                }

                reader.ReadStartElement();
                int index = 0;
                while (reader.IsStartElement("Column"))
                {
                    string name = reader.GetAttribute("name");
                    Column column = this[name];
                    if (column == null)
                    {
                        reader.Skip();
                    }
                    else
                    {
                        reader.ReadStartElement("Column");
                        column.ReadXml(reader);
                        reader.ReadEndElement();
                        originalIndex.Remove(column);
                        SetIndexOf(column, index);
                        index++;
                    }
                }

                // Set the index of any column which is new to its original
                // value
                //
                foreach (KeyValuePair<Column, int> pair in originalIndex)
                {
                    SetIndexOf(pair.Key, pair.Value);
                }
                
                reader.ReadEndElement();
            }
            finally
            {
                Tree.ResumeLayout();
            }
        }


        #endregion
    }

}


