#ifndef PYXIS__UTILITY__VALUE_TABLE_H
#define PYXIS__UTILITY__VALUE_TABLE_H
/******************************************************************************
value_table.h

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"

// standard includes
#include <iosfwd>
#include <vector>

// forward declarations
class PYXValueColumn;

/*!
A PYXValueTable is a fixed-size "table" of data values accessed as PXYValue objects.
The rows are called "records", and the columns are called "fields".  Rows (records)
and columns (fields) are indexed by integers.  Every ordered pair (row,col) where row
is a valid row-index and col is a valid column-index identifies a "slot" or variable
which can logically contain one PYXValue object.  The underlying data representation
need not use PYXValue objects, and it need not have a tabular structure; these are
only aspects of the interface.

The names "record" and "field" have been chosen to imply exactly what is usually
meant by these terms, specifically:
  - All records have identical structure: a sequence of fields, same number,
    same order.
  - Each field has an associated data type.  Because the record structure is
    the same for all records, the fields naturally line up into columns; hence
    all data slots in a column have the same data type.

In a database or other real-world data table, columns/fields normally have an
associated set of attributes (name, type, valid range, etc.) which relate to
their semantic interpretation, and collectively constitute the "identity" of
each field.  For PYXValueTable, the column index serves as a minimal identity;
more advanced identity attributes such as field names can be added in derived
classes if desired.  The notion of data type, however, is enforced at this level.

Given a valid (row,col) pair addressing a slot in the PYXValueTable, the type of
the PYXValue occupying that slot must either be the PYXValue::eType associated
with the col'th column or the null value (PYValue::eType::knNull).  The getValue()
method can optionally return extra information, in cases where the retrieved data
value is null, indicating whether the data slot is simply empty (no prior call to
setValue() has yet been done) or has been set explicitly to null.
*/
//! PYXValueTable represents a 2D table of slots for PYXValue objects.
class PYXLIB_DECL PYXValueTable
{
	friend PYXLIB_DECL std::ostream& operator <<(	std::ostream& out,
													PYXValueTable& pvt	);
	friend PYXLIB_DECL std::istream& operator >>(	std::istream& in,
													PYXValueTable& pvt	);

public:

	//! Unit test method
	static void test();

	//! Default constructor creates an empty table
	PYXValueTable() {}

	//! Simple constructor: use when all columns have non-array type
	PYXValueTable(	const int nRows,
					const std::vector<PYXValue::eType>& vecTypes	);

	//! Obtain an estimate of memory required for simple constructor
	static const int estimateHeapBytes(	const int nRows,
										const std::vector<PYXValue::eType>& vecTypes	);

	//! General constructor: use when one or more columns have array type
	PYXValueTable(	const int nRows,
					const std::vector<PYXValue::eType>& vecTypes,
					const std::vector<int>& vecCounts	);

	//! Obtain an estimate of memory required for general constructor
	static const int estimateHeapBytes(	const int nRows,
										const std::vector<PYXValue::eType>& vecTypes,
										const std::vector<int>& vecCounts	);

	//! Copy constructor
	PYXValueTable(const PYXValueTable&);

	//! Get a copy of a single channel of data.
	PYXValueTable(const PYXValueTable&, int nChannel);

	//! Destructor
	virtual ~PYXValueTable();

	//! Serialize to stream
	void serialize(std::ostream& out);

	//! De-serialize from stream
	void deserialize(std::istream& in);

	//! Get total memory allocated
	int getHeapBytes() const;

	//! Get number of table rows
	inline int getNumberOfRows() const;

	//! Get number of table columns
	inline int getNumberOfColumns() const;

	//! Get the base type for given column
	PYXValue::eType getColumnType(const int nColumn) const;

	//! Get array count for given column
	inline int getCount(const int nColumn) const;

	//! Get a value, and optionally an indication of whether or not it was ever set
	PYXValue getValue(	const int nRow,
						const int nColumn,
						bool* pbInitialized = 0	) const;

	//! Get a value, returns false for null value
	bool getValue(	const int nRow,
					const int nColumn,
					PYXValue* pValue) const;

	//! Set a value with bounds and type checking
	void setValue(const int nRow, const int nColumn, const PYXValue& value);

protected:

	//! Disable copy assignment
	void operator =(const PYXValueTable&);

	//! Base data types for each column
	std::vector<PYXValue::eType> m_vecColumnType;

private:

	//! Implementation: vector of PYXValueColumn, 1 per column
	std::vector<PYXValueColumn*> m_vecColumnData;
};

//! Stream output operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, PYXValueTable& pvt);

//! Stream input operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXValueTable& pvt);

#endif // guard
