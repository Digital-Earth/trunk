#ifndef FEATURE_FIELD_RASTERIZER_2_H
#define FEATURE_FIELD_RASTERIZER_2_H
/******************************************************************************
feature_field_rasterizer2.h

begin		: 2017-03-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/coord_3d.h"

// standard includes
#include <cassert>
#include <vector>

/*!
Process that transform values of a coverage using a transform mapping table.
*/
//! Process that transform values of a coverage using a transform mapping table.
class MODULE_ANALYSIS_PROCS_DECL FeatureFieldRasterizer2 : public ProcessImpl<FeatureFieldRasterizer2>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureFieldRasterizer2();

protected:
	//! Destructor
	virtual ~FeatureFieldRasterizer2();

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

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;	

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

public:

	static void test();

private:

	virtual void createGeometry() const;

	void FeatureFieldRasterizer2::generateAverageTile(const PYXTile & tile,
													  PYXValueTile & valueTile) const;

	void FeatureFieldRasterizer2::generateMinTile(const PYXTile & tile,
												  PYXValueTile & valueTile) const;

	void FeatureFieldRasterizer2::generateMaxTile(const PYXTile & tile,
												  PYXValueTile & valueTile) const;

private:

	//! The input coverage.
	boost::intrusive_ptr<IFeatureCollection> m_inputFC;

	//! name of the field to raster
	std::string m_fieldName;

	int m_fieldIndex;

	//! target type of the transform ("byte","int","float","double")
	std::string m_outputType;

	//! aggregation type of the transform ("min","max","average")
	std::string m_aggregate;

	//! transform value
	std::map<PYXValue,PYXValue> m_transform;

	bool m_exactMatch;
};

#endif // guard
