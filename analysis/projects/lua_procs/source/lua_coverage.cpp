/******************************************************************************
lua_coverage.cpp

begin		: 2007-03-23
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_LUA_PROCS_SOURCE
#include "lua_coverage.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// lua includes
extern "C" {
#include "lauxlib.h"
#include "lualib.h"
}

// standard includes
#include <cassert>

// {FFC4F5C3-979F-42d3-95D0-04078E65F5B4}
PYXCOM_DEFINE_CLSID(LuaCoverage, 
0xffc4f5c3, 0x979f, 0x42d3, 0x95, 0xd0, 0x4, 0x7, 0x8e, 0x65, 0xf5, 0xb4);
PYXCOM_CLASS_INTERFACES(LuaCoverage, IProcess::iid, ICoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(LuaCoverage, "Lua Coverage", "A coverage that is processed through an arbitrary amount of Lua code.", "Drop", // "Analysis",
					ICoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, -1, "Input Coverage(s)", "A coverage that is associated with the Lua process.")
	IPROCESS_SPEC_PARAMETER(IString::iid, 0, -1, "Lua Module(s)", "A string of Lua code used for processing.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<LuaCoverage> gTester;

// Context strings
std::string strContextRGB("rgb");
std::string strContextCLUT("clut");
std::string strContextElevation("elev");
std::string strContextGreyScale("greyscale");
std::string strContextClass("class");
std::string strContextNone("none");

const std::string& contextToStr(PYXFieldDefinition::eContextType nContextType)
{
	switch (nContextType)
	{
		case PYXFieldDefinition::knContextRGB:			return strContextRGB;
		case PYXFieldDefinition::knContextCLUT:			return strContextCLUT;
		case PYXFieldDefinition::knContextElevation:	return strContextElevation;
		case PYXFieldDefinition::knContextGreyScale:	return strContextGreyScale;
		case PYXFieldDefinition::knContextClass:		return strContextClass;
		default:										return strContextNone;
	}
}

PYXFieldDefinition::eContextType strToContext(const std::string& strContext)
{
	if (strContext == strContextRGB)
	{
		return PYXFieldDefinition::knContextRGB;
	}
	else if (strContext == strContextCLUT)
	{
		return PYXFieldDefinition::knContextCLUT;
	}
	else if (strContext == strContextElevation)
	{
		return PYXFieldDefinition::knContextElevation;
	}
	else if (strContext == strContextGreyScale)
	{
		return PYXFieldDefinition::knContextGreyScale;
	}
	else if (strContext == strContextClass)
	{
		return PYXFieldDefinition::knContextClass;
	}
	else
	{
		return PYXFieldDefinition::knContextNone;
	}
}

/*!
Expected arguments:
  1 input number (1 to N)
  2 string index
  3 field index (1 to N)
Returns: the input coverage values
*/
int getInputValue(lua_State* L)
{
	int nInput = luaL_checkint(L, 1);
	PYXIcosIndex index(luaL_checkstring(L, 2));
	int nField = luaL_optint(L, 3, 0) - 1;

	PYXValue v = ((ICoverage*)lua_touserdata(L, lua_upvalueindex(nInput)))
		->getCoverageValue(index, nField);

	// TODO it will be more efficient to look at the specs and definitions to
	// do a lot of this computation beforehand.

	if (v.isNull())
	{
		return 0;
	}

	int nCount = v.getArraySize();

	if (v.getArrayType() == PYXValue::knString)
	{
		if (v.isArrayNullable())
		{
			for (int n = 0; n != nCount; ++n)
			{
				if (v.isNull(n))
				{
					lua_pushnil(L);
				}
				else
				{
					lua_pushstring(L, v.getString(n).c_str());
				}
			}
		}
		else
		{
			for (int n = 0; n != nCount; ++n)
			{
				lua_pushstring(L, v.getString(n).c_str());
			}
		}
	}
	else
	{
		if (v.isArrayNullable())
		{
			for (int n = 0; n != nCount; ++n)
			{
				if (v.isNull(n))
				{
					lua_pushnil(L);
				}
				else
				{
					lua_pushnumber(L, v.getDouble(n));
				}
			}
		}
		else
		{
			for (int n = 0; n != nCount; ++n)
			{
				lua_pushnumber(L, v.getDouble(n));
			}
		}
	}

	return nCount;
}

}

