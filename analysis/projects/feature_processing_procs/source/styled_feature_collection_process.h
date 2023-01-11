#ifndef styled_feature_collection_process_H
#define styled_feature_collection_process_H
/******************************************************************************
styled_feature_collection_process.h

begin		: June 11, 2008
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/pipe/process.h"


/*!
Input: Feature Collection
Output: Styled Feature Collection
*/
//! Applies a user-specified style to a feature collection.
class MODULE_FEATURE_PROCESSING_PROCS_DECL StyledFeatureCollectionProcess : 
	public ProcessImpl<StyledFeatureCollectionProcess>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	StyledFeatureCollectionProcess();

	//! Destructor
	~StyledFeatureCollectionProcess();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(StyledFeatureCollectionProcess, IProcess);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const;

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput();

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;
	
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;
	
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public:

	static void test();

public :

	//! The attribute name for use in Process Editor.
	static const std::string kstrLineColour;

	//! The attribute name for use in Process Editor.
	static const std::string kstrIcon;

private:

	//! The input to this process.
	boost::intrusive_ptr<IFeatureCollection> m_spInputFC;

	//! The line colour style specified by the user.
	std::string m_strLineColour;

	//! The icon specified by the user.
	std::string m_strIconID;

	void updateStyle();

	//! The process mutex.
	boost::recursive_mutex m_procMutex;

};

#endif // guard
