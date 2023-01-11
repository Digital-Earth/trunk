#region File Header
//
//      FILE:   Row.cs.
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
using System.Drawing;
using System.Diagnostics;
using System.ComponentModel;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines information about a single row displayed in a <see cref="VirtualTree"/> and its
    /// relationships to other rows in the <see cref="VirtualTree"/>.   
    /// </summary>
    /// <remarks>
    /// <para>
    /// Unlike most other tree implementations <see cref="VirtualTree"/> is designed from the ground
    /// up to use data binding.  It minimizes resource usage, even when viewing extremely large datasets,
    /// by only loading and maintaining information that it needs to display the current tree 
    /// representation/state.   This information is stored in Row objects.   The user cannot directly create
    /// Row objects - they are created by the <see cref="VirtualTree"/> when required to display the
    /// tree representation.   
    /// </para>
    /// <para>
    /// Row objects store information (obtained through databinding) about the parent-child relationships 
    /// between items displayed in the tree and provide a mechanism by which the application can locate, 
    /// select and modify the state of the displayed Rows.
    /// </para>
    /// <para>
    /// The visual representation of Rows is performed by <see cref="RowWidget">RowWidgets</see>.  The
    /// <see cref="VirtualTree"/> creates a <see cref="RowWidget"/> for each Row that is currently visible
    /// to the user to display the visual representation of the Row and handle mouse interactions for the Row.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraRows.html">Working with Rows</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="RowBinding"/>
    /// <seealso cref="RowWidget"/>
    /// <seealso cref="Column"/>
    public class Row 
    {
        #region Member Variables

        /// <summary>
        /// The tree this row belongs to
        /// </summary>
        private VirtualTree _tree;      

        /// <summary>
        /// The domain item the row corresponds to
        /// </summary>
        private object _item;                 

        /// <summary>
        /// A list of items to be displayed in the tree as children of the domain item
        /// </summary>
        private IList _childItems;           

        /// <summary>
        /// The relative index of this row within its parent.
        /// </summary>
        private int _childIndex;

        /// <summary>
        /// The absolute index of this row within the tree
        /// </summary>
        private int _rowIndex;          

        /// <summary>
        /// The absolute index of the last descendant of this row within the tree.  If
        /// the row has no children then this will be the same as _rowIndex.
        /// </summary>
        private int _lastDescendantRowIndex;           

        /// <summary>
        /// The absolute index of the last child of this row within the tree. If
        /// the row has no children then this will be the same as _rowIndex.
        /// </summary>
        private int _lastChildRowIndex;           

        /// <summary>
        /// The nesting level of this row ie how many anscestors does this row have
        /// </summary>
        private int _level = 0;           

        /// <summary>
        /// The parent row for this row (if any)
        /// </summary>
        private Row _parentRow;              

        /// <summary>
        /// Child rows of this row which have been cached.   Rows are
        /// indexed by the relative child index (ie zero is the first child)
        /// </summary> 
        private SortedList _childRows = new SortedList(); 

        /// <summary>
        /// Have the children of this row been loaded
        /// </summary>
        private bool _childItemsLoaded = false; 

        /// <summary>
        /// Is this row currently expanded
        /// </summary>
        private bool _expanded = false;

        /// <summary>
        /// The number of currently active rows (for debugging purposes)
        /// </summary>
        static int _activeRows = 0;

        #endregion

        #region Public Methods
        
        /// <summary>
        /// The VirtualTree this row is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return _tree; }
        }

        /// <summary>
        /// The domain item this row represents
        /// </summary>
        public object Item
        {
            get { return _item; }
        }

        /// <summary>
        /// The list of child domain items for this row (may be null)
        /// </summary>
        public IList ChildItems
        {
            get { return _childItems; }
        }

        /// <summary>
        /// Return the parent row of this row
        /// </summary>
        /// <remarks>
        /// This is null if this is the root row of the tree
        /// </remarks>
        public Row ParentRow
        {
            get { return _parentRow; }
        }

        /// <summary>
        /// Returns the path of this row relative to the root row.
        /// </summary>
        /// <remarks>
        /// The path for the root row is an emtpy list.
        /// </remarks>
        public IList Path
        {
            get
            {
                ArrayList path = new ArrayList();
                Row row = this;
                while (row.ParentRow != null)
                {
                    path.Insert(0, row.Item);
                    row = row.ParentRow;
                }
                return path;
            }
        }

        /// <summary>
        /// Return the number of children this row has (handles case where childItems is null).  
        /// </summary>
        public int NumChildren
        {
            get { return (_childItems == null) ? 0 : _childItems.Count; }
        }

        /// <summary>
        /// Has this row been expanded?
        /// </summary>
        public bool Expanded
        {
            get { return _expanded; } 
            set
            {
                if (value != _expanded)
                {
                    _expanded = value;
                    if (value)
                        _tree.OnRowExpand(this);
                    else
                        _tree.OnRowCollapse(this);
                    Reindex();
                    _tree.PerformLayout();
                }
            }
        }

        /// <summary>
        /// Expand the row and scroll children into view if necessary
        /// </summary>
        public virtual void Expand()
        {
            Expanded = true;
            if (_tree.AutoScrollOnExpand)
            {
                // scroll the last child into view
                //
                int lastChildIndex = LastChildRowIndex;
                if (lastChildIndex > _tree.BottomRowIndex)
                {
                    _tree.BottomRowIndex = lastChildIndex;

                    // ensure this row doesn't get scrolled out of view
                    // handling case where this is the root row and we
                    // ShowRootRow set to false
                    //
                    int rowIndex = RowIndex;
                    if (this == _tree.RootRow && !_tree.ShowRootRow)
                    {
                        rowIndex = 1;
                    }
                    if (rowIndex < _tree.TopRowIndex)
                    {
                        _tree.TopRowIndex = rowIndex;
                    }
                }
            }

            // Set the context row to the first child
            //
            if (NumChildren > 0)
            {
                _tree.ContextRow = ChildRowByIndex(0);
            }

        }

        /// <summary>
        /// Collapse the row and set the <see cref="VirtualTree.ContextRow"/>
        /// </summary>
        public virtual void Collapse()
        {
            Expanded = false;
            _tree.ContextRow = this;
        }

        /// <summary>
        /// Has this row been selected?
        /// </summary>
        public bool Selected
        {
            get 
            { 
                return _tree.Selected(this); 
            } 
            set 
            { 
                if (value)
                    _tree.SelectedRows.Add(this);
                else
                    _tree.SelectedRows.Remove(this);
            }
        }

        /// <summary>
        /// Is this row being edited by the user
        /// </summary>
        public bool Editing
        {
            get { return (_tree.EditRow  == this); }
        }

        /// <summary>
        /// Is this row the top row?
        /// </summary>
        public bool IsTopRow
        {
            get { return (_tree.TopRow == this); }
        }

        /// <summary>
        /// Does this row have focus?
        /// </summary>
        public bool HasFocus
        {
            get { return (_tree.FocusRow == this); }
        }

        /// <summary>
        /// Return the nesting level of this row within the tree hierarchy
        /// </summary>
        public int Level
        {
            get { return _level; }
        }

        /// <summary>
        /// Get the relative index of this row within its parent
        /// </summary>
        public int ChildIndex
        {
            get { return _childIndex; }
        }

        /// <summary>
        /// Returns the absolute row index within the <see cref="VirtualTree"/> of this row.
        /// </summary>
        public int RowIndex
        {
            get { return _rowIndex; } 
        }

        /// <summary>
        /// Returns the absolute row index within the <see cref="VirtualTree"/> of the last 
        /// descendant of this row.
        /// </summary>
        /// <remarks>
        /// If the row has no children then this will be the same as <see cref="RowIndex"/>.
        /// </remarks>
        public int LastDescendantRowIndex
        {
            get { return _lastDescendantRowIndex; } 
        }

        /// <summary>
        /// Returns the absolute row index within the <see cref="VirtualTree"/> of the last 
        /// direct child of this row
        /// </summary>
        /// <remarks>
        /// If the row has no children then this will be the same as <see cref="RowIndex"/>.
        /// </remarks>
        public int LastChildRowIndex
        {
            get { return _lastChildRowIndex; }
        }

        /// <summary>
        /// Returns true if an expansion icon should be shown for this item
        /// </summary>
        public bool ShowExpansionIndicator
        {
            get { return (_level > 0) && (!_childItemsLoaded || NumChildren > 0); }
        }

        /// <summary>
        /// Return the child row of this row at the given child index.  
        /// </summary>
        /// <param name="childIndex">The child index of the row to get</param>
        /// <returns>Returns the row at the given child index - or null if the index is out of bounds</returns>
        public Row ChildRowByIndex(int childIndex)
        {
            // ensure that we have loaded the child items for this row
            //
            LoadChildItemsIfRequired();
            if (_childItems == null) return null; // there are no children
            if (childIndex < 0 || childIndex >= _childItems.Count) return null; // invalid index

            Row row = (Row)_childRows[childIndex];
            if (row == null)
            {
                // find the cached child row prior to this new row (if any)
                //
                Row prevRow = FindCachedChildRowPriorToChildIndex(childIndex);

                // calculate the absolute row index of this child row
                //
                int rowIndex;
                if (prevRow == null)
                {
                    rowIndex = _rowIndex + childIndex + 1;
                }
                else
                {
                    rowIndex = prevRow.LastDescendantRowIndex + (childIndex - prevRow.ChildIndex);
                }                   
                row = _tree.CreateRow(_childItems[childIndex], this, childIndex, rowIndex);
            }
            return row;
        }

        /// <summary>
        /// Return the child row of this row that corresponds to the given item.  
        /// If the item is not a child item then returns null.
        /// </summary>
        /// <param name="item">The item to return the row for</param>
        /// <returns>Returns the row corresponding to the item - or null if the item is not a child of this row</returns>
        public Row ChildRow(object item)
        {
            // ensure that we have loaded the child items for this row
            //
            LoadChildItemsIfRequired();
            if (_childItems == null) return null; // there are no children
            return (ChildRowByIndex(_childItems.IndexOf(item)));
        }

        /// <summary>
        /// Return a descendant row of this row with the given path relative to this row.
        /// </summary>
        /// <param name="path">
        /// The path (child,grandchild,greatgrandchild etc) to the row, relative to (but not including)
        /// this row.
        /// </param>
        /// <returns>The row corresponding to the relative path</returns>
        public Row FindDescendantRow(IList path)
        {
            if (path == null || path.Count == 0) return this;
            Row childRow = ChildRow(path[0]);
            if (path.Count == 1 || childRow == null) return childRow;
            object[] childPath = new object[path.Count - 1];
            for (int i=0; i < childPath.Length; i++)
            {
                childPath[i] = path[i+1];
            }
            return childRow.FindDescendantRow(childPath);
        }

        /// <summary>
        /// Return a descendant row of this row with the given positional path relative to this row.
        /// </summary>
        /// <param name="path">
        /// An array of child indices which specifies the path (child,grandchild,greatgrandchild etc) to 
        /// the row, relative to (but not including) this row.
        /// </param>
        /// <returns>The row corresponding to the relative positional path</returns>
        public Row FindDescendantRowByIndex(int[] path)
        {
            if (path == null || path.Length == 0) return this;
            Row childRow = ChildRowByIndex(path[0]);
            if (path.Length == 1 || childRow == null) return childRow;
            int[] childPath = (int[])ArrayUtilities.Slice(path, 1, path.Length - 1);
            return childRow.FindDescendantRowByIndex(childPath);
        }

        /// <summary>
        /// Update the children of this row following a change to the list of child items for this
        /// row.   
        /// </summary>
        /// <remarks>
        /// If the list of child items supports <see cref="IBindingList"/> then this is called automatically
        /// when the list is changed.  The client application must call this manually for lists which
        /// don't support <see cref="IBindingList"/> to see changes to the list of child items reflected in 
        /// the tree.
        /// </remarks>
        /// <param name="reloadChildren">
        /// If true the children are reloaded. This is useful if you
        /// need to generate a completely new list of children for a row rather than modifying the 
        /// current list of children.
        /// </param>
        /// <param name="recursive">If true recursively updates all descendants</param>
        public void UpdateChildren(bool reloadChildren, bool recursive)
        {
            _tree.SuspendLayout();
            try
            {

                UpdateChildrenNoReindex(reloadChildren, recursive);
                Reindex();

                // ensure that cached row widgets update their data next time they
                // are displayed
                //
                _tree.UpdateRowData();
            }
            finally
            {
                _tree.ResumeLayout();
            }
        }
    
        /// <summary>
        /// Expand all of the ancestors (ie parent, grandparent etc) of this row.  
        /// </summary>
        public void ExpandAncestors()
        {
            if (_parentRow != null)
            {
                _parentRow.Expanded = true;
                _parentRow.ExpandAncestors();
            }
        }

        /// <summary>
        /// Returns true if all the ancestors of this row are expanded (thus
        /// the row is potentially visible)
        /// </summary>
        public bool AncestorsExpanded
        {
            get 
            {
                if (_parentRow == null) return true;
                return _parentRow.AncestorsExpanded && _parentRow.Expanded;
            }
        }

        /// <summary>
        /// Expand the children of this row.  
        /// </summary>
        /// <param name="recursive">If true recursively expands all descendants</param>
        public void ExpandChildren(bool recursive)
        {
            ExpandChildrenNoReindex(recursive);
            Reindex();          
            _tree.PerformLayout();                          
        }

        /// <summary>
        /// Collapse the children of this row.  
        /// </summary>
        /// <param name="recursive">If true recursively collapses all descendants</param>
        public void CollapseChildren(bool recursive)
        {
            CollapseChildrenNoReindex(recursive);
            Reindex();          
            _tree.PerformLayout();                          
        }

        /// <summary>
        /// Ensure that this row is visible.  
        /// </summary>
        /// <remarks>
        /// This expands the anscestors of the row and then ensures that the row is within the 
        /// top and bottom rows of the <see cref="VirtualTree"/>.   The <see cref="VirtualTree"/>
        /// must have been made visible prior to using this method (so that the top and bottom
        /// visible rows have been calculated).
        /// </remarks>
        public void EnsureVisible()
        {
            ExpandAncestors();
            if (_rowIndex < _tree.TopRowIndex)
            {
                _tree.TopRowIndex = _rowIndex;
            }
            else if (_rowIndex > _tree.BottomRowIndex)
            {
                _tree.BottomRowIndex = _rowIndex;
            }
        }

        /// <summary>
        /// Return true if this row is currently visible
        /// </summary>
        public bool Visible
        {
            get 
            {
                return (AncestorsExpanded && _rowIndex >= _tree.TopRowIndex && 
                        _rowIndex <= _tree.BottomRowIndex + 1);
            }
        }   

        /// <summary>
        /// Return true if this row is a descendant of the given row
        /// </summary>
        /// <param name="fromRow">The row to check as a possible anscestor</param>
        /// <returns>True if this row is a descendant</returns>
        public bool IsDescendant(Row fromRow)
        {
            if (_parentRow == fromRow)
                return true;
            if (_parentRow == null)
                return false;
            else
                return _parentRow.IsDescendant(fromRow);
        }

        /// <summary>
        /// Returns true if the row is no longer valid and has been disposed
        /// </summary>
        public bool Disposed
        {
            get { return (_childRows == null); }
        }
 
        #endregion

        #region Internal Methods

        /// <summary>
        /// Create a new row object for the given domain item
        /// </summary>
        /// <param name="tree">The tree the row belongs to</param>
        /// <param name="item">The domain item the row corresponds to</param>
        /// <param name="parentRow">The row correspond to the item's parent</param>
        /// <param name="childIndex">The relative index of this row within its parent</param>
        /// <param name="rowIndex">The absolute row index that this row starts on</param>
        internal protected Row(VirtualTree tree, object item, Row parentRow, int childIndex, int rowIndex)
        {
            _tree = tree;
            _item = item;
            _parentRow = parentRow;
            _childIndex = childIndex;
            _rowIndex = rowIndex;
            _lastDescendantRowIndex = rowIndex;
            _lastChildRowIndex = rowIndex;

            if (parentRow == null)
            {
                _level = 0;
                Expanded = true;
            }
            else
            {
                // add this row into the parent's set of child rows
                //
                parentRow._childRows[childIndex] = this;
                _level = parentRow.Level + 1;
            }
 
            RowChildPolicy childPolicy = _tree.GetChildPolicyForRow(this);
            if (childPolicy != RowChildPolicy.LoadOnExpand)
            {
                LoadChildItemsIfRequired();
            }
 
            if (childPolicy == RowChildPolicy.AutoExpand)
            {   
                // NOTE: The children of this row may in turn be autoexpanded however
                // if we just set the Expand property these children will not be immediately
                // enumerated - because rows are created during the tree Layout event and
                // calling PerformLayout from within this has no effect.  Instead we
                // call Tree.UpdateWidgets to force the tree to update the  displayed rows
                //
                _expanded = true;
                _tree.OnRowExpand(this);
                if (NumChildren > 0)
                {
                    Reindex();
                    _tree.UpdateWidgets();
                }
            }

            _activeRows++;
            // ### Debug.WriteLine("Created Row: " + _activeRows);
        }

        /// <summary>
        /// Row destructor
        /// </summary>
        ~Row()
        {
            _activeRows--;
            // ###   Debug.WriteLine("Destroyed Row: " + _activeRows);
        }

        /// <summary>
        /// Populates the given row table with row entries, from the branch of the tree rooted
        /// at this row, that fall within the given absolute start/end row indices.  Recursively
        /// calls this method on each of its expanded children to fully populate the row table.
        /// </summary>
        /// <param name="startIndex">Specifies the starting absolute row index</param>
        /// <param name="endIndex">Specifies the ending absolute row index</param>
        /// <param name="rows">Collection of rows indexed by absolute row index updated by this method</param>
        internal void GetRows(int startIndex, int endIndex, Hashtable rows)
        {
            // check if this row or any of its children fall within the given range
            //
            if (startIndex > _lastDescendantRowIndex || endIndex < _rowIndex) return;
    
            // add this row to the row table
            //
            rows[_rowIndex] = this;

            // check whether this row has expanded children 
            //
            if (!Expanded || NumChildren == 0) return;

            // find the child row prior to the startIndex (if any)
            //
            Row prevChildRow = FindCachedChildRowPriorToRowIndex(startIndex);

            // calculate the child index to start at and the corresponding row index
            //
            int childIndex;
            int rowIndex;
            if (prevChildRow == null)
            {
                // no cached rows prior to the given startIndex so calculate the child index directly
                // from the difference between startIndex and the index of this row
                //
                if (startIndex > _rowIndex)
                {
                    childIndex = startIndex - _rowIndex - 1;
                    rowIndex = startIndex;
                }
                else
                {
                    // start index is prior to this row so start from the first child
                    //
                    rowIndex = _rowIndex + 1;
                    childIndex = 0;
                }
            }
            else
            {
                // if the prevChildRow brackets the startIndex then just set the rowIndex to this child
                //
                if (prevChildRow.LastDescendantRowIndex >= startIndex)
                {
                    rowIndex = prevChildRow.RowIndex;
                    childIndex = prevChildRow.ChildIndex;
                }
                else
                {
                    // otherwise adjust child index for the gap between the last cached child
                    // and the child at rowIndex == startIndex
                    //
                    childIndex = prevChildRow.ChildIndex + startIndex - prevChildRow.LastDescendantRowIndex;
                    rowIndex = startIndex;
                }
            }                     
    
            // for each of the children of this row recursively GetRows
            //
            int numChildItems = NumChildren;
            for (;rowIndex <= endIndex && childIndex < numChildItems; childIndex++, rowIndex++)
            {
                // check if the row has been cached previously
                //
                Row row = (Row)_childRows[childIndex];
                if (row == null)
                {
                    // no cached row so create a new row object
                    //
                    row = _tree.CreateRow(_childItems[childIndex], this, childIndex, rowIndex);
                }
                row.GetRows(startIndex, endIndex, rows);
                rowIndex = row.LastDescendantRowIndex;
            }
        }

        /// <summary>
        /// Calculates new row indices for this row and each of its children. 
        /// </summary>
        /// <param name="index">The new index for this row</param>
        internal void Reindex(int index)
        {
            // ignore reindex of rows that have been disposed - this can happen if there is rentrant
            // code that while reindexing causes a data update
            //
            if (Disposed) return;

            _rowIndex = index;
            _lastChildRowIndex = _rowIndex;
            _lastDescendantRowIndex = _rowIndex;
            if (Expanded)
            {
                // ensure that we have loaded the child items for this row
                //
                LoadChildItemsIfRequired();
                if (Disposed) return;

                int prevChildIndex = -1;         // index of the prev child relative to this row
                int childRowIndex = _rowIndex;   // absolute row index of the current child 

                // we only need to consider child rows which have previously been expanded (and thus
                // cached) If a row hasn't been expanded then it can't have any effect of the first/last 
                // index of this row
                //
                // Copy the child rows so that we can handle re-entrant calls
                //
                SortedList childRows = _childRows.Clone() as SortedList;
                foreach (DictionaryEntry entry in childRows)
                {
                    Row childRow = (Row)entry.Value;
                    int childIndex = (int)entry.Key;

                    // determine what the starting index of this child row should be by working
                    // out how many items we have skipped since the previous cached child row.  
                    // 
                    childRowIndex = childIndex - prevChildIndex + childRowIndex;

                    // recursively reindex this child row
                    //
                    childRow.Reindex(childRowIndex);
            
                    // update the last child index
                    //
                    _lastChildRowIndex = childRowIndex;
            
                    // Move the row index to the last descendant index of this row
                    //
                    childRowIndex = childRow.LastDescendantRowIndex;
                    prevChildIndex = childIndex;
                }

                // calculate the the number of direct children of this row following the last
                // expanded child row.
                //
                int remainingChildren = NumChildren - prevChildIndex - 1;
                _lastDescendantRowIndex = childRowIndex + remainingChildren;
                if (remainingChildren > 0)
                {
                    _lastChildRowIndex = _lastDescendantRowIndex;
                }
            }

            // While these assertions should be true in general - they can be broken
            // by collections that implement IBindingList and fire ListChanged events
            // when the list is accessed.  In this case the breakage is only temporary
            // and has no lasting impact
            //
            // Debug.Assert(_lastChildRowIndex >= _rowIndex);
            // Debug.Assert(_lastDescendantRowIndex >= _lastChildRowIndex);

        }

        /// <summary>
        /// Calculates new LastDescendantRowIndex and LastChildIndex for this row when a child row is expanded or
        /// collapsed.  Reindexes each of the children following the given child row and recursively works back up the
        /// tree reindexing all children following the affected row. 
        /// </summary>
        /// <param name="startChildRow">The child row to start reindexing from</param>
        internal void ReindexFromChild(Row startChildRow)
        {
            // ignore reindex of rows that have been disposed - this can happen if there is rentrant
            // code that while reindexing causes a data update
            //
            if (Disposed) return;

            if (Expanded)
            {  
                // ensure that the child we are reindexing from is cached - it may not be
                // CacheAllChildren is set to false
                //
                _childRows[startChildRow.ChildIndex] = startChildRow;

                int prevChildIndex = startChildRow.ChildIndex;              // index of the prev child relative to this row
                int childRowIndex = startChildRow.LastDescendantRowIndex;   // absolute row index of the current child 

                // we only need to consider child rows which have previously been expanded (and thus
                // cached) If a row hasn't been expanded then it can't have any effect of the first/last 
                // index of this row
                //
                // Copy the child rows so that we can handle re-entrant calls
                //
                SortedList childRows = _childRows.Clone() as SortedList;
                foreach (DictionaryEntry entry in childRows)
                {
                    int childIndex = (int)entry.Key;

                    // skip any rows before the given child row (and the given child row)
                    //
                    if (childIndex > startChildRow.ChildIndex)
                    {
                        Row childRow = (Row)entry.Value;
                
                        // determine what the starting index of this child row should be by working
                        // out how many items we have skipped since the previous cached child row.  
                        // 
                        childRowIndex = childIndex - prevChildIndex + childRowIndex;

                        // recursively reindex this child row
                        //
                        childRow.Reindex(childRowIndex);
            
                        // update the last child index
                        //
                        _lastChildRowIndex = childRowIndex;
            
                        // Move the row index to the last descendant index of this row
                        //
                        childRowIndex = childRow.LastDescendantRowIndex;
                        prevChildIndex = childIndex;
                    }
                }

                // calculate the the number of direct children of this row following the last
                // expanded child row.
                //
                int remainingChildren = NumChildren - prevChildIndex - 1;
                _lastDescendantRowIndex = childRowIndex + remainingChildren;
                if (remainingChildren > 0)
                {
                    _lastChildRowIndex = _lastDescendantRowIndex;
                }

            }  
            else
            {
                // if not expanded
                _lastDescendantRowIndex = _rowIndex;
                _lastChildRowIndex = _rowIndex;
            }

            // Reindex the parent row starting from this row
            //
            if (_parentRow != null)
            {
                _parentRow.ReindexFromChild(this);
            }
        }

        /// <summary>
        /// Reindexes the tree starting from this row.  Rows above this row are unaffected.
        /// </summary>
        internal void Reindex()
        {
            Reindex(RowIndex);
            if (ParentRow != null)
            {
                ParentRow.ReindexFromChild(this);
            }
        }

        /// <summary>
        /// Dispose of any cached rows that are not required to support the current display 
        /// </summary>
        internal void DisposeUnusedRows()
        {
            // update the ChildIndices of the cached child rows 
            //
            SortedList childRows = new SortedList();
            foreach (DictionaryEntry entry in _childRows)
            {
                Row row = (Row)entry.Value;

                // If the row is not currently used by the tree and isn't visible
                // then dispose of it
                //
                if (row.CanDispose() && !row.Visible)
                {
                    row.Dispose();
                }
                else
                {
                    childRows[entry.Key] = row;
                    row.DisposeUnusedRows();
                }
            }
            _childRows = childRows;
        }

        /// <summary>
        /// Dispose of a row when it is no longer part of the tree hierarchy
        /// </summary>
        internal protected virtual void Dispose()
        {
            // check that the row has not already been disposed
            //
            if (!Disposed)
            {
                // inform the tree that row is being disposed of so it can release any references to the row
                //
                Tree.RowDisposed(this);

                // ##  Debug.WriteLine("Row Dispose");

                // detach from the child item list if necessary
                //
                UnloadChildItems();

                // dispose of any child rows
                //
                foreach (DictionaryEntry entry in _childRows)
                {
                    Row row = (Row)entry.Value;
                    row.Dispose();
                }

                // clear the child rows and release references
                //
                _childRows.Clear();
                _item = null;
                _childRows = null;
            }
        }


        #endregion

        #region Local Methods

        /// <summary>
        /// Handle a change to the list of children for this row
        /// </summary>
        /// <param name="sender">The list that originated the changed event</param>
        /// <param name="e">Details of the change</param>
        private void OnChildItemsChanged(object sender, ListChangedEventArgs e)
        {
            _tree.OnRowChildItemsChanged(this, sender, e);
        }

        /// <summary>
        /// Load the childItems by calling the tree to get the children for this item.  If the
        /// returned list supports binding then attach an event handler to be notified of changes.
        /// </summary>
        private void LoadChildItems()
        {
            Debug.Assert(!_childItemsLoaded);
            _childItems = _tree.GetChildrenForRow(this);
            if (_childItems is IBindingList)
            {
                ((IBindingList)_childItems).ListChanged += new ListChangedEventHandler(OnChildItemsChanged);
            }
            _childItemsLoaded = true;
        }

        /// <summary>
        /// Remove the IBindingList event handler (if any) from the current childItems.
        /// </summary>
        private void UnloadChildItems()
        {
            if (_childItems is IBindingList)
            {
                ((IBindingList)_childItems).ListChanged -= new ListChangedEventHandler(OnChildItemsChanged);
            }
            _childItems = null;
            _childItemsLoaded = false;
        }

        /// <summary>
        /// Checks whether the child items have been loaded and loads them if they haven't
        /// </summary>
        private void LoadChildItemsIfRequired()
        {
            if (!_childItemsLoaded)
            {
                LoadChildItems();
            }
        }

        /// <summary>
        /// Return the last cached row before the given row index.
        /// </summary>
        /// <param name="beforeRowIndex">The absolute row index to start at</param>
        /// <returns>The last cached child row before the given absolute index</returns>
        private Row FindCachedChildRowPriorToRowIndex(int beforeRowIndex)
        {
            Row prevRow = null;
            foreach (DictionaryEntry entry in _childRows)
            {
                Row row = (Row)entry.Value;
                if (row.RowIndex > beforeRowIndex) break;
                prevRow = row;
            }
            return prevRow;
        }

        /// <summary>
        /// Return the last cached row before the given relative child index.
        /// </summary>
        /// <param name="beforeChildIndex">The relative child index to start at</param>
        /// <returns>The last cached child row before the given child index</returns>
        private Row FindCachedChildRowPriorToChildIndex(int beforeChildIndex)
        {
            Row prevRow = null;
            foreach (DictionaryEntry entry in _childRows)
            {
                Row row = (Row)entry.Value;
                if (row.ChildIndex > beforeChildIndex) break;
                prevRow = row;
            }
            return prevRow;
        }

        /// <summary>
        /// Returns true if the row can be disposed. ie it is no longer directly referenced by the tree 
        /// (selected, expanded etc)
        /// </summary>
        /// <returns>True if this row can be disposed</returns>
        private bool CanDispose()
        {
            return !(Expanded || Selected || Visible || HasFocus || Editing);
        }

        /// <summary>
        /// Expand the children of this row without reindexing.   This allows the public
        /// ExpandChildren and ExpandDescendants methods to defer reindexing until all
        /// children of the row have been expanded. 
        /// </summary>
        /// <param name="recursive">If true recursively expands all descendants</param>
        private void ExpandChildrenNoReindex(bool recursive)
        {
            // ensure that we have loaded the child items for this row
            //
            LoadChildItemsIfRequired();
            if (_childItems == null) return;
           
            int rowIndex = this._rowIndex;
            for (int childIndex = 0; childIndex < _childItems.Count; childIndex++)
            {
                Row row = (Row)_childRows[childIndex];
                if (row == null)
                {
                    row = _tree.CreateRow(_childItems[childIndex], this, childIndex, rowIndex + 1);
                }

                // set this row to be expanded (without reindexing it)
                //
                row._expanded = true;
                _tree.OnRowExpand(this);
                if (recursive)
                {
                    row.ExpandChildrenNoReindex(recursive);
                }
                rowIndex = row.LastDescendantRowIndex;
            }
        }

        /// <summary>
        /// Collapse the children of this row without reindexing.   This allows the public
        /// CollapseChildren and CollapseDescendants methods to defer reindexing until all
        /// children of the row have been collapsed. 
        /// </summary>
        /// <param name="recursive">If true recursively collapses all descendants</param>
        private void CollapseChildrenNoReindex(bool recursive)
        {
            // we only have to iterate through the cached rows (if the row is not cached
            // it cannot be expanded)
            //
            foreach (DictionaryEntry entry in _childRows)
            {
                Row row = (Row)entry.Value;
                row._expanded = false;
                _tree.OnRowCollapse(row);
                if (recursive)
                {
                    row.CollapseChildrenNoReindex(recursive);
                }
            }
        }

        /// <summary>
        /// Update the children of this row following a change to the list of child items for this
        /// row without reindexing.  This allows the public UpdateChildren and UpdateDescendants 
        /// methods to defer reindexing until all children of the row have been updated. 
        /// </summary>
        /// <param name="reloadChildren">If true the children are reloaded</param>
        /// <param name="recursive">If true recursively updates all descendants</param>
        private void UpdateChildrenNoReindex(bool reloadChildren, bool recursive)
        {
            if (reloadChildren && _childItemsLoaded)
            {
                UnloadChildItems();
                LoadChildItems();
            }

            // ignore reindex of rows that have been disposed - this can happen if there is rentrant
            // code that while reindexing causes a data update
            //
            if (Disposed) return;

            // update the ChildIndices of the cached child rows 
            //
            SortedList childRows = new SortedList();

            // clone to avoid rentrancy issues
            //
            SortedList oldChildRows = _childRows.Clone() as SortedList;
            foreach (DictionaryEntry entry in oldChildRows)
            {
                Row row = (Row)entry.Value;

                // Only update rows that are expanded or referenced by the tree directly
                // Calling childItems.IndexOf for each row is potentially an expensive operation - so we want 
                // to minimise the number of calls.   This also provides an opportunity to dump rows that may 
                // have been displayed in the past but are no longer required
                //
                if (row.CanDispose())
                {
                    row.Dispose();
                }
                else
                {
                    int childIndex = -1;
                    if (_childItems != null)
                        childIndex = _childItems.IndexOf(row.Item);
                    if (childIndex >= 0)
                    {
                        row._childIndex = childIndex;
                        childRows[childIndex] = row;

                        if (recursive)
                        {
                            row.UpdateChildrenNoReindex(reloadChildren, recursive);
                        }
                    }
                    else
                    {
                        // the child item this row was associated with no longer exists so
                        // dispose of the row
                        //
                        row.Dispose();
                    }
                }
            }
            _childRows = childRows;
        }


        #endregion


    }
}