LuaCoverage::LuaCoverage()
{
	m_L = luaL_newstate();
	if (!m_L)
	{
		PYXTHROW(PYXException, "luaL_newstate failed");
	}
	luaL_openlibs(m_L);
}

LuaCoverage::~LuaCoverage()
{
	lua_close(m_L);
}

void LuaCoverage::test()
{
	// TODO something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE LuaCoverage::getAttributes() const
{
	return m_mapAttr;
}

std::string STDMETHODCALLTYPE LuaCoverage::getAttributeSchema() const
{
	// TODO[kabiraman]: The Process Editor no longer supports optional attributes.
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?><xs:schema target"
		"Namespace=\"http://tempuri.org/XMLSchema.xsd\" elementFormDefault="
		"\"qualified\" xmlns=\"http://tempuri.org/XMLSchema.xsd\" xmlns:mstns"
		"=\"http://tempuri.org/XMLSchema.xsd\" xmlns:xs=\"http://www.w3.org/"
		"2001/XMLSchema\"><xs:element name=\"LuaCoverage\"><xs:complexType>"
		"<xs:sequence><xs:element name=\"optional_attributes\" type="
		"\"optional_attributes\" maxOccurs=\"unbounded\" /></xs:sequence>"
		"</xs:complexType></xs:element><xs:complexType name=\"optional_"
		"attributes\"><xs:sequence><xs:element name=\"attribute_name\" "
		"type=\"xs:string\" /><xs:element name=\"value\" type=\"xs:string\""
		" /></xs:sequence></xs:complexType></xs:schema>";
}

void STDMETHODCALLTYPE LuaCoverage::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
	m_mapAttr = mapAttr;
}

