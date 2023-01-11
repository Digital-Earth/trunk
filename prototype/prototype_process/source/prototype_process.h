#ifndef PROTOTYPE_PROCESS_H
#define PROTOTYPE_PROCESS_H
/******************************************************************************
prototype_process.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "prototype_process_config.h"

#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"

/*!
*/
//! A prototype process.
class PROTOTYPE_PROCESS_DECL ProcA : public ProcessImpl
{
	PYXCOM_DECLARE_CLASS();

public:

	ProcA() :
		ProcessImpl(boost::intrusive_ptr<IProcessSpec>(), "ProcA name", "ProcA desc")
	{}

	virtual boost::intrusive_ptr<const IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return boost::intrusive_ptr<IUnknown>();
	}

	virtual boost::intrusive_ptr<IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return boost::intrusive_ptr<IUnknown>();
	}

public: // IProcess

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		return m_mapAttr;
	}

	virtual std::string STDMETHODCALLTYPE getData() const
	{
		return m_strData;
	}

	virtual void STDMETHODCALLTYPE setData(const std::string& strData)
	{
		TRACE_INFO("ProcA got data: " << strData);
		m_strData = strData;
	}

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr)
	{
		m_mapAttr = mapAttr;
	}

	IPROCESS_GETSPEC_IMPL();

private:

	std::map<std::string, std::string> m_mapAttr;
	std::string m_strData;
};

/*!
*/
//! A prototype process.
class PROTOTYPE_PROCESS_DECL ProcB : public ProcessImpl
{
	PYXCOM_DECLARE_CLASS();

public:

	ProcB() :
		ProcessImpl(boost::intrusive_ptr<IProcessSpec>(), "ProcB name", "ProcB desc")
	{}

	virtual boost::intrusive_ptr<const IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return boost::intrusive_ptr<const IUnknown>();
	}

	virtual boost::intrusive_ptr<IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return boost::intrusive_ptr<IUnknown>();
	}

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

};

/*!
*/
//! A prototype process.
class PROTOTYPE_PROCESS_DECL ProcC : public ProcessImpl
{
	PYXCOM_DECLARE_CLASS();

public:

	ProcC() :
		ProcessImpl(boost::intrusive_ptr<IProcessSpec>(), "ProcC name", "ProcC desc")
	{}

	virtual boost::intrusive_ptr<const IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return boost::intrusive_ptr<IUnknown>();
	}

	virtual boost::intrusive_ptr<IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return boost::intrusive_ptr<IUnknown>();
	}

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

};

#if 1
/*!
*/
//! A simple coverage spanning the world.
class PROTOTYPE_PROCESS_DECL SoccerBallCoverage : public ICoverage
{
	PYXCOM_DECLARE_CLASS();

public:

	SoccerBallCoverage()
	{
	}

	SoccerBallCoverage(int nResolution)
	{
		m_spGeom = PYXGlobalGeometry::create(nResolution);
	}

public: // IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(const PYXIcosIndex& index) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

};

/*!
*/
//! A prototype process.
class PROTOTYPE_PROCESS_DECL SoccerBallProc : public ProcessImpl
{
	PYXCOM_DECLARE_CLASS();

public:

	SoccerBallProc() :
		ProcessImpl(boost::intrusive_ptr<IProcessSpec>(), "SoccerBallProc instance", "SoccerBallProc instance")
	{
		PYXCOMCreateInstance(SoccerBallCoverage::clsid, 0, ICoverage::iid, (void**) &m_spCov);
	}

	virtual boost::intrusive_ptr<const IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return boost::intrusive_ptr<IUnknown>();
	}

	virtual boost::intrusive_ptr<IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return boost::intrusive_ptr<IUnknown>();
	}

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr)
	{
		std::map<std::string, std::string>::const_iterator it;
		it = mapAttr.find("res");
		if (it != mapAttr.end())
		{
			int nRes = atoi(it->second.c_str());
			//m_spCov->getGeometry()->setCellResolution(nRes);
			// TODO set resolution of output or something
		}
	}

private:

	boost::intrusive_ptr<SoccerBallCoverage> m_spCov;
};

#if 0
/*!
*/
//! A simple coverage spanning the world.
class PROTOTYPE_PROCESS_DECL SoccerBall : public ICoverage, public ProcessImpl
{
	PYXCOM_DECLARE_CLASS();

public:

	SoccerBall()
	{
		m_spGeom = PYXGlobalGeometry::create(5);
	}

	SoccerBall(int nResolution)
	{
		m_spGeom = PYXGlobalGeometry::create(nResolution);
	}

public: // IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
		IUNKNOWN_QI_CASE(IProcess)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(const PYXIcosIndex& index) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

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

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr)
	{
		std::map<std::string, std::string>::const_iterator it;
		it = mapAttr.find("res");
		if (it != mapAttr.end())
		{
			int nRes = atoi(it->second.c_str());
			getGeometry()->setCellResolution(nRes);
		}
	}
};
#endif
#endif

#endif // guard
