#ifndef FEATURES_SUMMARY_FILTER_H
#define FEATURES_SUMMARY_FILTER_H
/******************************************************************************
features_summary_filter.h

begin		: March 21, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"

#include "generic_feature_group.h"
#include "data_collection.h"

class FeaturesSummaryFilterContext: public GenericFeaturesGroup::ContextWithHistograms
{
public:
	class Filter : public PYXObject
	{
	public:
		virtual Range<int> filter(boost::intrusive_ptr<IFeature> feature) = 0;
		virtual Range<int> filter(boost::intrusive_ptr<IFeatureGroup> group) = 0;
	};

public:
	const static int knCurrentVersion = 4;

public:
	virtual boost::intrusive_ptr<GenericFeaturesGroup> getRootGroup() const;

	virtual PYXPointer<GenericFeaturesGroup::ChildrenList> getChildren(const GenericFeaturesGroup & parent) const;

	virtual boost::intrusive_ptr<IFeature> getFeature(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & featureNode) const;
	virtual boost::intrusive_ptr<GenericFeaturesGroup> getGroup(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & groupNode) const;

	virtual boost::intrusive_ptr<IFeature> getFeature(const GenericFeaturesGroup & parent, const std::string & featureId) const;
	virtual std::string getFeatureGroupIdForFeature(const std::string & featureId) const;

	virtual PYXPointer<PYXTableDefinition> getFeatureDefinition(const GenericFeaturesGroup & parent) const;
	virtual PYXValue getFieldValue(const GenericFeaturesGroup & parent,int index) const;

	virtual Range<int> getFeaturesCount(const GenericFeaturesGroup & parent) const;
	virtual PYXPointer<PYXGeometry> getGeometry(const GenericFeaturesGroup & parent) const;
	virtual std::string getStyle(const GenericFeaturesGroup & parent) const;

	boost::intrusive_ptr<IFeatureGroup> getInputGroup(const GenericFeaturesGroup & parent) const;

	const PYXPointer<Filter> & getFilter() const { return m_filter; }

private:
	boost::intrusive_ptr<IFeatureGroup> m_inputFG;
	PYXPointer<Filter> m_filter;

private:
	class GroupFactory;
	class InputGroupFactory;

	boost::intrusive_ptr<GenericFeaturesGroup> generateFilteredGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup) const;
	void mergeSubGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup,
		GenericFeaturesGroup::GroupData & data,
		GenericFeaturesGroup::ChildrenList & children,
		PYXPointer<PYXTileCollection> geom,
		boost::recursive_mutex & mutex) const;

public:
	static PYXPointer<FeaturesSummaryFilterContext> create(const boost::intrusive_ptr<IFeatureGroup> & fg,const PYXPointer<PYXLocalStorage> & storage, const PYXPointer<Filter> & filter)
	{
		return PYXNEW(FeaturesSummaryFilterContext,fg,storage,filter);
	}

	FeaturesSummaryFilterContext(const boost::intrusive_ptr<IFeatureGroup> & fg,const PYXPointer<PYXLocalStorage> & storage, const PYXPointer<Filter> & filter)
		: GenericFeaturesGroup::ContextWithHistograms(storage), m_inputFG(fg), m_filter(filter)
	{
	}
};



//! A process that filters a feature group by a field value.
class MODULE_FEATURE_PROCESSING_PROCS_DECL FeaturesSummaryAttributeRangeFilter : 
	public ProcessImpl<FeaturesSummaryAttributeRangeFilter>, public IFeatureGroup
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeaturesSummaryAttributeRangeFilter();

	//! Destructor
	~FeaturesSummaryAttributeRangeFilter();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(FeaturesSummaryAttributeRangeFilter, IProcess);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

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

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public: // IFeatureGroup
	virtual Range<int> STDMETHODCALLTYPE getFeaturesCount() const;

	virtual bool STDMETHODCALLTYPE moreDetailsAvailable() const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(int fieldIndex) const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroup(const std::string & groupId) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroupForFeature(const std::string & featureId) const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator() const;
	
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator(const PYXGeometry& geometry) const;

public:
	static void test();

private:
	boost::intrusive_ptr<IFeatureGroup> m_spInputFG;

	PYXPointer<FeaturesSummaryFilterContext> m_context;
	boost::intrusive_ptr<GenericFeaturesGroup> m_rootGroup;

	PYXValueRange m_range;
	int m_fieldIndex;
};


//! A process that filters a feature group by a given geometry (intersects).
class MODULE_FEATURE_PROCESSING_PROCS_DECL FeaturesSummaryGeometryFilter : 
	public ProcessImpl<FeaturesSummaryGeometryFilter>, public IFeatureGroup
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeaturesSummaryGeometryFilter();

	//! Destructor
	~FeaturesSummaryGeometryFilter();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(FeaturesSummaryGeometryFilter, IProcess);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

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

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public: // IFeatureGroup
	virtual Range<int> STDMETHODCALLTYPE getFeaturesCount() const;

	virtual bool STDMETHODCALLTYPE moreDetailsAvailable() const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(int fieldIndex) const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroup(const std::string & groupId) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroupForFeature(const std::string & featureId) const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator() const;
	
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator(const PYXGeometry& geometry) const;

public:
	static void test();

private:
	boost::intrusive_ptr<IFeatureGroup> m_spInputFG;
	boost::intrusive_ptr<IFeature> m_spInputGeometry;

	PYXPointer<FeaturesSummaryFilterContext> m_context;
	boost::intrusive_ptr<GenericFeaturesGroup> m_rootGroup;	
};


#endif // guard
