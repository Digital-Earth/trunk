#region File Header
//
//      FILE:   ObjectRowBinding.cs.
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
using System.Data;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using System.Drawing.Design;
using System.Reflection;
using Infralution.Common;
using Infralution.Controls;
using System.Globalization;
using NS = Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a <see cref="Row"/> of a 
    /// <see cref="VirtualTree"/> and relationships between Rows to the properties of an
    /// <see cref="Object"/> using <see cref="System.Reflection"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class is typically defined using the <see cref="VirtualTree"/> visual designer.  It can be used
    /// when the <see cref="VirtualTree.DataSource"/> is set to be any object.  
    /// The <see cref="TypeName"/> property specifies the name of the object <see cref="Type"/> 
    /// that this binding applies to.   The user creates <see cref="ObjectCellBinding">ObjectCellBindings</see> 
    /// (using the visual designer) that map the object properties/fields to <see cref="NS.VirtualTree.Columns"/> of 
    /// the <see cref="VirtualTree"/>. 
    /// </para>
    /// <para>
    /// The <see cref="ChildProperty"/> property specifies the name of an object property that returns 
    /// a <see cref="IList">list</see> of child items.  It can also be set to "this" or "Me" to specify that
    /// the item itself supports the <see cref="IList"/> interface to provide children.  This is particularly useful 
    /// when defining the binding for the root row when the <see cref="VirtualTree.DataSource"/> is an object that 
    /// supports <see cref="IList"/>.
    /// </para>
    /// <para>
    /// The <see cref="ParentProperty"/> property specifies the name of an object property that returns 
    /// the parent for the given item. 
    /// </para>
    /// </remarks>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.ObjectCellBinding"/>
    /// <seealso cref="RowData"/>
    public class ObjectRowBinding : RowBinding
    {

        #region Member Variables

        private Type   _type;
        private string _typeName;
        private string _typedListName;

        private string _childProperty;
        private string _parentProperty;

        private PropertyDescriptorCollection _propertyDescriptors;
        private Type _descriptorType;

        #endregion

        #region Public Interface


        /// <summary>
        /// Initialise a default object.
        /// </summary>
        public ObjectRowBinding()
        {
        }

        /// <summary>
        ///  Initialise an ObjectBinding object.
        /// </summary>
        /// <param name="type">The type to bind to</param>
        public ObjectRowBinding(Type type)
        {
            _type = type;
            if (_type != null)
            {
                _typeName = type.FullName;
            }
        }

        /// <summary>
        /// Return the cell binding to use for the given column 
        /// </summary>
        /// <param name="column">The column to get the cell binding for</param>
        /// <returns>The cell binding to use</returns>
        public new ObjectCellBinding CellBinding(Column column)
        {
            return (ObjectCellBinding) base.CellBinding(column);
        }

        /// <summary>
        /// The name of the <see cref="ITypedList"/> (if any) that the bound objects belongs to  
        /// </summary>
        /// <remarks>
        /// If the object that you wish to bind to belongs to a TypedList ie
        /// a list that supports <see cref="ITypedList"/> then you can bind to 
        /// the object by selecting the name of the TypedList that it belongs to.
        /// For standard object binding leave this null.
        /// </remarks>
        [Category("Data"), 
        Description("The name of the ITypedList (if any) that the bound objects belongs to"),
        DefaultValue(null)]
        public string TypedListName
        {
            get { return _typedListName; }
            set 
            { 
                _typedListName = StringUtilities.BlankToNull(value);
            }
        }

        /// <summary>
        /// Set/Get the name of the type that this binding applies to.  
        /// </summary>
        /// <remarks>
        /// This may be either a local type name, a fully qualified type name or
        /// a partial type (the last part of the typename).  If the name resolves to
        /// a Type then this also sets the <see cref="Type"/> property.
        /// </remarks>
        [Category("Data")] 
        [Description("The name of the type that this binding applies to")]
        [Editor("Infralution.Controls.Design.ObjectTypeNameEditor, " + Controls.DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string TypeName
        {
            get { return _typeName; }
            set 
            { 
                _typeName = StringUtilities.BlankToNull(value);
                if (_typeName != null)
                {
                    _type = ReflectionUtilities.ResolveType(_typeName);
                }
                else
                {
                    _type = null;
                }
            }
        }

        /// <summary>
        /// Set/Get the type that this binding applies to.
        /// </summary>
        /// <remarks>
        /// If this property is not set then binding will use pattern matching on the
        /// <see cref="TypeName"/> property.
        /// </remarks>
        [Browsable(false), 
         DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Type Type
        {
            get { return _type; }
            set 
            { 
                _type = value;
                _typeName = (value == null) ? null : _type.FullName; 
            }
        }

        /// <summary>
        /// Set/Get the property of the object to be used to find the list of child objects for this object.
        /// </summary>
        /// <remarks>
        /// If set to "this" or "Me" then the children of an item are found by using the <see cref="IList"/>
        /// interface of the item itself to get the children.  This is particularly useful when defining the binding
        /// for the root row when the <see cref="VirtualTree.DataSource"/> is an object that supports <see cref="IList"/>.
        /// </remarks>
        [Category("Data")]
        [Description("The property used to find children of this item type")]
        [Editor("Infralution.Controls.VirtualTree.Design.ObjectFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string ChildProperty
        {
            get { return _childProperty; }
            set { _childProperty = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the property of the object to be used to find parent object for a given item.
        /// </summary>
        /// <remarks>
        /// This property allows the <see cref="NS.VirtualTree"/> to locate the row that uniquely corresponds to a
        /// given item in the tree.  It must be set if the binding is to support the simplified methods 
        /// for locating and selecting items in the <see cref="VirtualTree"/>.  This includes 
        /// <see cref="NS.VirtualTree.FindRow(object)"/>, <see cref="NS.VirtualTree.SelectedItem"/> and 
        /// <see cref="NS.VirtualTree.SelectedItems"/>.   If the tree hierarchy is such that an item may
        /// appear multiple times within the tree then these simplified methods cannot be used and this property
        /// does not need to be set.
        /// </remarks>
        [Category("Data")]
        [Description("The property used to find the parent of this item type")]
        [Editor("Infralution.Controls.VirtualTree.Design.ObjectFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string ParentProperty
        {
            get { return _parentProperty; }
            set { _parentProperty = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Determines whether this binding is applicable for the given item.
        /// </summary>
        /// <param name="item">The item to bind</param>
        /// <returns>True if the TypeName matches that of the item</returns>
        public override bool BindsTo(object item)
        {
            if (_typeName == null || item == null) return false;
            
            if (_type != null)
            {
                return _type.IsAssignableFrom(item.GetType());
            }
            else // resort to matching by name
            {
                string typeName = item.GetType().FullName;
                return typeName.EndsWith(_typeName);
            }
        }

        /// <summary>
        /// Determines whether this binding is applicable for the given item/row
        /// </summary>
        /// <param name="item">The item to bind</param>
        /// <param name="row">The row the item belongs to (if any)</param>
        /// <returns>True if the TypeName matches that of the item</returns>
        /// <remarks>
        /// Overridden to handle binding of items belonging TypedLists.  If
        /// <see cref="TypedListName"/> is set then this checks if containing list
        /// is an <see cref="ITypedList"/> and returns true if the containing list 
        /// name matches <see cref="TypedListName"/>.
        /// </remarks>
        public override bool BindsTo(object item, Row row)
        {
            if (_typedListName == null || row == null)
                return BindsTo(item);

            Row parentRow = row.ParentRow;
            if (parentRow == null) return false;
            ITypedList typedList = parentRow.ChildItems as ITypedList;
            if (typedList == null) return false;

            return (typedList.GetListName(null) == _typedListName);
        }

        /// <summary>
        /// Returns the list of children for the given row
        /// </summary>
        /// <remarks>Uses reflection to find the children for the row</remarks>
        /// <param name="row">The row to get the children for</param>
        /// <returns>The list of children for the item</returns>
        public override IList GetChildrenForRow(Row row)
        {
            IList children = null;
            if (UseGetChildrenEvent)
            {
                children = base.GetChildrenForRow(row);            
            }
            else if (_childProperty != null)
            {
                object item = row.Item;
                string upperProperty = _childProperty.ToUpper(CultureInfo.InvariantCulture); 
                if (upperProperty == "THIS" || upperProperty == "ME")
                {
                    if (item is IList)
                        children = (IList)item;
                    else if (item is IListSource) 
                        children = ((IListSource)item).GetList();
                }
                else
                {
                    PropertyDescriptor pd = GetPropertyDescriptorForRow(row, _childProperty);
                    if (pd != null)
                    {
                        children = (pd.GetValue(item) as IList);
                    }
                }
            }
            SortChildren(children);
            return children;
        }

        /// <summary>
        /// Returns the parent object for the given item.
        /// </summary>
        /// <remarks>Raises the <see cref="GetParentForItem"/> event if set otherwise uses the <see cref="ParentProperty"/></remarks>
        /// <param name="item">The item to get the parent for</param>
        /// <returns>The parent item for the given item</returns>
        public override object GetParentForItem(object item)
        {
            if (UseGetParentEvent) return base.GetParentForItem(item);   
            object parent = null;
            if (_parentProperty != null)
            {
                PropertyDescriptor pd = GetPropertyDescriptor(item, _parentProperty);
                if (pd != null)
                {
                    parent = pd.GetValue(item);
                }
            }
            return parent;
        }
                  
        /// <summary>
        /// Return true if the given row is allowed to be dragged
        /// </summary>
        /// <param name="row">The row to get the allowed drop locations for</param>
        /// <returns>True if the row is allowed to be dragged</returns>
        public override bool GetAllowDrag(Row row)
        {
            return (row.ParentRow == null) ? false : AllowDrag;
        }

        /// <summary>
        /// Return the allowed drop locations for the given row and dropped data
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The allowed drop locations</returns>
        public override RowDropLocation GetAllowedDropLocations(Row row, IDataObject data)
        {
            // check the item being dropped 
            //            
            Row[] dropRows = (Row[])data.GetData(typeof(Row[]));
            if (dropRows == null) return RowDropLocation.None;

            bool canDropOn = AllowDropOnRow;
            bool canDropAbove = AllowDropAboveRow;
            bool canDropBelow = AllowDropBelowRow;

            foreach (Row dropRow in dropRows)
            {
                // can't a row drop on itself or it's children
                //
                if (row == dropRow || row.IsDescendant(dropRow))
                {
                    canDropOn = false;
                    canDropAbove = false;
                    canDropBelow = false;
                }
                else
                {
                    // if the binding for the parent of the dropped row is not this binding 
                    // then we can't drop the row on this row
                    //
                    RowBinding parentBinding = Tree.GetBindingForRow(dropRow.ParentRow);
                    if (parentBinding != this)
                        canDropOn = false;

                    // if the binding for the dropped row is not this binding or this row is the root row 
                    // then we can't drop above or below this row
                    //
                    RowBinding binding = Tree.GetBindingForRow(dropRow);
                    if (binding != this || row.ParentRow == null)
                    {
                        canDropAbove = false;
                        canDropBelow = false;
                    }
                }

            }
            RowDropLocation locations = RowDropLocation.None;
            if (canDropOn) 
                locations |= RowDropLocation.OnRow;
            if (canDropAbove)
                locations |= RowDropLocation.AboveRow;
            if (canDropBelow)
                locations |= RowDropLocation.BelowRow;
            return locations;
        }

        /// <summary>
        /// Handle dropping of data onto a row of this type
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The drop location</param>
        /// <param name="data">The data being dropped</param>
        /// <param name="dropEffect">The type of drop operation</param>
        public override void OnDrop(Row row, RowDropLocation dropLocation, IDataObject data, DragDropEffects dropEffect)
        {            
            // check the item being dropped 
            //            
            Row[] dropRows = (Row[])data.GetData(typeof(Row[]));
            if (dropRows == null) return;

            Tree.SuspendLayout();
            Tree.SuspendDataUpdate();
            try
            {
                switch (dropLocation)
                {
                    case RowDropLocation.OnRow:
                        DropOnRow(row, dropRows);
                        break;
                    case RowDropLocation.AboveRow:
                        DropAboveRow(row, dropRows);
                        break;
                    case RowDropLocation.BelowRow:
                        DropBelowRow(row, dropRows);
                        break;
                }

                if (Tree.SelectedRows.Count > 0)
                    Tree.FocusRow = Tree.SelectedRows[0];
            }
            finally
            {
                Tree.ResumeDataUpdate();
                Tree.ResumeLayout();
                Tree.PerformLayout();
            }
        }

        /// <summary>
        /// Get the text to display in the bindings editor for this binding.
        /// </summary>
        public override string DisplayName
        {
            get
            {
                string name = "Unknown";
                if (_typedListName != null)
                    name = _typedListName;
                else if (_typeName != null)
                    name = _typeName;
                return String.Format("Object({0})", name);
            }
        }

        /// <summary>
        /// Creates a new <see cref="ObjectCellBinding"/>.
        /// </summary>
        /// <returns>A new ObjectCellBinding</returns>
        public override CellBinding CreateCellBinding()
        {
            return new ObjectCellBinding();
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Find the row binding for items from the given typedlist
        /// </summary>
        /// <param name="typedList">The typed list to find the child binding for</param>
        /// <returns>The binding for the given typed list item</returns>
        private ObjectRowBinding GetListItemBinding(ITypedList typedList)
        {
            string typedListName = typedList.GetListName(null);
            foreach (RowBinding binding in Tree.RowBindings)
            {
                ObjectRowBinding orb = binding as ObjectRowBinding;
                if (orb.TypedListName == typedListName)
                    return orb;
            }
            return null;
        }


        /// <summary>
        /// Sorts the given list of children based on the current sort column of the tree
        /// </summary>
        /// <remarks>
        /// The children list must support IBindingList and ITypedList for sorting to be
        /// supported.
        /// </remarks>
        /// <param name="children">The list of children to support</param>
        protected virtual void SortChildren(IList children)
        {
            // Sort the children if the children list supports IBindingList sorting
            //
            if (children is IBindingList && children is ITypedList)
            {
                ITypedList typedList = (ITypedList)children;
                IBindingList bindingList = (IBindingList)children;

                if (bindingList.SupportsSorting)
                {
                    // locate the property to sort by
                    //
                    Column sortColumn = Tree.SortColumn;
                    PropertyDescriptor property = null;
                    if (sortColumn != null)
                    {
                        // find the row binding for items in the typed list
                        //
                        ObjectRowBinding itemRowBinding = GetListItemBinding(typedList);
                        if (itemRowBinding != null)
                        {
                            // get the property descriptor corresponding to the sort column (if any)
                            //
                            ObjectCellBinding itemCellBinding = itemRowBinding.CellBinding(sortColumn);
                            if (itemCellBinding != null && itemCellBinding.Field != null)
                            {
                                property = itemRowBinding.GetPropertyDescriptorForTypedList(typedList, itemCellBinding.Field);
                            }
                        }
                    }
                    
                    // now sort the list by the property
                    //
                    if (property == null)
                    {
                        if (bindingList.IsSorted)
                            bindingList.RemoveSort();
                    }
                    else
                    {
                        if (bindingList.SortProperty != property || 
                            bindingList.SortDirection != sortColumn.SortDirection)
                        {
                            bindingList.ApplySort(property, sortColumn.SortDirection);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Set the parent object for the given item.
        /// </summary>
        /// <param name="item">The item to set the parent for</param>
        /// <param name="parent">The parent object</param>
        protected virtual void SetParentForItem(object item, object parent)
        {
            if (_parentProperty != null)
            {
                PropertyDescriptor pd = GetPropertyDescriptor(item, _parentProperty);
                if (pd != null && !pd.IsReadOnly)
                {
                    pd.SetValue(item, parent);
                }
            }
        }

        /// <summary>
        /// Handle dropping the given drop row on to the row
        /// </summary>
        /// <param name="row">The row to drop onto</param>
        /// <param name="dropRows">The rows being dropped</param>
        protected virtual void DropOnRow(Row row, Row[] dropRows)
        {
            ArrayList items = new ArrayList();
            foreach (Row dropRow in dropRows)
            {
                object item = dropRow.Item;
                items.Add(item);

                if (row.ChildItems != null)
                {
                    dropRow.ParentRow.ChildItems.Remove(item);
                    row.ChildItems.Add(item);
                }
                SetParentForItem(item, row.Item);
            }
            row.Expanded = true;
            Tree.UpdateRows(false);

            // if we are dropping data from another tree then we need to update it
            // as well
            //
            if (dropRows.Length > 0 && dropRows[0].Tree != Tree)
            {
                dropRows[0].Tree.UpdateRows(false);
            }

            // find the new rows to select - have to do this AFTER calling UpdateChildren
            //
            ArrayList selectRows = new ArrayList();
            foreach (object item in items)
            {
                Row selectRow = row.ChildRow(item);
                if (selectRow != null)
                    selectRows.Add(selectRow);
            }
            Tree.SelectedRows.Set(selectRows);

        }

        /// <summary>
        /// Handle dropping the given drop row below the row
        /// </summary>
        /// <param name="row">The row to drop below</param>
        /// <param name="dropRows">The rows being dropped</param>
        protected virtual void DropBelowRow(Row row, Row[] dropRows)
        {
            ArrayList items = new ArrayList();
            for (int i=dropRows.Length - 1; i >= 0; i--)
            {
                Row dropRow = dropRows[i];
                object item = dropRow.Item;
                items.Add(item);

                int index = row.ChildIndex + 1;

                // if we are moving a row within the same parent then
                // removing a drop item before this row changes the index of 
                // this row so we have to adjust the index accordingly
                //
                if ((dropRow.ParentRow == row.ParentRow) &&
                    (dropRow.ChildIndex < row.ChildIndex))
                {
                    index--;
                }
                dropRow.ParentRow.ChildItems.Remove(item);
                row.ParentRow.ChildItems.Insert(index, item);
                SetParentForItem(item, row.ParentRow.Item);
            }

            Tree.RootRow.UpdateChildren(false, true);

            // if we are dropping data from another tree then we need to update it
            // as well
            //
            if (dropRows.Length > 0 && dropRows[0].Tree != Tree)
            {
                dropRows[0].Tree.UpdateRows(false);
            }

            // find the new rows to select - have to do this AFTER calling UpdateChildren
            //
            ArrayList selectRows = new ArrayList();
            foreach (object item in items)
            {
                Row selectRow = row.ParentRow.ChildRow(item);
                if (selectRow != null)
                    selectRows.Add(selectRow);
            }
            Tree.SelectedRows.Set(selectRows);

        }

        /// <summary>
        /// Handle dropping the given drop row above the row
        /// </summary>
        /// <param name="row">The row to drop above</param>
        /// <param name="dropRows">The rows being dropped</param>
        protected virtual void DropAboveRow(Row row, Row[] dropRows)
        {
            ArrayList items = new ArrayList();
            for (int i=dropRows.Length - 1; i >= 0; i--)
            {
                Row dropRow = dropRows[i];
                object item = dropRow.Item;
                items.Add(item);

                int index = row.ChildIndex;

                // if we are moving a row within the same parent then
                // remoing a drop item before this row changes the index of 
                // this row so we have to adjust the index accordingly
                //
                if ((dropRow.ParentRow == row.ParentRow) &&
                    (dropRow.ChildIndex < row.ChildIndex))
                {
                    index--;
                }
                dropRow.ParentRow.ChildItems.Remove(item);
                row.ParentRow.ChildItems.Insert(index, item);
                SetParentForItem(item, row.ParentRow.Item);
            }

            Tree.RootRow.UpdateChildren(false, true);

            // if we are dropping data from another tree then we need to update it
            // as well
            //
            if (dropRows.Length > 0 && dropRows[0].Tree != Tree)
            {
                dropRows[0].Tree.UpdateRows(false);
            }

            // find the new rows to select - have to do this AFTER calling UpdateChildren
            //
            ArrayList selectRows = new ArrayList();
            foreach (object item in items)
            {
                Row selectRow = row.ParentRow.ChildRow(item);
                if (selectRow != null)
                    selectRows.Add(selectRow);
            }
            Tree.SelectedRows.Set(selectRows);
        }


        /// <summary>
        /// Return the property descriptor for the given property of an item
        /// </summary>
        /// <remarks>
        /// The row binding caches the property descriptors to improve performance.
        /// </remarks>
        /// <param name="item">The item to get the descriptor for</param>
        /// <param name="property">The name of the property</param>
        /// <returns>The property descriptor</returns>
        protected PropertyDescriptor GetPropertyDescriptor(object item, string property)
        {
            if (_descriptorType != item.GetType())
            {
                _propertyDescriptors = TypeDescriptor.GetProperties(item);
                _descriptorType = item.GetType();
            }
            return _propertyDescriptors[property];
        }

        /// <summary>
        /// Return the property descriptor for the given property of a row
        /// </summary>
        /// <remarks>
        /// The row binding caches the property descriptors to improve performance.
        /// </remarks>
        /// <param name="row">The row containing the item to get the descriptor for</param>
        /// <param name="property">The name of the property</param>
        /// <returns>The property descriptor - or null if not found</returns>
        internal protected PropertyDescriptor GetPropertyDescriptorForRow(Row row, string property)
        {
            if (_typedListName == null)
                return GetPropertyDescriptor(row.Item, property);
            Row parentRow = row.ParentRow;
            if (parentRow == null) return null;
            ITypedList typedList = parentRow.ChildItems as ITypedList;
            if (typedList == null) return null;
            return GetPropertyDescriptorForTypedList(typedList, property);
        }

        /// <summary>
        /// Return the property descriptor for the given property of a typed list
        /// </summary>
        /// <remarks>
        /// The row binding caches the property descriptors to improve performance.
        /// </remarks>
        /// <param name="typedList">The typedList containing the item to get the descriptor for</param>
        /// <param name="property">The name of the property</param>
        /// <returns>The property descriptor - or null if not found</returns>
        protected PropertyDescriptor GetPropertyDescriptorForTypedList(ITypedList typedList, string property)
        {
            if (_descriptorType != typedList.GetType())
            {
                _propertyDescriptors = typedList.GetItemProperties(null);
                _descriptorType = typedList.GetType();
            }
            return _propertyDescriptors[property];
        }

        #endregion
    }
}
