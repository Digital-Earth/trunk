#ifndef BLUE_PYXIS_H
#define BLUE_PYXIS_H
/******************************************************************************
blue_pyxis.h

begin		: 2007-03-04
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_pyxis_coverages.h"

// pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/process.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

// standard includes
#include <deque>
#include <fstream>

/*!
Uses blue marble next generation 2km.
*/
//! Blue Marble PYXIS coverage process.
class MODULE_PYXIS_COVERAGES_DECL BluePyxis : public ProcessImpl<BluePyxis>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	BluePyxis();

public: // IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

private:

	virtual void createGeometry() const;

	bool configRes(int nRes) const;

	bool gen();

private:

	std::map<std::string, std::string> m_mapAttr;
	boost::filesystem::path m_path;
	bool m_bElev;

	enum { m_knNumValueTileCache = 64 };
	mutable std::deque<PYXPointer<PYXValueTile> > m_spValueTileCache;

	mutable boost::mutex m_mutex;
	mutable int m_nMinRes;
	mutable int m_nMaxRes;
	mutable int m_nRes;
	mutable int m_nSize;
	mutable std::ifstream m_in;
};

#endif // guard
