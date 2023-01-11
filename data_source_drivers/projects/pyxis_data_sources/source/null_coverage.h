#ifndef NULL_COVERAGE_H
#define NULL_COVERAGE_H
/******************************************************************************
null_coverage.h

begin		: 2006-05-25
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_pyxis_coverages.h"

// pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <fstream>

/*!
This coverage is designed to provide a null value for any request. The coverage
has a global geometry and its only attribute is resolution. The first field is used
to hold a null RGB value and the second a null elevation value.
*/
//! A coverage that always returns a NULL value.
class MODULE_PYXIS_COVERAGES_DECL NullCoverage : public ProcessImpl<NullCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	NullCoverage() :
		m_nResolution(10),
		m_dataType(knRGB)
	{
		m_strID = "Null Coverage " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	}

	// implementation
	enum dataType
	{
		knRGB = 0, 
		knElevation
	};

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( NullCoverage, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual IProcess::eInitStatus initImpl();

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

	virtual void STDMETHODCALLTYPE setAttributes(
		const std::map<std::string, std::string>& mapAttr);

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;
private:

	virtual void createGeometry() const
	{
		m_spGeom = PYXGlobalGeometry::create(m_nResolution);
	}

private:

	//! The type of data to pass through as null
	dataType m_dataType;

	//! The resolution for the data source
	int m_nResolution;

	friend MODULE_PYXIS_COVERAGES_DECL bool operator ==(const NullCoverage& lhs, const NullCoverage& rhs);

};

//! The equality operator.
inline MODULE_PYXIS_COVERAGES_DECL bool operator ==(const NullCoverage& lhs, const NullCoverage& rhs)
{
	return (lhs.getProcVersion() == rhs.getProcVersion() && lhs.m_nResolution == rhs.m_nResolution); 
}

//! The inequality operator.
inline bool MODULE_PYXIS_COVERAGES_DECL operator !=(const NullCoverage& lhs, const NullCoverage& rhs)
{
	return (!(lhs == rhs));
}

#endif
