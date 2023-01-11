/******************************************************************************
Excel.cs

begin		: 2010-02-12
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Data;

namespace ApplicationUtility
{
    /// <summary>
    /// A singleton Excel object, responsible for loading workbooks. 
    /// </summary>
    public class Excel : IExcel
    {
        #region Static

        #region Workbook

        /// <summary>
        /// An Excel workbook.
        /// </summary>
        public class Workbook : IWorkbook, IDirectorReferenceCounter
        {
            #region Static

            /// <summary>
            /// An Excel table.
            /// </summary>
            /// <remarks>
            /// A table is a grid of cells.
            /// The first non-null row is numbered 0,
            /// and numbering continues to the last non-null row.
            /// Within this, the first non-null column is numbered 0,
            /// and numbering continues to the last non-null column.
            /// </remarks>
            public class Table : IWorkbookTable, IDirectorReferenceCounter
            {
                #region Static

                /// <summary>
                /// Sets the value to a string type and sets its string data.
                /// </summary>
                private static void SetStringValue(PYXValue value, String data)
                {
                    value.setType(PYXValue.eType.knString);
                    value.setString(data);
                    System.Diagnostics.Debug.Assert(value.getType() == PYXValue.eType.knString);
                    System.Diagnostics.Debug.Assert(value.getString() == data);
                }

                /// <summary>
                /// Reads the value at the row and column and populates the value,
                /// or returns false if unsuccessful.
                /// </summary>
                private static bool ReadValue(DataRow dataRow, DataColumn dataColumn, PYXValue value)
                {
                    object rawValue = dataRow[dataColumn.Ordinal];
                    Type dataType = dataColumn.DataType;

                    if (rawValue.Equals(DBNull.Value))
                    {
                        value.setType(PYXValue.eType.knNull);
                        return true;
                    }

                    try
                    {
                        if (!dataType.Equals(typeof(String)))
                        {
                            try
                            {
                                if (dataType.Equals(typeof(Int32)))
                                {
                                    int data = (int)rawValue;
                                    value.setType(PYXValue.eType.knInt32);
                                    value.setInt32(data);
                                    System.Diagnostics.Debug.Assert(value.getType() == PYXValue.eType.knInt32);
                                    System.Diagnostics.Debug.Assert(value.getInt32() == data);
                                    return true;
                                }
                                if (dataType.Equals(typeof(Double)))
                                {
                                    double data = (double)rawValue;
                                    value.setType(PYXValue.eType.knDouble);
                                    value.setDouble(data);
                                    System.Diagnostics.Debug.Assert(value.getType() == PYXValue.eType.knDouble);
                                    System.Diagnostics.Debug.Assert(value.getDouble() == data);
                                    return true;
                                }
                                if (dataType.Equals(typeof(DateTime)))
                                {
                                    SetStringValue(value, ((DateTime)rawValue).ToString());
                                    return true;
                                }
                                System.Diagnostics.Trace.WriteLine(
                                    String.Format(
                                        "Unsupported type \"{0}\"; attempting to cast to string.",
                                        dataType.ToString()));
                            }
                            catch (Exception e)
                            {
                                System.Diagnostics.Trace.WriteLine(
                                    String.Format(
                                        "Exception \"{0}\" when casting to \"{1}\"; attempting to cast to string.",
                                        e.Message,
                                        dataType.ToString()));
                            }
                        }
                        SetStringValue(value, (String)rawValue);
                        return true;
                    }
                    catch (Exception)
                    {
                        System.Diagnostics.Trace.WriteLine(
                            "Could not cast the data value to string; setting to null.");
                    }
                    value.setType(PYXValue.eType.knNull);
                    return false;
                }

                /// <summary>
                /// A view on an Excel table, having a heading row and one or more data rows.
                /// </summary>
                public class View : IWorkbookView, IDirectorReferenceCounter
                {
                    #region Static

                    /// <summary>
                    /// An immutable set of unique defining parameters of a view.
                    /// </summary>
                    public class Definition
                    {
                        /// <summary>
                        /// The first 0-based column offset.
                        /// </summary>
                        private readonly uint m_firstColumnOffset;

                        /// <summary>
                        /// The last 0-based column offset.
                        /// </summary>
                        private readonly uint m_lastColumnOffset;

                        /// <summary>
                        /// The 0-based heading row offset.
                        /// </summary>
                        private readonly uint m_headingRowOffset;

                        /// <summary>
                        /// The first 0-based data row offset.
                        /// </summary>
                        private readonly uint m_firstDataRowOffset;

                        /// <summary>
                        /// The last 0-based data row offset.
                        /// </summary>
                        private readonly uint m_lastDataRowOffset;

                        /// <summary>
                        /// Constructs a workbook view definition.
                        /// </summary>
                        /// <param name="firstColumnOffset">The first 0-based column offset.</param>
                        /// <param name="lastColumnOffset">The last 0-based column offset.</param>
                        /// <param name="headingRowOffset">The 0-based heading row offset.</param>
                        /// <param name="firstDataRowOffset">The first 0-based data row offset.</param>
                        /// <param name="lastDataRowOffset">The last 0-based data row offset.</param>
                        public Definition(
                            uint firstColumnOffset, uint lastColumnOffset,
                            uint headingRowOffset,
                            uint firstDataRowOffset, uint lastDataRowOffset)
                        {
                            m_firstColumnOffset = firstColumnOffset;
                            m_lastColumnOffset = lastColumnOffset;
                            m_headingRowOffset = headingRowOffset;
                            m_firstDataRowOffset = firstDataRowOffset;
                            m_lastDataRowOffset = lastDataRowOffset;
                        }

                        /// <summary>
                        /// Gets the first 0-based column offset.
                        /// </summary>
                        public uint FirstColumnOffset
                        {
                            get { return m_firstColumnOffset; }
                        }

                        /// <summary>
                        /// Gets the last 0-based column offset.
                        /// </summary>
                        public uint LastColumnOffset
                        {
                            get { return m_lastColumnOffset; }
                        }

                        /// <summary>
                        /// Gets the 0-based heading row offset.
                        /// </summary>
                        public uint HeadingRowOffset
                        {
                            get { return m_headingRowOffset; }
                        }

                        /// <summary>
                        /// Gets the first 0-based data row offset.
                        /// </summary>
                        public uint FirstDataRowOffset
                        {
                            get { return m_firstDataRowOffset; }
                        }

                        /// <summary>
                        /// Gets the last 0-based data row offset.
                        /// </summary>
                        public uint LastDataRowOffset
                        {
                            get { return m_lastDataRowOffset; }
                        }

                        /// <summary>
                        /// Returns true if the definitions are identical.
                        /// </summary>
                        public bool Equals(Definition definition)
                        {
                            if (definition == null) return false;
                            if (definition == this) return true;
                            return
                                FirstColumnOffset.Equals(definition.FirstColumnOffset) &&
                                LastColumnOffset.Equals(definition.LastColumnOffset) &&
                                HeadingRowOffset.Equals(definition.HeadingRowOffset) &&
                                FirstDataRowOffset.Equals(definition.FirstDataRowOffset) &&
                                LastDataRowOffset.Equals(definition.LastDataRowOffset);
                        }

                        #region Object

                        /// <summary>
                        /// Returns a hash code for the definition.
                        /// </summary>
                        public override int GetHashCode()
                        {
                            const int hashMultiplier = 31;

                            int hash = GetType().GetHashCode();
                            hash = (hash * hashMultiplier) + FirstColumnOffset.GetHashCode();
                            hash = (hash * hashMultiplier) + LastColumnOffset.GetHashCode();
                            hash = (hash * hashMultiplier) + HeadingRowOffset.GetHashCode();
                            hash = (hash * hashMultiplier) + FirstDataRowOffset.GetHashCode();
                            hash = (hash * hashMultiplier) + LastDataRowOffset.GetHashCode();
                            return hash;
                        }

                        /// <summary>
                        /// Returns true if the parameter represents an identical definition.
                        /// </summary>
                        public override bool Equals(object obj)
                        {
                            return Equals(obj as Definition);
                        }

                        #endregion
                    }

                    #endregion

                    #region Fields

                    /// <summary>
                    /// The source table.
                    /// </summary>
                    private readonly Table m_table;

                    /// <summary>
                    /// The definition for this view.
                    /// </summary>
                    private readonly Definition m_definition;

                    /// <summary>
                    /// The underlying data view.
                    /// </summary>
                    private readonly DataView m_dataView;

                    /// <summary>
                    /// The underlying heading row.
                    /// </summary>
                    private readonly DataRow m_headingRow;

                    /// <summary>
                    /// The first 0-based column offset.
                    /// </summary>
                    private readonly int m_firstColumnOffset;

                    /// <summary>
                    /// The last 0-based column offset.
                    /// </summary>
                    private readonly int m_lastColumnOffset;

                    #endregion

                    /// <summary>
                    /// Constructs a view on a table according to the definition.
                    /// </summary>
                    public View(Table table, Definition definition)
                    {
                        if (table == null)
                        {
                            throw new ArgumentNullException("table");
                        }
                        if (definition == null)
                        {
                            throw new ArgumentNullException("definition");
                        }

                        m_table = table;
                        m_definition = definition;

                        DataTable dataTable = m_table.DataTable;
                        System.Diagnostics.Debug.Assert(dataTable != null,
                            "The Table.DataTable property should never return null.");

                        // Check column offsets.
                        int columnCount = dataTable.Columns.Count;
                        if (columnCount <= definition.FirstColumnOffset)
                        {
                            throw new ArgumentException(
                                String.Format(
                                    "The first column offset exceeds {0} (the number of columns in the table).",
                                    columnCount),
                                "definition");
                        }
                        if (columnCount <= definition.LastColumnOffset)
                        {
                            throw new ArgumentException(
                                String.Format(
                                    "The last column offset exceeds {0} (the number of columns in the table).",
                                    columnCount),
                                "definition");
                        }
                        if (definition.LastColumnOffset < definition.FirstColumnOffset)
                        {
                            throw new ArgumentException(
                                "The last column offset cannot be less than the first.",
                                "definition");
                        }

                        // Check row offsets.
                        int rowCount = dataTable.Rows.Count;
                        if (rowCount <= definition.HeadingRowOffset)
                        {
                            throw new ArgumentException(
                                String.Format(
                                    "The heading row offset exceeds {0} (the number of rows in the table).",
                                    rowCount),
                                "definition");
                        }

                        // Get the index column name.
                        int indexColumnOffset = (0 < columnCount) ? (columnCount - 1) : 0;
                        String indexColumnName = dataTable.Columns[indexColumnOffset].ColumnName;

                        // Build the row filter.
                        // Note: we are no longer use the max row to allow tables to grow
                        string rowFilter = String.Format(
                            //"{0} >= '{1}' AND {0} <= '{2}'",
                            "{0} >= '{1}'",
                            indexColumnName,
                            definition.FirstDataRowOffset);
                            //, definition.LastDataRowOffset);

                        // Construct and set the view.
                        m_dataView = new DataView(dataTable,
                            rowFilter, indexColumnName, DataViewRowState.CurrentRows);

                        // Set the headings.
                        // Don't include row index column.
                        m_headingRow = dataTable.Rows[(int)definition.HeadingRowOffset];

                        // Set the column range.
                        m_firstColumnOffset = (int)definition.FirstColumnOffset;
                        m_lastColumnOffset = (int)definition.LastColumnOffset;
                    }

                    /// <summary>
                    /// Gets the number of data rows.
                    /// </summary>
                    private int DataRowCount
                    {
                        get { return m_dataView.Count; }
                    }

                    /// <summary>
                    /// Gets the number of columns.
                    /// </summary>
                    private int ColumnCount
                    {
                        get { return m_lastColumnOffset - m_firstColumnOffset + 1; }
                    }

                    /// <summary>
                    /// Gets the underlying data view.
                    /// </summary>
                    public DataView DataView
                    {
                        get { return m_dataView; }
                    }

                    /// <summary>
                    /// Returns true if the data row is empty.
                    /// </summary>
                    private bool IsDataRowEmpty(DataRowView dataRow)
                    {
                        for (int columnOffset = m_firstColumnOffset;
                            columnOffset <= m_lastColumnOffset;
                            ++columnOffset)
                        {
                            if (!dataRow.Row.IsNull(columnOffset))
                            {
                                return false;
                            }
                        }
                        return true;
                    }

                    #region IWorkbookView

                    /// <summary>
                    /// Returns true if the view is empty.
                    /// </summary>
                    /// <returns></returns>
                    public override bool IsEmpty()
                    {
                        // Get the column and row count.
                        // If either is 0, this is an empty table.
                        int columnCount = ColumnCount;
                        int dataRowCount = DataRowCount;
                        if (columnCount == 0 || dataRowCount == 0)
                        {
                            return true;
                        }

                        // If all the rows are null, return true; otherwise, return false.
                        foreach (DataRowView dataRow in m_dataView)
                        {
                            if (!IsDataRowEmpty(dataRow))
                            {
                                return false;
                            }
                        }
                        return true;
                    }

                    /// <summary>
                    /// Returns the number of columns in the view.
                    /// </summary>
                    public override uint GetColumnCount()
                    {
                        return (uint)ColumnCount;
                    }

                    /// <summary>
                    /// Returns the name of the column at the 0-based offset.
                    /// For all columns except the row index column, this differs from the heading,
                    /// which is given by a row in the table.
                    /// </summary>
                    /// <param name="columnOffset">The 0-based column offset.</param>
                    /// <param name="value">The value to populate.</param>
                    public override void GetColumnName(uint columnOffset, PYXValue value)
                    {
                        int columnCount = ColumnCount;
                        if (columnCount <= columnOffset)
                        {
                            throw new ArgumentOutOfRangeException("columnOffset",
                                String.Format(
                                    "The column offset exceeds {0} (the number of columns in the view).",
                                    columnCount));
                        }

                        DataColumn dataColumn = m_dataView.Table.Columns[m_firstColumnOffset + (int)columnOffset];
                        value.setType(PYXValue.eType.knString);
                        value.setString(dataColumn.ColumnName);
                    }

                    /// <summary>
                    /// Gets the values from the heading row, or returns false if unsuccessful.
                    /// </summary>
                    public override bool GetHeadingRow(Vector_Value columns)
                    {
                        // Allocate enough columns in the output column vector.
                        int columnCount = ColumnCount;
                        while (columns.Count < columnCount)
                        {
                            columns.Add(new PYXValue());
                        }

                        bool hasValidData = false;

                        for (uint columnOffset = 0;
                            columnOffset < columnCount;
                            ++columnOffset)
                        {
                            if (GetHeadingCell(columnOffset, columns[(int)columnOffset]))
                            {
                                hasValidData = true;
                            }
                        }

                        return hasValidData;
                    }

                    /// <summary>
                    /// Gets a value at the 0-based column offset in the heading row, or returns false if unsuccessful.
                    /// For the row index column, gets the column name.
                    /// </summary>
                    /// <param name="columnOffset">The 0-based column offset in the view.</param>
                    /// <param name="value">The value to populate.</param>
                    /// <returns>Whether or not the heading value could be retrieved.</returns>
                    public override bool GetHeadingCell(uint columnOffset, PYXValue value)
                    {
                        int columnCount = ColumnCount;
                        if (columnCount <= columnOffset)
                        {
                            throw new ArgumentOutOfRangeException("columnOffset",
                                String.Format(
                                    "The column offset exceeds {0} (the number of columns in the view).",
                                    columnCount));
                        }

                        int dataColumnOffset = m_firstColumnOffset + (int)columnOffset;
                        if (dataColumnOffset + 1 == m_dataView.Table.Columns.Count)
                        {
                            // It's the index column; return its caption.
                            value.setType(PYXValue.eType.knString);
                            value.setString(m_dataView.Table.Columns[dataColumnOffset].Caption);
                            return true;
                        }
                        return ReadValue(m_headingRow,
                            m_dataView.Table.Columns[dataColumnOffset], value);
                    }

                    /// <summary>
                    /// Returns the number of data rows in the view.
                    /// </summary>
                    public override uint GetDataRowCount()
                    {
                        return (uint)DataRowCount;
                    }

                    /// <summary>
                    /// Returns true if the data row is empty (not counting the row index).
                    /// </summary>
                    public override bool IsDataRowEmpty(uint dataRowOffset)
                    {
                        int dataRowCount = DataRowCount;
                        if (dataRowCount <= dataRowOffset)
                        {
                            throw new ArgumentOutOfRangeException("dataRowOffset",
                                String.Format(
                                    "The data row offset exceeds {0} (the number of data rows in the view).",
                                    dataRowCount));
                        }

                        return IsDataRowEmpty(m_dataView[(int)dataRowOffset]);
                    }

                    /// <summary>
                    /// Gets the values from the data row at the 0-based offset, or returns false if unsuccessful.
                    /// </summary>
                    public override bool GetDataRow(uint dataRowOffset, Vector_Value columns)
                    {
                        int dataRowCount = DataRowCount;
                        if (dataRowCount <= dataRowOffset)
                        {
                            throw new ArgumentOutOfRangeException("dataRowOffset",
                                String.Format(
                                    "The data row offset exceeds {0} (the number of data rows in the view).",
                                    dataRowCount));
                        }

                        // Get the data row.
                        DataRow dataRow = m_dataView[(int)dataRowOffset].Row;

                        // Allocate enough columns in the output column vector.
                        int columnCount = ColumnCount;
                        while (columns.Count < columnCount)
                        {
                            columns.Add(new PYXValue());
                        }

                        bool hasValidData = false;

                        int columnOffset = 0;
                        for (int dataColumnOffset = (int)m_firstColumnOffset;
                            dataColumnOffset <= m_lastColumnOffset;
                            ++dataColumnOffset)
                        {
                            DataColumn dataColumn = m_dataView.Table.Columns[dataColumnOffset];
                            if (ReadValue(dataRow, dataColumn, columns[columnOffset]))
                            {
                                hasValidData = true;
                            }
                            ++columnOffset;
                        }

                        return hasValidData;
                    }

                    /// <summary>
                    /// Gets the value at the 0-based column offset in the data row at the 0-based offset, or returns false.
                    /// </summary>
                    /// <param name="dataRowOffset">The 0-based data row offset in the view.</param>
                    /// <param name="columnOffset">The 0-based column offset in the view.</param>
                    /// <param name="value">The value to populate.</param>
                    /// <returns>Whether or not the data value could be retrieved.</returns>
                    public override bool GetDataCell(uint dataRowOffset, uint columnOffset, PYXValue value)
                    {
                        int dataRowCount = DataRowCount;
                        if (dataRowCount <= dataRowOffset)
                        {
                            throw new ArgumentOutOfRangeException("dataRowOffset",
                                String.Format(
                                    "The data row offset exceeds {0} (the number of data rows in the view).",
                                    dataRowCount));
                        }

                        int columnCount = ColumnCount;
                        if (columnCount <= columnOffset)
                        {
                            throw new ArgumentOutOfRangeException("columnOffset",
                                String.Format(
                                    "The column offset exceeds {0} (the number of columns in the view).",
                                    columnCount));
                        }

                        DataRow dataRow = m_dataView[(int)dataRowOffset].Row;
                        DataColumn dataColumn = m_dataView.Table.Columns[m_firstColumnOffset + (int)columnOffset];
                        return ReadValue(dataRow, dataColumn, value);
                    }

                    #region IDirectorReferenceCounter Members

                    public void setSwigCMemOwn(bool value)
                    {
                        swigCMemOwn = value;
                    }

                    public int doAddRef()
                    {
                        return base.addRef();
                    }

                    public int doRelease()
                    {
                        return base.release();
                    }

                    #endregion

                    #region PYXObject

                    /// <summary>
                    /// Override the reference-counting addRef.  This is not called 
                    /// directly!
                    /// </summary>
                    /// <returns>Current reference count (after increment).</returns>
                    public override int addRef()
                    {
                        if (getCPtr(this).Handle == IntPtr.Zero)
                        {
                            return 1;
                        }
                        // Increment reference count.
                        int referenceCount = ReferenceManager.addRef(this);
                        
                        // If the reference count is now 2, that means that an unmanaged
                        // reference now exists.  Create a managed reference to this 
                        // object, to ensure that it will survive at least as long 
                        // as the unmanaged reference.
                        if (referenceCount == 2)
                        {
                            lock (this.m_table.m_views)
                            {
                                this.m_table.m_views.Add(this.m_definition, this);
                            }
                        }

                        return referenceCount;
                    }

                    /// <summary>
                    /// Override the reference-counting release.  This is not called 
                    /// directly!
                    /// </summary>
                    /// <returns>Current reference count (after decrement).</returns>
                    public override int release()
                    {
                        if (getCPtr(this).Handle == IntPtr.Zero)
                        {
                            return 1;
                        }
                        // If this has not been disposed, decrement the reference count.
                        int referenceCount = ReferenceManager.release(this);
                        
                        // If the last unmanaged reference has been released.
                        // Remove the managed reference to this object,
                        // which may lead to the object's garbage collection.
                        if (referenceCount < 2)
                        {
                            lock (this.m_table.m_views)
                            {
                                this.m_table.m_views.Remove(this.m_definition);
                            }
                            System.Diagnostics.Debug.Assert(0 < referenceCount,
                                "The reference count should never get to 0 while there is a managed reference.");
                            referenceCount = 1;
                        }

                        return referenceCount;
                    }

                    #endregion

                    #endregion
                }

                /// <summary>
                /// Returns true if the row is empty.
                /// </summary>
                private static bool IsRowEmpty(DataRow dataRow)
                {
                    // Don't count the row index column (which is last).
                    int columnCountInRow = dataRow.ItemArray.Length - 1;
                    while (0 < columnCountInRow)
                    {
                        if (!dataRow.IsNull(--columnCountInRow))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                // Returns the rows of the table starting at the first top-left non-null
                // and ending at the bottom-right non-null.
                private static string BuildQuery(String tableName)
                {
                    return String.Format("SELECT * FROM [{0}]", tableName);
                }

                #endregion

                /// <summary>
                /// The source workbook.
                /// </summary>
                private readonly Workbook m_workbook;

                /// <summary>
                /// The table name.
                /// </summary>
                private readonly String m_name;

                /// <summary>
                /// The underlying data table.
                /// </summary>
                private readonly DataTable m_dataTable;

                /// <summary>
                /// A dictionary mapping view definition to view.
                /// </summary>
                private readonly Dictionary<View.Definition, View> m_views =
                    new Dictionary<View.Definition, View>();

                /// <summary>
                /// Constructs a table with an auto-generated row index column on the end.
                /// </summary>
                public Table(Workbook workbook, String name)
                {
                    if (workbook == null)
                    {
                        throw new ArgumentNullException("workbook");
                    }
                    if (name == null)
                    {
                        throw new ArgumentNullException("tableName");
                    }

                    m_workbook = workbook;
                    m_name = name;
                    m_dataTable = workbook.m_dataSource.RunQuery(BuildQuery(name));

                    // Add "row" column to the end, containing row offsets.
                    {
                        int dataColumnOffset = m_dataTable.Columns.Count;

                        DataColumn dataColumn = new DataColumn();
                        dataColumn.DataType = System.Type.GetType("System.Int32");
                        dataColumn.Caption = "Row";
                        m_dataTable.Columns.Add(dataColumn);
                        System.Diagnostics.Debug.Assert(dataColumn.Ordinal == dataColumnOffset);

                        int dataRowOffset = 0;
                        foreach (DataRow dataRow in m_dataTable.Rows)
                        {
                            dataRow[dataColumnOffset] = dataRowOffset;
                            ++dataRowOffset;
                        }
                    }
                }

                /// <summary>
                /// Gets the underlying data table.
                /// </summary>
                public DataTable DataTable
                {
                    get { return m_dataTable; }
                }

                /// <summary>
                /// Returns a view on the table, or throws if unsuccessful.
                /// </summary>
                /// <param name="firstColumnOffset">The first 0-based column offset.</param>
                /// <param name="lastColumnOffset">The last 0-based column offset.</param>
                /// <param name="headingRowOffset">The 0-based heading row offset.</param>
                /// <param name="firstDataRowOffset">The first 0-based data row offset.</param>
                /// <param name="lastDataRowOffset">The last 0-based data row offset.</param>
                /// <returns></returns>
                public View GetView(
                    uint firstColumnOffset, uint lastColumnOffset,
                    uint headingRowOffset,
                    uint firstDataRowOffset, uint lastDataRowOffset)
                {
                    View.Definition definition = new View.Definition(
                        firstColumnOffset, lastColumnOffset,
                        headingRowOffset,
                        firstDataRowOffset, lastDataRowOffset);

                    lock (m_views)
                    {
                        View view;
                        if (m_views.TryGetValue(definition, out view))
                        {
                            return view;
                        }
                    }
                    return new View(this, definition);
                }

                #region Object

                public override string ToString()
                {
                    return m_name;
                }

                #endregion

                #region IWorkbookTable

                public override IWorkbookViewPointer CreateView(
                    uint firstColumnOffset, uint lastColumnOffset,
                    uint headingRowOffset,
                    uint firstDataRowOffset, uint lastDataRowOffset)
                {
                    return new IWorkbookViewPointer(
                        GetView(
                            firstColumnOffset, lastColumnOffset,
                            headingRowOffset,
                            firstDataRowOffset, lastDataRowOffset));
                }

                public override IWorkbookViewPointer CreateDefaultView()
                {
                    //note that the last columns is a fake column with row index.
                    uint columnsCount = (uint)this.DataTable.Columns.Count;
                    if (columnsCount > 1) columnsCount--;

                    uint rowCount = (uint)this.DataTable.Rows.Count;
                    if (rowCount == 0) rowCount = 1;

                    //we assume heading row is row #0.
                    return new IWorkbookViewPointer(
                        GetView(0, columnsCount - 1, 0, 1, rowCount - 1));
                }

                /// <summary>
                /// Returns the number of rows. 
                /// </summary>
                /// <remarks>
                /// Can throw.
                /// </remarks>
                public override uint GetRowCount()
                {
                    return (uint)m_dataTable.Rows.Count;
                }

                /// <summary>
                /// Returns the number of columns.
                /// </summary>
                /// <remarks>
                /// Can throw.
                /// </remarks>
                public override uint GetColumnCount()
                {
                    return (uint)m_dataTable.Columns.Count;
                }

                /// <summary>
                /// Gets the value at the 0-based column offset in the row at the 0-based offset, or returns false.
                /// </summary>
                /// <param name="rowOffset">The 0-based row offset in the view.</param>
                /// <param name="columnOffset">The 0-based column offset in the view.</param>
                /// <param name="value">The value to populate.</param>
                /// <returns>Whether or not the data value could be retrieved.</returns>
                public override bool GetCell(uint rowOffset, uint columnOffset, PYXValue value)
                {
                    int rowCount = m_dataTable.Rows.Count;
                    if (rowCount <= rowOffset)
                    {
                        throw new ArgumentOutOfRangeException("rowOffset",
                            String.Format(
                                "The row offset exceeds {0} (the number of rows in the table).",
                                rowCount));
                    }

                    int columnCount = m_dataTable.Columns.Count;
                    if (columnCount <= columnOffset)
                    {
                        throw new ArgumentOutOfRangeException("columnOffset",
                            String.Format(
                                "The column offset exceeds {0} (the number of columns in the table).",
                                columnCount));
                    }

                    DataRow dataRow = m_dataTable.Rows[(int)rowOffset];
                    DataColumn dataColumn = m_dataTable.Columns[(int)columnOffset];
                    return ReadValue(dataRow, dataColumn, value);
                }

                /// <summary>
                /// Returns true if the table is empty.
                /// Doesn't count the generated row indices.
                /// </summary>
                /// <remarks>
                /// Can throw.
                /// </remarks>
                public override bool IsEmpty()
                {
                    // Get the column and row count.
                    // If either is 0, this is an empty table.
                    int columnCount = m_dataTable.Columns.Count;
                    int rowCount = m_dataTable.Rows.Count;
                    if (columnCount == 0 || rowCount == 0)
                    {
                        return true;
                    }

                    // If all the rows are null, return true; otherwise, return false.
                    foreach (DataRow dataRow in m_dataTable.Rows)
                    {
                        if (!IsRowEmpty(dataRow))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                /// <summary>
                /// Returns true if the row at the 0-based offset is empty.
                /// Doesn't count the generated row index.
                /// </summary>
                /// <remarks>
                /// Can throw.
                /// </remarks>
                public override bool IsRowEmpty(uint rowOffset)
                {
                    int rowCount = m_dataTable.Rows.Count;
                    if (rowCount <= rowOffset)
                    {
                        throw new ArgumentOutOfRangeException("rowOffset",
                            String.Format(
                                "The row offset must be less than the row count ({0})",
                                rowCount));
                    }

                    return IsRowEmpty(m_dataTable.Rows[(int)rowOffset]);
                }

                #region IDirectorReferenceCounter Members

                public void setSwigCMemOwn(bool value)
                {
                    swigCMemOwn = value;
                }

                public int doAddRef()
                {
                    return base.addRef();
                }

                public int doRelease()
                {
                    return base.release();
                }

                #endregion

                #region PYXObject

                /// <summary>
                /// Override the reference-counting addRef.  This is not called 
                /// directly!
                /// </summary>
                /// <returns>Current reference count (after increment).</returns>
                public override int addRef()
                {
                    if (getCPtr(this).Handle == IntPtr.Zero)
                    {
                        return 1;
                    }
                    // Increment reference count.
                    int referenceCount = ReferenceManager.addRef(this);
                    
                    // If the reference count is now 2, that means that an unmanaged
                    // reference now exists.  Create a managed reference to this 
                    // object, to ensure that it will survive at least as long 
                    // as the unmanaged reference.
                    if (referenceCount == 2)
                    {
                        lock (this.m_workbook.m_tables)
                        {
                            this.m_workbook.m_tables.Add(this.ToString(), this);
                        }
                    }

                    return referenceCount;
                }

                /// <summary>
                /// Override the reference-counting release.  This is not called 
                /// directly!
                /// </summary>
                /// <returns>Current reference count (after decrement).</returns>
                public override int release()
                {
                    if (getCPtr(this).Handle == IntPtr.Zero)
                    {
                        return 1;
                    }
                    // If this has not been disposed, decrement the reference count.
                    int referenceCount = ReferenceManager.release(this);

                    // If the last unmanaged reference has been released.
                    // Remove the managed reference to this object,
                    // which may lead to the object's garbage collection.
                    if (referenceCount < 2)
                    {
                        lock (this.m_workbook.m_tables)
                        {
                            this.m_workbook.m_tables.Remove(this.ToString());
                        }
                        System.Diagnostics.Debug.Assert(0 < referenceCount,
                            "The reference count should never get to 0 while there is a managed reference.");
                        referenceCount = 1;
                    }

                    return referenceCount;
                }

                #endregion

                #endregion
            }

            #endregion

            #region Fields

            /// <summary>
            /// The OLE DB data source.
            /// </summary>
            private readonly OleDbDataSource m_dataSource;

            /// <summary>
            /// A dictionary mapping table name to table.
            /// </summary>
            private readonly Dictionary<string, Table> m_tables =
                new Dictionary<string, Table>();

            #endregion

            #region Construction

            /// <summary>
            /// Opens the workbook, or throws if none or there is a problem.
            /// </summary>
            public Workbook(System.IO.FileInfo fileInfo)
            {
                string connectionString;
                switch (fileInfo.Extension.ToUpper())
                {
                    case ".XLS":
                        connectionString = string.Format(
                            "Provider=Microsoft.Jet.OLEDB.4.0;Data Source={0};Extended Properties=\"Excel 8.0;MODE=READ;READONLY=TRUE;IMEX=1;HDR=NO\"",
                            fileInfo.ToString());
                        break;
                    case ".XLSX":
                        connectionString = string.Format(
                            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source={0};Extended Properties=\"Excel 12.0 Xml;MODE=READ;READONLY=TRUE;IMEX=1;HDR=NO\"",
                            fileInfo.ToString());
                        break;
                    default:
                        throw new NotAnExcelFileException(
                            String.Format("The file \"{0}\" is not a valid Excel file.",
                                fileInfo.ToString()));
                }

                try
                {
                    this.m_dataSource = new OleDbDataSource(fileInfo, connectionString);
                }
                catch (System.InvalidOperationException ex)
                {
                    throw new Excel2007OleDbDriverNotInstalledException(ex.ToString());
                }
                catch (Exception ex)
                {
                    throw new UnknownOleDbConnectException(ex.ToString());
                }
            }

            #endregion

            #region Object

            /// <summary>
            /// Returns the full file path for the workbook.
            /// </summary>
            public override string ToString()
            {
                return m_dataSource.FileInfo.FullName;
            }

            /// <summary>
            /// Returns the hash code for the workbook, based on the file info.
            /// </summary>
            public override int GetHashCode()
            {
                return m_dataSource.FileInfo.FullName.GetHashCode();
            }

            /// <summary>
            /// Returns true if the workbooks are identical, using the same file.
            /// </summary>
            public override bool Equals(object obj)
            {
                return Equals(obj as Workbook);
            }

            #endregion

            #region IDisposable

            /// <summary>
            /// Disposes the workbook.
            /// </summary>
            public override void Dispose()
            {
                // This can be called before the constructor is finished.
                if (this.m_dataSource != null)
                {
                    this.m_dataSource.Dispose();
                }

                base.Dispose();
            }

            #endregion

            #region IWorkbook

            /// <summary>
            /// Returns the name of the workbook.
            /// </summary>
            public override String GetName()
            {
                return m_dataSource.FileInfo.Name;
            }

            /// <summary>
            /// Gets the names of sheets and named ranges within the workbook.
            /// </summary>
            public override void GetTableNames(Vector_String tableNames)
            {
                DataTable dataTable = m_dataSource.GetTables();

                // Resize output vector as necessary.
                int tableNameCount = dataTable.Rows.Count;
                while (tableNames.Count < tableNameCount)
                {
                    tableNames.Add("");
                }

                int tableNameOffset = 0;
                foreach (DataRow dataRow in dataTable.Rows)
                {
                    string tableName = dataRow["TABLE_NAME"].ToString();

                    // Dequote and unescape inner quotes.
                    if (tableName.StartsWith("'") && tableName.EndsWith("'"))
                    {
                        tableName = tableName.Substring(1, tableName.Length - 2).Replace("''", "'");
                    }

                    tableNames[tableNameOffset] = tableName;
                    ++tableNameOffset;
                }
            }

            public override IWorkbookTablePointer CreateTable(string tableName)
            {
                return new IWorkbookTablePointer(GetTable(tableName));
            }            

            #region IDirectorReferenceCounter Members

            public void setSwigCMemOwn(bool value)
            {
                swigCMemOwn = value;
            }

            public int doAddRef()
            {
                return base.addRef();
            }

            public int doRelease()
            {
                return base.release();
            }

            #endregion

            #region PYXObject

            /// <summary>
            /// Override the reference-counting addRef.  This is not called 
            /// directly!
            /// </summary>
            /// <returns>Current reference count (after increment).</returns>
            public override int addRef()
            {
                if (getCPtr(this).Handle == IntPtr.Zero)
                {
                    return 1;
                }

                // Increment reference count.
                int referenceCount = ReferenceManager.addRef(this);
                
                // If the reference count is now 2, that means that an unmanaged
                // reference now exists.  Create a managed reference to this 
                // object, to ensure that it will survive at least as long 
                // as the unmanaged reference.
                if (referenceCount == 2)
                {
                    lock (Excel.Instance.m_workbooks)
                    {
                        Excel.Instance.m_workbooks.Add(this.ToString(), this);
                    }
                }

                return referenceCount;
            }

            /// <summary>
            /// Override the reference-counting release.  This is not called 
            /// directly!
            /// </summary>
            /// <returns>Current reference count (after decrement).</returns>
            public override int release()
            {
                if (getCPtr(this).Handle == IntPtr.Zero)
                {
                    return 1;
                }
                
                int referenceCount = ReferenceManager.release(this);

                // If the last unmanaged reference has been released.
                // Remove the managed reference to this object,
                // which may lead to the object's garbage collection.
                if (referenceCount < 2)
                {
                    lock (Excel.Instance.m_workbooks)
                    {
                        Excel.Instance.m_workbooks.Remove(this.ToString());
                    }
                    System.Diagnostics.Debug.Assert(0 < referenceCount,
                        "The reference count should never get to 0 while there is a managed reference.");
                    referenceCount = 1;
                }

                return referenceCount;
            }

            #endregion

            #endregion

            #region Workbook

            /// <summary>
            /// Returns true if the workbooks are identical, using the same file.
            /// </summary>
            public bool Equals(Workbook workbook)
            {
                if (workbook == null) return false;
                if (workbook == this) return true;
                return m_dataSource.FileInfo.Equals(workbook.m_dataSource.FileInfo);
            }

            /// <summary>
            /// Returns the workbook table with the given name, or null if none.
            /// </summary>
            public Table GetTable(String tableName)
            {
                lock (m_tables)
                {
                    Table table;
                    if (m_tables.TryGetValue(tableName, out table))
                    {
                        return table;
                    }
                }
                return new Table(this, tableName);
            }

            #endregion
        }

        #endregion

        #region Exceptions

        public class NotAnExcelFileException : System.Exception
        {
            public NotAnExcelFileException(string errorMessage) :
                base(errorMessage)
            {
            }
        }

        public class Excel2007OleDbDriverNotInstalledException : System.Exception
        {
            public Excel2007OleDbDriverNotInstalledException(string errorMessage) :
                base(errorMessage)
            {
            }
        }

        public class UnknownOleDbConnectException : System.Exception
        {
            public UnknownOleDbConnectException(string errorMessage) :
                base(errorMessage)
            {
            }
        }

        #endregion

        #region Fields

        /// <summary>
        /// The singleton instance of the workbook manager.
        /// </summary>
        public static readonly Excel Instance = new Excel();

        #endregion

        #endregion

        /// <summary>
        /// A table of workbooks to manage lifetime.
        /// </summary>
        private readonly Dictionary<string, Workbook> m_workbooks =
            new Dictionary<string, Workbook>();

        /// <summary>
        /// Constructs the Excel implementation.
        /// </summary>
        protected Excel()
        {
        }

        /// <summary>
        /// Returns the workbook for the file name.
        /// Can throw.
        /// </summary>
        public Workbook GetWorkbook(string fileName)
        {
            System.IO.FileInfo fileInfo = new System.IO.FileInfo(fileName);
            fileName = fileInfo.ToString(); // Normalize the name.

            lock (m_workbooks)
            {
                Workbook workbook;
                if (m_workbooks.TryGetValue(fileName, out workbook))
                {
                    return workbook;
                }
            }
            return new Workbook(fileInfo); // Can throw.
        }

        #region IWorkbook

        /// <summary>
        /// Disposes the Excel implementation.
        /// </summary>
        public override void Dispose()
        {
            lock (m_workbooks)
            {
                foreach (Workbook workbook in m_workbooks.Values)
                {
                    workbook.Dispose();
                }
                m_workbooks.Clear();
            }

            base.Dispose();
        }

        public override IWorkbookPointer CreateWorkbook(string filename)
        {
            return new IWorkbookPointer(GetWorkbook(filename));
        }

        #endregion
    }
}
