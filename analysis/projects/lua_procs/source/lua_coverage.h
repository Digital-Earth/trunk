#ifndef LUA_COVERAGE_H
#define LUA_COVERAGE_H
/******************************************************************************
lua_coverage.h

begin		: 2007-03-23
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_lua_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/string.h"

// lua includes
extern "C" {
#include "lua.h"
}

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <fstream>

/*!
*/
//! Lua coverage process.
class MODULE_LUA_PROCS_DECL LuaCoverage : public ProcessImpl<LuaCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	LuaCoverage();

	//! Destructor
	~LuaCoverage();

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

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

public:

	static void test();

private:

	virtual boost::intrusive_ptr<ICoverage> getInput(int n) const;

	virtual boost::intrusive_ptr<IString> getModule(int n) const;

	virtual void createGeometry() const;

private:

	//! The lua state.
	lua_State* m_L;

	//! The attributes.
	std::map<std::string, std::string> m_mapAttr;
};

#endif // guard
