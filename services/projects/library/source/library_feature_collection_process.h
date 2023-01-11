#ifndef LIBRARY__LIBRARY_FEATURE_COLLECTION_PROCESS_H
#define LIBRARY__LIBRARY_FEATURE_COLLECTION_PROCESS_H
/******************************************************************************
library_feature_collection_process.h

begin      : 11/12/2007 9:53:55 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes 
#include "library_initializer.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/pipe/process.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <memory>

// local forward declarations
class PYXGeometry;

/*!
*/
//! 
class /* LIBRARY_DECL */ LIBRARY_DECL LibraryFeatureCollectionProcess : 
	public ProcessImpl<LibraryFeatureCollectionProcess>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	LibraryFeatureCollectionProcess();

	//! Unit testing method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL(); 

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const ;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() 
	{ 
		return m_spFeatureDefn; 
	} 
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const 
	{ 
		return m_spFeatureDefn; 
	} 

	virtual PYXPointer<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const 
	{ 
		return m_vecStyles; 
	} 

	virtual bool STDMETHODCALLTYPE canRasterize(void) const {return true;}

private: 
	std::vector<FeatureStyle> m_vecStyles; 

// Library Outline Process
private:

	//! Create an iterator for the selected geometry (null geometry is global).
	PYXPointer<FeatureIterator> createIterator(PYXPointer<PYXGeometry> spGeom) const;

	//! Return the feature at the specified offset within the data block.
	PYXPointer<IFeature> getFeature(const ProcRef& procref) const;

	//! Open the specified excel file. 
	void open(const boost::filesystem::path& path);

	//! Set up the fields of data
	void initMetaData();

	//! Create the geometry for the process.
	PYXPointer<PYXGeometry> createGeometry();

	//! The native resolution for the process
	int m_nResolution;

	//! The table definition for the features in the collection
	PYXPointer<PYXTableDefinition> m_spFeatureDefn;

	//! The collection of library item procrefs that makes up the feature collection.
	std::vector<ProcRef> m_vecProcref;

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;

	friend class LibraryOutlineIterator;

	//! Iterate over all features in an associated XLS feature collection.
	class LIBRARY_DECL LibraryOutlineIterator : public FeatureIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<LibraryOutlineIterator> create(
			PYXPointer<PYXGeometry> spGeom,
			const std::vector<ProcRef>& vecProcref	)
		{
			return PYXNEW(LibraryOutlineIterator, spGeom, vecProcref	);
		}

		LibraryOutlineIterator(
			PYXPointer<PYXGeometry> spGeom,
			const std::vector<ProcRef>& vecProcref	) :
				m_spGeom(spGeom),
				m_vecProcref(vecProcref),
				m_nCurrent(0)
		{
			/*
				prime the iterator so that the current feature is sure to pass the
				geometry filter. Will set the iterator to end if no features are in the 
				geometry.
			*/
			getFeature();
		}

		//! Destructor
		virtual ~LibraryOutlineIterator() {;}

	public: // FeatureIterator

		//! Move to the next feature.
		virtual void next() 
		{
			++m_nCurrent;
			getFeature();
		}

		//! The end condition test.
		virtual bool end() const 
		{
			return static_cast<int>(m_vecProcref.size()) <= m_nCurrent;
		}

		//! Get the current PYXIS feature.
		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:

		//! The limiting geometry (if available)
		PYXPointer<PYXGeometry> m_spGeom;

		//! the total number of features to iterate over.
		std::vector<ProcRef> m_vecProcref;

		//! the current feature offset
		mutable int m_nCurrent;
	};

};

#endif