IProcess::eInitStatus LuaCoverage::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Lua Coverage: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// "Reboot" the Lua state.
	int nError;
	lua_close(m_L);
	m_L = luaL_newstate();
	if (!m_L)
	{
		PYXTHROW(PYXException, "luaL_newstate failed");
	}
	luaL_openlibs(m_L);

	// TODO could more of this be pushed as local (not global) variables?

	// Push number of inputs as a global number.
	if (!getParameter(0))
	{
	    PYXTHROW(PYXException, "Missing input.");
	}

	int nInputs = getParameter(0)->getValueCount();
	lua_pushnumber(m_L, nInputs);
	lua_setglobal(m_L, "nInputs");

	// Push the input coverage definitions as a global table.
	lua_newtable(m_L);
	for (int nInput = 0; nInput != nInputs; ++nInput)
	{
		lua_newtable(m_L); // cov defn
		PYXPointer<const PYXTableDefinition> spCovDefn =
			getInput(nInput)->getCoverageDefinition();
		int nFields = spCovDefn->getFieldCount();
		for (int nField = 0; nField != nFields; ++nField)
		{
			lua_newtable(m_L); // field defn
			const PYXFieldDefinition& fieldDefn =
				spCovDefn->getFieldDefinition(nField);
			lua_pushstring(m_L, fieldDefn.getName().c_str());
			lua_rawseti(m_L, -2, 1); // add name to field defn
			lua_pushstring(m_L, PYXValue::getString(fieldDefn.getType()));
			lua_rawseti(m_L, -2, 2); // add type to field defn
			lua_pushnumber(m_L, fieldDefn.getCount());
			lua_rawseti(m_L, -2, 3); // add count to field defn
			lua_pushstring(m_L, contextToStr(fieldDefn.getContext()).c_str());
			lua_rawseti(m_L, -2, 4); // add context to field defn
			lua_rawseti(m_L, -2, nField + 1); // add field defn to cov defn
		}
		lua_rawseti(m_L, -2, nInput + 1); // add cov defn to table
	}
	lua_setglobal(m_L, "incovdefn");

	// Push the input value function as a closure with each input coverage
	// as an upvalue.
	for (int nInput = 0; nInput != nInputs; ++nInput)
	{
		lua_pushlightuserdata(m_L, (void*)getInput(nInput).get());
	}
	lua_pushcclosure(m_L, &getInputValue, nInputs);
	lua_setglobal(m_L, "getInputValue");

	// Push each attribute as a global string.
	for (std::map<std::string, std::string>::iterator it = m_mapAttr.begin();
		it != m_mapAttr.end(); ++it)
	{
		lua_pushstring(m_L, it->second.c_str());
		lua_setglobal(m_L, it->first.c_str());
	}

	// Load required modules.
	if (!getParameter(1))
	{
	    PYXTHROW(PYXException, "Missing input.");
	}
	int nModules = getParameter(1)->getValueCount();
	for (int nModule = 0; nModule != nModules; ++nModule)
	{
		const std::string& strData = getModule(nModule)->str();

		// Compile the data as a chunk.
		nError = luaL_loadbuffer(m_L, strData.c_str(), strData.size(),
			(StringUtils::toString("module") + StringUtils::toString(nModule)).c_str());
		if (nError)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("luaL_loadbuffer failed: \n" + std::string(lua_tostring(m_L, -1)));
			return knFailedToInit;
		}
		nError = lua_pcall(m_L, 0, 0, 0);
		if (nError)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError( "lua_pcall failed: \n" + std::string(lua_tostring(m_L, -1)));
			return knFailedToInit;
		}
	}

	// Compile the data as a chunk.
	std::string strData = getData();
	nError = luaL_loadbuffer(m_L, strData.c_str(), strData.size(), "data");
	if (nError)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError( "luaL_loadbuffer failed: \n" + std::string(lua_tostring(m_L, -1)));
		return knFailedToInit;
	}
	nError = lua_pcall(m_L, 0, 0, 0);
	if (nError)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError( "lua_pcall failed: \n" + std::string(lua_tostring(m_L, -1)));
		return knFailedToInit;
	}

#if 0
	// Coverage description.
	lua_getglobal(m_L, "covdesc"); // get cov desc
	setDescription(lua_isstring(m_L, -1) ? lua_tostring(m_L, -1) : "");
	lua_pop(m_L, -1); // pop cov desc
