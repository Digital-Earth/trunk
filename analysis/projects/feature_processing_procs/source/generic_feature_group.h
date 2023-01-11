#ifndef GENERIC_FEATURE_GROUP_H
#define GENERIC_FEATURE_GROUP_H
/******************************************************************************
generic_feature_group.h

begin		: Apr 09, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/data/histogram.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/utility/thread_pool.h"

#include "data_collection.h"
//#include "histogram.h"

#include <boost/algorithm/string/predicate.hpp>

class GenericFeaturesGroup : public IFeatureGroup
{
public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();


public:
	enum NodeType
	{
		knFeature,
		knGroup
	};

	class NodeData : public PYXObject, ObjectMemoryUsageCounter<NodeData>
	{
	public:
		NodeType nodeType;
		std::string id;
		PYXBoundingCircle circle;
		mutable PYXConstBufferSlice serializedData;

	public:
		static PYXPointer<NodeData> create()
		{
			return PYXNEW(NodeData);
		}

		NodeData()
		{
		}

		virtual void createSerializedData() const
		{
		}

		virtual void readSerializedData() 
		{
		}
	};

	class GroupData : public NodeData
	{
	public:
		Range<int> featuresCount;
		PYXConstBufferSlice serializedGeometry;

	public:
		static PYXPointer<GroupData> create()
		{
			return PYXNEW(GroupData);
		}

		GroupData() : featuresCount(0)
		{
			nodeType = knGroup;
		}

		virtual void createSerializedData() const
		{
			PYXStringWireBuffer buffer;
			int delta = featuresCount.min;
			buffer << PYXCompactInteger(delta);
			delta = featuresCount.max-featuresCount.min;
			buffer << PYXCompactInteger(delta) << serializedGeometry;

			serializedData = *buffer.getBuffer();

		}

		virtual void readSerializedData() 
		{
			PYXConstWireBuffer buffer(serializedData);
			int delta;
			buffer >> PYXCompactInteger(delta);
			featuresCount.min = delta;
			buffer >> PYXCompactInteger(delta);
			featuresCount.max = featuresCount.min + delta;
			buffer >> serializedGeometry;
		}
	};

	class ChildrenList : public PYXObject, ObjectMemoryUsageCounter<ChildrenList>
	{
	public:
		typedef std::vector<PYXPointer<NodeData>> List;
		List childrenNodes;

	public:
		static PYXPointer<ChildrenList> create()
		{
			return PYXNEW(ChildrenList);
		}

		ChildrenList()
		{
		}

	public:
		PYXPointer<NodeData> findFeature(const std::string & id) const
		{
			for(List::const_iterator it = childrenNodes.begin(); it != childrenNodes.end(); ++it)
			{
				if ((*it)->nodeType == knFeature && (*it)->id == id)
				{
					return *it;
				}
			}
			return NULL;
		}

		PYXPointer<NodeData> findGroup(const std::string & id) const
		{
			for(List::const_iterator it = childrenNodes.begin(); it != childrenNodes.end(); ++it)
			{
				if ((*it)->nodeType == knGroup && (*it)->id == id)
				{
					return *it;
				}
			}
			return NULL;
		}
	};

public:
	class Context : public PYXObject
	{
	public:
		virtual boost::intrusive_ptr<GenericFeaturesGroup> getRootGroup() const = 0;

		//return list of children nodes.
		virtual PYXPointer<ChildrenList> getChildren(const GenericFeaturesGroup & parent) const = 0;

		//create feature/group from a nodeData info
		virtual boost::intrusive_ptr<IFeature> getFeature(const GenericFeaturesGroup & parent, const PYXPointer<NodeData> & featureNode) const = 0;
		virtual boost::intrusive_ptr<GenericFeaturesGroup> getGroup(const GenericFeaturesGroup & parent, const PYXPointer<NodeData> & groupNode) const = 0;

		//create a feature from an ID, method should work even if the featureId is not the parent child.
		virtual boost::intrusive_ptr<IFeature> getFeature(const GenericFeaturesGroup & parent, const std::string & featureId) const = 0;
		virtual std::string getFeatureGroupIdForFeature(const std::string & featureId) const = 0;

		virtual PYXPointer<PYXTableDefinition> getFeatureDefinition(const GenericFeaturesGroup & parent) const = 0;
		virtual PYXValue getFieldValue(const GenericFeaturesGroup & parent,int index) const = 0;

		virtual Range<int> getFeaturesCount(const GenericFeaturesGroup & parent) const = 0;
		virtual PYXPointer<PYXGeometry> getGeometry(const GenericFeaturesGroup & parent) const = 0;
		virtual std::string getStyle(const GenericFeaturesGroup & parent) const = 0;

		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,const PYXGeometry & geometry, int nFieldIndex) const = 0;
		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,int nFieldIndex) const = 0;


	protected:
		PYXDataCollection & getDataCollectionForGroup(const GenericFeaturesGroup & parent) const
		{
			return *parent.m_dataCollection;
		}

		const NodeData & getDataForGroup(const GenericFeaturesGroup & parent) const
		{
			return *(parent.m_data);
		}
	};

	class ContextWithStorage : public Context
	{
	protected:
		mutable PYXPointer<PYXLocalStorage> m_storage;

	public:
		ContextWithStorage(const PYXPointer<PYXLocalStorage> & storage) : m_storage(storage) {};

	public:
		const PYXPointer<PYXLocalStorage> & getStorage() const { return m_storage; };
	};

	class ContextWithHistograms : public ContextWithStorage
	{
	public:
		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,const PYXGeometry & geometry, int nFieldIndex) const;
		virtual PYXPointer<PYXHistogram> getFieldHistogram(const GenericFeaturesGroup & parent,int nFieldIndex) const;

	public:
		ContextWithHistograms(const PYXPointer<PYXLocalStorage> & storage) : ContextWithStorage(storage) {};
	};


protected:
	boost::intrusive_ptr<GenericFeaturesGroup> getGroup(const PYXPointer<NodeData> & groupData) const
	{
		return m_context->getGroup(*this,groupData);
	}

public:
	const PYXBoundingCircle & getBoundingCircle() const { return m_data->circle; }

	const PYXPointer<NodeData> & getNodeData() const { return m_data; }

	boost::intrusive_ptr<GenericFeaturesGroup> getGroup(const std::string & groupId) const
	{
		return getGroup(m_context->getChildren(*this)->findGroup(groupId));
	}

	boost::intrusive_ptr<GenericFeaturesGroup> findGroup(const std::string & groupId) const
	{
		if (this->getID() == groupId)
		{
			return boost::intrusive_ptr<GenericFeaturesGroup>(const_cast<GenericFeaturesGroup*>(this));
		}

		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knGroup)
			{
				continue;
			}
			if (ref->id == groupId)
			{
				return m_context->getGroup(*this,ref);
			}
			else if (boost::starts_with(groupId,ref->id))
			{
				boost::intrusive_ptr<GenericFeaturesGroup> foundedGroup = m_context->getGroup(*this,ref)->findGroup(groupId);
				if (foundedGroup)
				{
					return foundedGroup;
				}
			}
		}
		return NULL;
	}

	void visitSubGroups(const boost::function<void(const PYXPointer<GenericFeaturesGroup> &)> & visitor) const
	{
		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knGroup)
			{
				continue;
			}
			visitor(getGroup(ref));
		}
	}

	void visitSubGroups(const boost::function<void(const PYXPointer<GenericFeaturesGroup> &)> & visitor,const PYXVectorRegion & region) const
	{
		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knGroup)
			{
				continue;
			}
			if (region.intersects(ref->circle) != PYXRegion::knNone)
			{
				visitor(getGroup(ref));
			}
		}
	}

	PYXPointer<PYXTaskWithContinuation> visitSubGroupsParallel(const boost::function<void(const PYXPointer<GenericFeaturesGroup> &)> & visitor) const
	{
		PYXPointer<PYXTaskWithContinuation> task = PYXTaskWithContinuation::create();

		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knGroup)
			{
				continue;
			}
			task->startChild(boost::bind(visitor,getGroup(ref)));
		}

		task->start();
		return task;
	}

	PYXPointer<PYXTaskWithContinuation> visitSubGroupsParallel(const boost::function<void(const PYXPointer<GenericFeaturesGroup> &)> & visitor,const PYXGeometry & geometry) const
	{
		PYXPointer<PYXTaskWithContinuation> task = PYXTaskWithContinuation::create();

		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knGroup)
			{
				continue;
			}
			if (geometry.intersects(PYXVectorGeometry(ref->circle)))
			{
				task->startChild(boost::bind(visitor,getGroup(ref)));
			}
		}

		task->start();
		return task;
	}

	void visitFeatures(const boost::function<void(const boost::intrusive_ptr<IFeature>&)> & visitor) const
	{
		PYXPointer<ChildrenList> children = m_context->getChildren(*this);
		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knFeature)
			{
				continue;
			}
			visitor(m_context->getFeature(*this,ref));
		}
	}

	void visitFeatures(const boost::function<void(const boost::intrusive_ptr<IFeature>&)> & visitor,const PYXGeometry & geometry) const
	{
		PYXPointer<ChildrenList> children = m_context->getChildren(*this);

		for(int i = 0; i < (int)children->childrenNodes.size(); i++)
		{
			const PYXPointer<NodeData> & ref = children->childrenNodes[i];
			if (ref->nodeType != knFeature)
			{
				continue;
			}
			if (geometry.intersects(PYXVectorGeometry(ref->circle)))
			{
				visitor(m_context->getFeature(*this,ref));
			}
		}
	}

public:
	static boost::intrusive_ptr<GenericFeaturesGroup> create(PYXPointer<const Context> context)
	{
		return context->getRootGroup();
	}

public:
	GenericFeaturesGroup(const PYXPointer<const Context> & context,const PYXPointer<NodeData> & data) : m_data(data), m_context(context), m_dataCollection(PYXDataCollection::create())
	{
		assert(m_data->nodeType == knGroup);
	}

private:
	PYXPointer<NodeData> m_data;
	
	PYXPointer<const Context> m_context;
	mutable PYXPointer<PYXDataCollection> m_dataCollection;

public: // IRecord
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_context->getFeatureDefinition(*this);
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_context->getFeatureDefinition(*this);
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{ 
		assert((nFieldIndex >= 0) && "Invalid argument");
		return m_context->getFieldValue(*this,nFieldIndex);
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		return;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		int nField = getDefinition()->getFieldIndex(strName);
		if (0 <= nField)
		{
			return getFieldValue(nField);
		}
		return PYXValue();
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		int nField = getDefinition()->getFieldIndex(strName);
		if (0 <= nField)
		{
			setFieldValue(value, nField);
		}
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{ 
		int nFieldCount = getDefinition()->getFieldCount(); 
		std::vector<PYXValue> vecValues; 
		for (int nField = 0; nField < nFieldCount; ++nField)
		{ 
			vecValues.push_back(getFieldValue(nField));
		}
		return vecValues; 
	}

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues) 
	{ 
		return;
	}

	virtual void STDMETHODCALLTYPE addField(	const std::string& strName, 
												PYXFieldDefinition::eContextType nContext, 
												PYXValue::eType nType, 
												int nCount = 1, 
												PYXValue value = PYXValue()	) 
	{
		return;
	}

public: // IFeature
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_data->id;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_context->getGeometry(*this);
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_context->getGeometry(*this);
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_context->getStyle(*this);
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		std::string style = m_context->getStyle(*this);
		if (style.size() == 0)
		{
			return "";
		}
		PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(style);
		return styleDoc->getNodeText("/style/" + strStyleToGet);
	}

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const
	{
		return std::vector<FeatureStyle>();
	}

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const
	{
		return m_context->getFeature(*this,strFeatureID);
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_context->getFeatureDefinition(*this);
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_context->getFeatureDefinition(*this);
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return true;
	}

	IFEATURECOLLECTION_IMPL_HINTS();

public: // IFeatureGroup
	virtual Range<int> STDMETHODCALLTYPE getFeaturesCount() const
	{
		return m_context->getFeaturesCount(*this);
	}

	virtual bool STDMETHODCALLTYPE moreDetailsAvailable() const
	{
		return true;
	}

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(int fieldIndex) const
	{
		return m_context->getFieldHistogram(*this,fieldIndex);
	}

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
	{
		return m_context->getFieldHistogram(*this,geometry,fieldIndex);
	}

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroup(const std::string & groupId) const
	{
		return findGroup(groupId);
	}

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroupForFeature(const std::string & featureId) const
	{
		return findGroup(m_context->getFeatureGroupIdForFeature(featureId));
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator() const
	{
		return GroupIterator::create(this);
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator(const PYXGeometry & geometry) const
	{
		return GroupIterator::create(this,geometry.clone());
	}

private:
	bool isFieldNumeric(int fieldIndex) const
	{
		return getFeatureDefinition()->getFieldDefinition(fieldIndex).isNumeric();
	}

private:
	class GroupIterator : public FeatureIterator
	{
	private:
		PYXPointer<const GenericFeaturesGroup> m_group;
		PYXPointer<PYXGeometry> m_geometry;
		PYXPointer<ChildrenList> m_children;
		ChildrenList::List::const_iterator m_childrenIt;

	public:
		static PYXPointer<GroupIterator> create(const PYXPointer<const GenericFeaturesGroup> & group)
		{
			return PYXNEW(GroupIterator,group,0);
		}

		static PYXPointer<GroupIterator> create(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry)
		{
			return PYXNEW(GroupIterator,group,geometry);
		}

		GroupIterator(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry);

		virtual ~GroupIterator();

	public:
		virtual bool end() const;

		virtual void next();

		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:
		void findNextMatch();
	};

	/*
	[shatzi]:Replaced by using FeatureIteratorLinq::selectMany.

	//Iterator over features only - this feature Iterator will not return GroupFeatures, only feature
	class Iterator : public FeatureIterator
	{
	private:
		PYXPointer<const GenericFeaturesGroup> m_group;
		PYXPointer<PYXGeometry> m_geometry;
		PYXPointer<ChildrenList> m_children;
		ChildrenList::List::const_iterator m_childrenIt;
		PYXPointer<Iterator> m_curChildIt;

	public:
		static PYXPointer<Iterator> create(const PYXPointer<const GenericFeaturesGroup> & group)
		{
			return PYXNEW(Iterator,group,0);
		}

		static PYXPointer<Iterator> create(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry)
		{
			return PYXNEW(Iterator,group,geometry);
		}

		Iterator(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry);

		virtual ~Iterator();

	public:
		virtual bool end() const;

		virtual void next();

		virtual boost::intrusive_ptr<IFeature> getFeature() const;
	
	private:
		void findNextMatch();
	};
	*/
};

PYXWireBuffer & operator>>(PYXWireBuffer & buffer,GenericFeaturesGroup::NodeData & data);
PYXWireBuffer & operator<<(PYXWireBuffer & buffer,const GenericFeaturesGroup::NodeData & data);

PYXWireBuffer & operator>>(PYXWireBuffer & buffer,GenericFeaturesGroup::ChildrenList & data);
PYXWireBuffer & operator<<(PYXWireBuffer & buffer,const GenericFeaturesGroup::ChildrenList & data);

#endif // guard
