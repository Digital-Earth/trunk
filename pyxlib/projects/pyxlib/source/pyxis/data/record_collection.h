#ifndef PYXIS__DATA__RECORD_COLLECTION_H
#define PYXIS__DATA__RECORD_COLLECTION_H
/******************************************************************************
record_collection.h

begin		: 2013-5-30
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/record.h"
#include "pyxis/utility/object.h"

// standard includes

/*!
*/
//! A feature iterator.
class PYXLIB_DECL RecordIterator : public PYXObject
{
public:

	virtual bool end() const = 0;

	virtual void next() = 0;

	virtual boost::intrusive_ptr<IRecord> getRecord() const = 0;
};

/*!
Delegates to STL-style iterators.
*/
//! Templated helper class for feature iterators.
template <typename IT>
class DefaultRecordIterator : public RecordIterator
{
public:

	DefaultRecordIterator(IT it, IT itEnd) :
		m_it(it),
		m_itEnd(itEnd)
	{
	}

	template <typename IT>
	static PYXPointer<DefaultRecordIterator<IT> > create(IT it, IT itEnd)
	{
		return PYXNEW(DefaultRecordIterator<IT>, it, itEnd);
	}

public:

	virtual bool end() const
	{
		return m_it == m_itEnd;
	}

	virtual void next()
	{
		++m_it;
	}

	virtual boost::intrusive_ptr<IRecord> getFeature() const
	{
		return *m_it;
	}

private:

	IT m_it;
	IT m_itEnd;
};

//! Templated helper function.
template <typename IT>
inline
PYXPointer<DefaultRecordIterator<IT> > createDefaultRecordIterator(IT it, IT itEnd)
{
	return DefaultRecordIterator<IT>::create(it, itEnd);
}

/*!
Delegates to STL-style iterators.
*/
//! Templated helper class for an empty feature iterator.
template <typename IT>
class EmptyRecordIterator : public RecordIterator
{
public:

	EmptyRecordIterator(IT it, IT itEnd) :
		m_it(it),
		m_itEnd(itEnd)
	{
	}

	template <typename IT>
	static PYXPointer<EmptyRecordIterator<IT> > create(IT it, IT itEnd)
	{
		return PYXNEW(EmptyRecordIterator<IT>, it, itEnd);
	}

public:

	virtual bool end() const
	{
		return true;
	}

	virtual void next()
	{		
	}

	virtual boost::intrusive_ptr<IRecord> getRecord() const
	{
		return boost::intrusive_ptr<IRecord>();
	}

private:

	IT m_it;
	IT m_itEnd;
};

//! Templated helper function.
template <typename IT>
inline
PYXPointer<EmptyRecordIterator<IT> > createEmptyRecordIterator(IT it, IT itEnd)
{
	return EmptyRecordIterator<IT>::create(it, itEnd);
}

/*!
Similar to feature collection, this interface provides a set of record that represent a table
The difference however is that there is no geometry associated with the records.
*/
//! A record collection.
struct PYXLIB_DECL IRecordCollection : public IRecord
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXPointer<RecordIterator> STDMETHODCALLTYPE getIterator() const = 0;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getRecordDefinition() const = 0;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE getRecord(const std::string& strRecordID) const = 0;

};


#endif // guard
