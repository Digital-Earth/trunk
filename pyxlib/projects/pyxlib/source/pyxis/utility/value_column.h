#ifndef PYXIS__UTILITY__VALUE_COLUMN_H
#define PYXIS__UTILITY__VALUE_COLUMN_H
/******************************************************************************
value_column.h

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/memory_manager.h"

// standard includes
#include <iosfwd>

/*!
PYXValueColumn is a private helper class for PYXValueTable.  A PYXValueColumn represents
one column of a PYXValueTable -- a fixed-size array of PYXValue objects.  Physically, a
packed binary representation is used, and values are converted to and from PYXValue as
required.  PYXValueColumn also supports both binary and text serialization.

PYXValue itself can already represent 1-D arrays of the following simple types (constants
defined in the enum PYXValue::eType):

\code
		knBool					// Type represents bool
		knChar					// Type represents char
		knInt8					// Type represents int8_t
		knUInt8					// Type represents uint8_t
		knInt16					// Type represents int16_t
		knUInt16				// Type represents uint16_t
		knInt32					// Type represents int32_t
		knUInt32				// Type represents uint32_t
		knFloat					// Type represents float
		knDouble				// Type represents double
		knString				// Type represents std::string
\endcode

A column of PYXValue objects is thus really a 2-D array of the base type; we speak of
the column's "height" (the number of PYXValue elements it can contain) and also of its
"width" (the number of base-type values in each element).  The width is 1 for the simple
types listed above, and may be more than 1 for arrays of those types, e.g., a PYXValueColumn
representing 24-bit RGB colours might have base type knUint8 and width 3.

PYXValue also supports the notion of a "null" value (PYXValue::eType::knNull), to allow
encoding of dense data sets containing "holes" or regions where no data is available.
The present implementation of PYXValueColumn uses a subsidiary bit array to support this.
To support use of PYXValueColumn as a cache, PYXValueColumn also distinguishes between
"null" and "uninitialized", allowing client code to provide a pointer to a bool variable
in calls to getValue(), which gets set whenever getValue() itself returns knNull.

You create a PYXValueColumn by providing a vector of base types from the above list.
The other PYXValue::eType codes can't be used for various reasons.  (A column of knNull
wouldn't make sense, a vector of knArray isn't necessary because it's implied by
width > 1, and the reserved type codes are still not yet defined in PYXValue itself).

The special type code knArray can't be used in defining a PYXValueColumn, but it is
used in getValue() and setValue().  For any PYXValueColumn with width > 1, getValue()
returns, and setValue() requires, a PYXValue object with type knArray, array-count equal
to the column width, and array-type equal to the column type.
*/
//! PYXValueColumn: array of PYXValue objects with packing and serialization
class PYXLIB_DECL PYXValueColumn : protected ObjectMemoryUsageCounter<PYXValueColumn>
{
public:

	//! Unit test method
	static void test();

	//! Destructor
	virtual ~PYXValueColumn();

	//! Serialize to stream
	void serialize(std::ostream& out);

	//! De-serialize from stream
	void deserialize(std::istream& in);

	//! Get number of elements
	int getHeight() const {return m_nColumnHeight;}

	//! Get element count
	int getWidth() const {return m_nColumnWidth;}

	//! Get total memory allocated
	int getHeapBytes() const {return m_nHeapBytes;}

	//! Get the type
	const PYXValue::eType getType() const {return m_type;}

	//! Get a value, and optionally an indication of whether or not it was ever set
	PYXValue getValue(const int nIndex, bool* pbInitialized=0) const;

	//! Get a value, and an indication of whether or not it was ever set
	bool getValue(const int nIndex, PYXValue* pValue) const;

	//! Set a value
	void setValue(const int nIndex, const PYXValue& value);

protected:

	//! Default constructor creates a null column
	PYXValueColumn();

	//! Standard constructor allocates space for data.
	PYXValueColumn(	const PYXValue::eType type,
					const int nColumnHeight,
					const int nColumnWidth = 1,
					const bool bNullable = true	);

	//! compute basic data block size (not including space for not-null bit vector)
	static void computeBasicDataBlockSize(	const PYXValue::eType type,
											const int nColumnHeight,
											const int nColumnWidth,
											int* pnSlotBytes,
											int* pnBlockBytes	);

	//! allocate data block for given type and dimensions
	void allocateDataBlock(	const PYXValue::eType type,
							const int nColumnHeight,
							const int nColumnWidth,
							const bool bNullable	);

	//! Get estimate of heap space required before standard constructor
	static const int estimateHeapBytes(	const PYXValue::eType type,
										const int nColumnHeight,
										const int nColumnWidth = 1,
										const bool bNullable = true	);

	//! Copy constructor
	PYXValueColumn(const PYXValueColumn&);

private:

	//! Element type
	PYXValue::eType m_type;

	//! Data slot size in bytes (0 if type is boolean)
	int m_nSlotBytes;

	//! Number of elements
	int m_nColumnHeight;

	//! Count of elements inside each array-type element (1 for simple types)
	int m_nColumnWidth;

	//! Size of allocated memory block at m_pValues
	int m_nBlockBytes;

	//! Approximate count of heap space owned by this PYXValueColumn
	int m_nHeapBytes;

	//! Storage for our values (for strings, the values are pointers)
	char* m_pValues;

	//! Bit-vector has bits set where data are non-null (pointer is 0 if not nullable)
	char* m_pNotNull;

	friend class PYXValueTable;
	friend PYXLIB_DECL std::ostream& operator <<(	std::ostream& out,
													PYXValueColumn& pvv	);
	friend PYXLIB_DECL std::istream& operator >>(	std::istream& in,
													PYXValueColumn& pvv	);
};

//! Stream output operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, PYXValueColumn& pvv);

//! Stream input operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXValueColumn& pvv);

#endif // guard
