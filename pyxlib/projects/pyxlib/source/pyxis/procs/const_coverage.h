#ifndef CONST_COVERAGE_H
#define CONST_COVERAGE_H
/******************************************************************************
const_coverage.h

begin		: 2006-04-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes

// pyxis includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/string_utils.h"

#include <boost/thread/thread.hpp>

/*!
A global data source that is capable of having multiple fields of different types of
data. The Field count, type and value can be set to any desired values. The default
setting is a resolution 10 data coverage with a single field that always returns an
array of 3 values (42, 42, 42) that can be viewed as RGB.
*/
//! A global coverage with a constant set of values across the entire globe.
class PYXLIB_DECL ConstCoverage : public ProcessImpl<ConstCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	ConstCoverage();

	//! Destructor
	~ConstCoverage();

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

    //! Enables managed test code to cast this to PYXCOM_IUnknown.
	boost::intrusive_ptr<PYXCOM_IUnknown> asIUnknown()
	{
		return (IProcess *) this;
	}

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( ConstCoverage, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}
	
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;

	// TODO:  implement a local copy of getCoverageTile that uses the techniques in
	// the getFieldTile for this class.

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

public: // ConstCoverage

	//! Set the resoultion of the geometry.
	void setGeometryResolution(int nResolution)
	{
		if (m_nResolution != nResolution)
		{
			// TODO: Send a notification.
			m_nResolution = nResolution;
			m_spGeom.reset();
		}
	}

	//! Sets the PYXValue that is returned for every get value for a given field. 
	void setReturnValue(	const PYXValue& value, 
							PYXFieldDefinition::eContextType nContext, 
							int nField = 0);

	//! Set the number of constant return values.
	void setFieldCount(int nFieldCount);

private:

	virtual void createGeometry() const
	{
		m_spGeom = PYXGlobalGeometry::create(m_nResolution);
	}

private:
	//! The thread responsible for changing the colours.
	static void colourChangeThread(ConstCoverage* cov);

	//! Used to ask the colour changing thread to stop.
	bool m_bNeedToStop;

	//! Used to signal that the colour changing thread is stopped.
	bool m_bIsStopped;

	//! The time between colour changes (only active if m_bIsRandom is true).
	int m_nSecondsBetweenChange;

	//! True if we are going to create a background thread that periodically chages the colour.
	bool m_bIsRandom;

	//! To manage the background thread that changes colours.
	boost::thread_group m_threads;

	//! Build a new coverage definition from the current vector of fields.
	void buildCoverageDefinition();

	//! The resolution for the data source
	int m_nResolution;

	//! The list of constant field values.
	std::vector<PYXValue> m_vecFields;

	//! The context for the field values.
	std::vector<PYXFieldDefinition::eContextType> m_vecContext;

	friend PYXLIB_DECL bool operator ==(const ConstCoverage& lhs, const ConstCoverage& rhs);

};

//! The equality operator.
bool PYXLIB_DECL operator ==(const ConstCoverage& lhs, const ConstCoverage& rhs);

//! The inequality operator.
bool PYXLIB_DECL operator !=(const ConstCoverage& lhs, const ConstCoverage& rhs);

#endif