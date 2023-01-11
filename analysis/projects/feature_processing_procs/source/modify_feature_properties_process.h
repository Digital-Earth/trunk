#ifndef MODIFY_FEATURE_PROPERTIES_PROCESS_H
#define MODIFY_FEATURE_PROPERTIES_PROCESS_H
/******************************************************************************
modify_feature_properties_process.h

begin		: aug 21, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/procs/feature_calculator.h"

#include "pyxis/data/feature_group.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"

#include "generic_feature_group.h"
#include "data_collection.h"

//! A process that filters a feature collection by resolution.
class MODULE_FEATURE_PROCESSING_PROCS_DECL ModifyFeaturePropertiesProcess : 
	public ProcessImpl<ModifyFeaturePropertiesProcess>, public IFeatureGroup
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	ModifyFeaturePropertiesProcess()
	{}

	//! Destructor
	~ModifyFeaturePropertiesProcess()
	{}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(ModifyFeaturePropertiesProcess, IProcess);

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

private:
	class Context;

	class State : public PYXObject
	{
	public:
		static PYXPointer<State> create()
		{
			return PYXNEW(State);
		}

		State()
		{
		}

		boost::intrusive_ptr<IFeatureGroup> inputFG;

		PYXPointer<Context> context;
		boost::intrusive_ptr<GenericFeaturesGroup> rootGroup;

		PYXPointer<PYXTableDefinition> outputDefinition;
		std::vector<boost::intrusive_ptr<IFeatureCalculator>> calculators;
	};

private:

	class Context : public GenericFeaturesGroup::ContextWithHistograms
	{
	public:
		const static int knCurrentVersion = 4;

	public:
		virtual boost::intrusive_ptr<GenericFeaturesGroup> getRootGroup() const;

		//return list of children nodes.
		virtual PYXPointer<GenericFeaturesGroup::ChildrenList> getChildren(const GenericFeaturesGroup & parent) const;

		//create feature/group from a nodeData info
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

		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,const PYXGeometry & geometry, int nFieldIndex) const;
		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,int nFieldIndex) const;

	private:
		PYXPointer<State> m_state;

	private:
		class GroupFactory;
		class InputGroupFactory;

		boost::intrusive_ptr<GenericFeaturesGroup> generateFilteredGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup) const;
		void mergeSubGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup,GenericFeaturesGroup::GroupData & data,PYXPointer<PYXTileCollection> geom,boost::recursive_mutex & mutex) const;

	public:
		static PYXPointer<Context> create(const PYXPointer<State> & state,const PYXPointer<PYXLocalStorage> & storage)
		{
			return PYXNEW(Context,state,storage);
		}

		Context(const PYXPointer<State> & state,const PYXPointer<PYXLocalStorage> & storage)
			: GenericFeaturesGroup::ContextWithHistograms(storage), m_state(state) 
		{
		}
	};

private:
	bool createOutputDefinition(State & state);

	class ChildrenFactory;

private:
	PYXPointer<State> m_state;
	mutable boost::recursive_mutex m_stateMutex;

};

#endif // guard