#endif

	// Coverage definition.
	m_spCovDefn = PYXTableDefinition::create();
	lua_getglobal(m_L, "covdefn"); // get cov defn table
	if (lua_istable(m_L, -1))
	{
		int nFields = static_cast<int>(lua_objlen(m_L, -1));
		for (int nField = 1; nField != nFields + 1; ++nField)
		{
			lua_rawgeti(m_L, -1, nField); // get field defn table
			if (lua_istable(m_L, -1))
			{
				lua_rawgeti(m_L, -1, 1); // get name
				lua_rawgeti(m_L, -2, 2); // get type
				lua_rawgeti(m_L, -3, 3); // get count
				lua_rawgeti(m_L, -4, 4); // get context

				m_spCovDefn->addFieldDefinition(
					lua_tostring(m_L, -4),
					strToContext(lua_tostring(m_L, -1)),
					PYXValue::getType(lua_tostring(m_L, -3)),
					static_cast<int>(lua_tointeger(m_L, -2)));

				lua_pop(m_L, 4); // pop name, type, count, context
			}
			lua_pop(m_L, 1); // pop field defn table
		}
	}
	else
	{
		// Default to RGB
		m_spCovDefn->addFieldDefinition(
			"rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
	}
	lua_pop(m_L, 1); // pop cov defn table

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue LuaCoverage::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	//TRACE_INFO(index.toString() << " field " << nFieldIndex);

	int nError;

	assert(0 <= nFieldIndex && nFieldIndex < getCoverageDefinition()->getFieldDefinition(nFieldIndex).getCount());
	const PYXFieldDefinition& fieldDefn = getCoverageDefinition()->getFieldDefinition(nFieldIndex);
	PYXValue::eType nValueType = fieldDefn.getType();
	int nValueCount = fieldDefn.getCount();

	lua_getglobal(m_L, "getCoverageValue");
	lua_pushstring(m_L, index.toString().c_str());
	lua_pushnumber(m_L, nFieldIndex + 1);

	nError = lua_pcall(m_L, 2, nValueCount, 0);
	if (nError)
	{
		PYXTHROW(PYXException, "lua_pcall failed: " << lua_tostring(m_L, -1));
	}

	PYXValue v;

#define ELSE_IF(TYPE, CPPTYPE, LUAFUNC) \
	else if (nValueType == TYPE) \
	{ \
		CPPTYPE* buf = new CPPTYPE[nValueCount]; \
		for (int n = 0; n != nValueCount; ++n) \
		{ \
			buf[n] = static_cast<CPPTYPE>(LUAFUNC(m_L, n - nValueCount)); \
			/*TRACE_INFO("got " << #CPPTYPE << '[' << n << ']' << " val " << buf[n]);*/ \
		} \
		v.swap(PYXValue::create(TYPE, buf, nValueCount, 0)); \
		delete [] buf; \
	}

	// TODO this could likely be written more efficiently

	// Handle reading null values	
	bool bNil = true;
	for (int n = 0; n != nValueCount; ++n)
	{
		if (!lua_isnil(m_L, n - nValueCount))
		{
			bNil = false;
			break;
		}
	}
	if (bNil)
	{
	}
#if 1
	else if (nValueType == PYXValue::knBool)
	{
		bool* buf = new bool[nValueCount];
		for (int n = 0; n != nValueCount; ++n)
		{
			buf[n] = lua_toboolean(m_L, n - nValueCount) != 0;
			/*TRACE_INFO("got " << #CPPTYPE << '[' << n << ']' << " val " << buf[n]);*/
		}
		v.swap(PYXValue::create(PYXValue::knBool, buf, nValueCount, 0));
		delete[] buf;
	}
#else
	ELSE_IF(PYXValue::knBool, bool, lua_toboolean)
#endif
	ELSE_IF(PYXValue::knChar, char, lua_tonumber)
	ELSE_IF(PYXValue::knInt8, int8_t, lua_tonumber)
	ELSE_IF(PYXValue::knUInt8, uint8_t, lua_tonumber)
	ELSE_IF(PYXValue::knInt16, int16_t, lua_tonumber)
	ELSE_IF(PYXValue::knUInt16, uint16_t, lua_tonumber)
	ELSE_IF(PYXValue::knInt32, int32_t, lua_tonumber)
	ELSE_IF(PYXValue::knUInt32, uint32_t, lua_tonumber)
	ELSE_IF(PYXValue::knFloat, float, lua_tonumber)
	ELSE_IF(PYXValue::knDouble, double, lua_tonumber)
	ELSE_IF(PYXValue::knString, std::string, lua_tostring)

#undef ELSE_IF

	lua_pop(m_L, nValueCount);
	return v;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

boost::intrusive_ptr<ICoverage> LuaCoverage::getInput(int n) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	boost::intrusive_ptr<ICoverage> spCov;
	assert(getParameter(0) &&
		getParameter(0)->getValue(n) &&
		getParameter(0)->getValue(n)->getOutput());
	getParameter(0)->getValue(n)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCov);
	return spCov;
}

boost::intrusive_ptr<IString> LuaCoverage::getModule(int n) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	boost::intrusive_ptr<IString> spStr;
	assert(getParameter(1) &&
		getParameter(1)->getValue(n) &&
		getParameter(1)->getValue(n)->getOutput());
	getParameter(1)->getValue(n)->getOutput()->QueryInterface(
		IString::iid, (void**) &spStr);
	return spStr;
}

void LuaCoverage::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (0 < getParameter(0)->getValueCount() && getInput(0)->getGeometry())
	{
		m_spGeom = getInput(0)->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}
}
