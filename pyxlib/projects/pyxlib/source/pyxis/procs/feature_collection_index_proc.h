#ifndef PYXIS__PROCS__FEATURE_COLLECTION_INDEX_PROC_H
#define PYXIS__PROCS__FEATURE_COLLECTION_INDEX_PROC_H
/******************************************************************************
feature_collection_index_proc.h

begin		: 2013-05-28
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/data/feature_collection_index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"

#include <set>



/*!
Index a Feature Collection fields to provide fast lookup and auto-completion features.
*/
//! Index a Feature Collection fields to provide fast lookup and auto-completion features.
class PYXLIB_DECL FeatureCollectionIndexProcess : public ProcessImpl<FeatureCollectionIndexProcess>, public IFeatureCollectionIndex
{
	PYXCOM_DECLARE_CLASS();

public:

	FeatureCollectionIndexProcess();

	//convenience constuctores
	FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::string & fieldName);
	FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::vector<std::string> & fieldsName);
	FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, int fieldIndex);
	FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::vector<int> & fieldsIndices);

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollectionIndex)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(FeatureCollectionIndexProcess, IProcess);

public: //IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollectionIndex*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollectionIndex*>(this);
	}

	
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);
	
protected:

	virtual IProcess::eInitStatus initImpl();	

public: // IFeatureCollectionIndex

	//return all features that contain the given value
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXValue & value) const;

	//return all features that contains the given value and intersects with the given geometry
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry,const PYXValue & value) const;

	//return a list of possible values to complete given an input value
	virtual std::vector<PYXValue> STDMETHODCALLTYPE suggest(const PYXValue & value) const ;

public: // misc

	static void test();

private:
	friend class IndexedFeatureIterator;
	typedef std::vector<std::string> FeaturesIDList;

private:
	void initIndex();
	FeaturesIDList findMatchingFeatures(const std::string & words) const;
	FeaturesIDList findMatchingFeatures(const std::set<std::string> & words) const;

private:
	//! Table definition.
	boost::intrusive_ptr<IFeatureCollection> m_inputFC;

	//! fields to index.
	std::vector<int> m_fieldIndices;

	//! Storage for the index class
	PYXPointer<PYXLocalStorage> m_storage;
};


#endif // guard
