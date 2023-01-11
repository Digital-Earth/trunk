#ifndef CHECKER_COVERAGE_H
#define CHECKER_COVERAGE_H
/******************************************************************************
checker_coverage.h

begin      : 21/03/2007 10:59:52 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_pyxis_coverages.h"

// pyxis includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/string_utils.h"

/*!
A global coverage that always returns the same colour for different each different type of cell.
The default values of the cell are:
\verbatim

Origin child = red
Vertex child = blue
Pentagon = green
\endverbatim
*/
//! A global coverage that resembles a pyxis patterned checkerboard.
class MODULE_PYXIS_COVERAGES_DECL CheckerCoverage : public ProcessImpl<CheckerCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	CheckerCoverage();


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
	
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;
	
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;

public: // ConstCoverage

	//! Set the resoultion of the geometry.
	void setGeometryResolution(int nResolution)
	{
		if (m_nResolution != nResolution)
		{
			// TODO: Send a notification.
			m_nResolution = nResolution;
			m_spGeom = 0;
		}
	}

	//! Sets the PYXValue that is returned for every get value for a given field. 
	bool setOriginChildValue(const PYXValue& value);

	//! Return the colour associated with origin child cells.
	PYXValue getOriginChildValue() const {return m_originColour;}

	//! Sets the PYXValue that is returned for every get value for a given field. 
	bool setVertexChildValue(const PYXValue& value);

	//! Return the colour associated with vertex child cells.
	PYXValue getVertexChildValue() const {return m_vertexColour;}

	//! Sets the PYXValue that is returned for every get value for a given field. 
	bool setPentagonValue(const PYXValue& value);

	//! Return the colour associated with pentagon cells.
	PYXValue getPentagonValue() const {return m_pentagonColour;}

private:

	virtual void createGeometry() const
	{
		m_spGeom = PYXGlobalGeometry::create(m_nResolution);
	}

private:

	//! Build a new coverage definition from the current vector of fields.
	void buildCoverageDefinition();

	//! Verify that the passed pyxvalue is a colour.
	bool isColour(const PYXValue& value);

	//! The resolution for the data source
	int m_nResolution;

	//! The colour value for pentagons.
	PYXValue m_pentagonColour;

	//! The colour value for origin child hexagons.
	PYXValue m_originColour;

	//! The colour value for vertex child hexagons.
	PYXValue m_vertexColour;

	friend MODULE_PYXIS_COVERAGES_DECL bool operator ==(const CheckerCoverage& lhs, const CheckerCoverage& rhs);
};

//! The equality operator.
bool MODULE_PYXIS_COVERAGES_DECL operator ==(const CheckerCoverage& lhs, const CheckerCoverage& rhs);

//! The inequality operator.
bool MODULE_PYXIS_COVERAGES_DECL operator !=(const CheckerCoverage& lhs, const CheckerCoverage& rhs);

#endif