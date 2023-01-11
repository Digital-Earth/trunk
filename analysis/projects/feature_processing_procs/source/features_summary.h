#ifndef FEATURES_SUMMARY_H
#define FEATURES_SUMMARY_H
/******************************************************************************
features_summary.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/pipe/pyxnet_channel.h"

#include "generic_feature_group.h"
#include "data_collection.h"


//! A process that filters a feature collection by resolution.
class MODULE_FEATURE_PROCESSING_PROCS_DECL FeaturesSummary : 
	public ProcessImpl<FeaturesSummary>, public IFeatureGroup
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeaturesSummary();

	//! Destructor
	~FeaturesSummary();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(FeaturesSummary, IProcess);

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

	virtual IProcess::eInitStatus STDMETHODCALLTYPE initProc(bool bRecursive = false);

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
	class GenericFeature : public IFeature
	{
	public:
		IUNKNOWN_QI_BEGIN
			IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

	public:
		static PYXPointer<PYXConstBufferSlice> serializeFeature(const boost::intrusive_ptr<IFeature> & feature);

		static boost::intrusive_ptr<IFeature> deserialize(const PYXPointer<PYXTableDefinition> featureDefinition, 
			const std::string & featureID, 
			const PYXConstBufferSlice & in)
		{
			return new GenericFeature(featureDefinition,featureID,in);
		}

		static PYXPointer<PYXGeometry> deserializeGeometryOnly(const PYXConstBufferSlice & in);		

	private:
		GenericFeature(const PYXPointer<PYXTableDefinition> featureDefinition, 
			const std::string & featureID, 
			const PYXConstBufferSlice & in);

	public: // IRecord
		IRECORD_IMPL();

	public: // IFeature
		virtual bool STDMETHODCALLTYPE isWritable() const
		{
			return false;
		}

		virtual const std::string& STDMETHODCALLTYPE getID() const
		{
			return m_strID;
		}

		virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
		{
			createGeometry();

			return m_spGeom;
		}

		virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
		{
			createGeometry();

			return m_spGeom;
		}

		virtual std::string STDMETHODCALLTYPE getStyle() const
		{
			return "";
		}

		virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
		{
			return "";
		}

	private:
		void createGeometry() const;

	private:
		PYXConstBufferSlice m_serializedGeometry;
		mutable PYXPointer<PYXGeometry> m_spGeom;
		std::string m_strID;
	};

	class Context : public GenericFeaturesGroup::ContextWithHistograms
	{
	public:
		const static int knCurrentVersion = 6;

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

	private:
		boost::intrusive_ptr<IFeatureCollection> m_fc;
		PYXPointer<PYXTableDefinition> m_featureDefinition;
		std::string m_strStyle;
		int m_memoryLimit;

	private:
		class GroupFactory;

	public:
		static PYXPointer<Context> create(
			const boost::intrusive_ptr<IFeatureCollection> & fc,
			const PYXPointer<PYXTableDefinition> & featureDefinition, 
			const std::string & style,
			const PYXPointer<PYXLocalStorage> & storage,
			int memoryLimit)
		{
			return PYXNEW(Context,fc,featureDefinition,style,storage,memoryLimit);
		}

		Context(
			const boost::intrusive_ptr<IFeatureCollection> & fc,
			const PYXPointer<PYXTableDefinition> & featureDefinition, 
			const std::string & style,
			const PYXPointer<PYXLocalStorage> & storage,
			int memoryLimit)
			: GenericFeaturesGroup::ContextWithHistograms(storage), m_fc(fc), m_featureDefinition(featureDefinition), m_strStyle(style), m_memoryLimit(memoryLimit)
		{
		}
	};
	
	friend class GroupCreator;

private: 
	//Utility Functions
	//void measureLocalStorageUsage();

	void initializeLocalStorage();

	// notify pyxnet we are sharing our data
	void publishPyxnetChannel();

	// wrapper call to make sure this pipeline is not get desotried before publishPyxnetChannel get called
	static void pointerSafePublishPyxnetChannel(boost::intrusive_ptr<FeaturesSummary> self);

	//! Check to see if our input process is initialized and ready to go.
	bool inputIsOK() const;

	//! Check if input failed because of a missing SRS
	bool inputHasMissingSRS();

	//! Check if input failed because of a missing user credentials
	bool inputHasMissingUserCredentials();

	//! Check if input failed because of missing or empty geometry
	bool inputHasMissingGeometry();

	//! Check if input failed because of a missing world file
	bool inputHasMissingWorldFile();

	//! load all needed metadata from pyxnet
	void downloadMetadata();

	//! store metadata on local storage
	void storeMetadata();

private:
	boost::intrusive_ptr<IFeatureCollection> m_spInputFC;

	PYXPointer<PYXTableDefinition> m_featuresDefinition;

	PYXPointer<PYXBufferedLocalStorage> m_localStorageBuffered;
	PYXPointer<PYXLocalStorage> m_localStorage;
	PYXPointer<PYXLocalStorage> m_localStorageWithPyxnet;

	PYXPointer<Context> m_context;
	boost::intrusive_ptr<GenericFeaturesGroup> m_rootGroup;	

	PYXPointer<PYXNETChannel> m_channel;
};

#endif // guard
