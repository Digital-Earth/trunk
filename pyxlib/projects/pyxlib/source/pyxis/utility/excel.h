#pragma once

/******************************************************************************
excel.h

begin      : 2009-11-27
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"

// Standard includes
#include <vector>
#include <string>

namespace Excel
{
	//! A view of a workbook table, having a heading row and one or more data rows.
	struct PYXLIB_DECL IWorkbookView : PYXObject
	{
		typedef PYXPointer<IWorkbookView const> Pointer;

		virtual ~IWorkbookView() = 0;

		//! Returns true if the view is empty.
		virtual bool IsEmpty() const = 0;

		//! Returns the total number of columns.
		virtual std::size_t GetColumnCount() const = 0;

		//! Gets the column name at the 0-based column offset.
		virtual void GetColumnName(std::size_t columnOffset, PYXValue & value) const = 0;

		//! Gets the values from the heading row, or returns false if unsuccessful.
		virtual bool GetHeadingRow(std::vector<PYXValue> & columns) const = 0;

		//! Gets a value at the 0-based column offset in the heading row, or returns false if unsuccessful.
		virtual bool GetHeadingCell(std::size_t columnOffset, PYXValue & value) const = 0;

		//! Returns the total number of data rows.
		virtual std::size_t GetDataRowCount() const = 0;

		//! Returns true if the data row at the 0-based offset is empty.
		virtual bool IsDataRowEmpty(std::size_t dataRowOffset) const = 0;

		//! Gets the values from the data row at the 0-based offset, or returns false if unsuccessful.
		virtual bool GetDataRow(std::size_t dataRowOffset, std::vector<PYXValue> & columns) const = 0;

		//! Gets the value at the 0-based column offset in the data row at the 0-based offset, or returns false.
		virtual bool GetDataCell(std::size_t dataRowOffset, std::size_t columnOffset, PYXValue & value) const = 0;

	public: // PYXObject

		virtual long release() const
		{
			return PYXObject::release();
		}

		virtual long addRef() const
		{
			return PYXObject::addRef();
		}

	protected:

		IWorkbookView()
		{
		}

	private:

		IWorkbookView(IWorkbookView const &);
		IWorkbookView & operator=(IWorkbookView const &);
	};

	//! A table in the workbook.
	/*!
	A table is a grid of cells.
	The first non-null row is numbered 0, and numbering continues to the last non-null row.
	Within this, the first non-null column is numbered 0, and numbering continues to the last non-null column.
	*/
	struct PYXLIB_DECL IWorkbookTable : PYXObject
	{
		typedef PYXPointer<IWorkbookTable const> Pointer;

		virtual ~IWorkbookTable() = 0;

		//! Returns true if the table is empty.
		virtual bool IsEmpty() const = 0;

		//! Creates the specified view of the table, where each offset is 0-based, or null if unsuccessful.
		virtual IWorkbookView::Pointer CreateView(
			std::size_t firstColumnOffset, std::size_t lastColumnOffset,
            std::size_t headingRowOffset,
			std::size_t firstDataRowOffset, std::size_t lastDataRowOffset) const = 0;

		//! Creates the default of view of the table or null if unsuccessful.
		virtual IWorkbookView::Pointer CreateDefaultView() const = 0;

		//! Returns the total number of columns.
		virtual std::size_t GetColumnCount() const = 0;

		//! Returns the total number of rows.
		virtual std::size_t GetRowCount() const = 0;

		//! Gets the value at the 0-based column offset in the row at the 0-based offset, or returns false.
		virtual bool GetCell(std::size_t rowOffset, std::size_t columnOffset, PYXValue & value) const = 0;

		//! Returns true if the row at the 0-based offset is empty.
		virtual bool IsRowEmpty(std::size_t rowOffset) const = 0;

	public: // PYXObject

		virtual long release() const
		{
			return PYXObject::release();
		}

		virtual long addRef() const
		{
			return PYXObject::addRef();
		}

	protected:

		IWorkbookTable()
		{
		}

	private:

		IWorkbookTable(IWorkbookTable const &);
		IWorkbookTable & operator=(IWorkbookTable const &);
	};

	//! An Excel workbook.
	struct PYXLIB_DECL IWorkbook : PYXObject
	{
		typedef PYXPointer<IWorkbook const> Pointer;

		virtual ~IWorkbook() = 0;

		//! Returns the name of the workbook, which corresponds to the leaf name in the file path.
		virtual std::string GetName() const = 0;

		//! Gets a list of table names in the workbook.
		virtual void GetTableNames(std::vector<std::string> & tableNames) const = 0;

		//! Creates the table with the specified name, or null if unsuccessful.
		virtual IWorkbookTable::Pointer CreateTable(
			std::string const & tableName) const = 0;

	public: // PYXObject

		virtual long addRef() const
		{
			return PYXObject::addRef();
		}

		virtual long release() const
		{
			return PYXObject::release();
		}

	protected:

		IWorkbook()
		{
		}

	private:

		IWorkbook(IWorkbook const &);
		IWorkbook & operator=(IWorkbook const &);
	};

	//! A class managing loading of Excel workbooks.
	struct PYXLIB_DECL IExcel
	{
		//! Sets and gets the workbook loader implementation.
		static IExcel const * Implementation(IExcel const * pImplementation = 0);

		virtual ~IExcel() = 0;

		//! Creates a workbook, loaded from the specified file, or returns null if unsuccessful.
		virtual IWorkbook::Pointer CreateWorkbook(std::string const & filename) const = 0;

	protected:

		IExcel()
		{
		}

	private:

		IExcel(IExcel const &);
		IExcel & operator=(IExcel const &);
	};
}
