/******************************************************************************
modify_feature_properties_process.cpp

begin		: aug 21, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "modify_feature_properties_process.h"


#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

#include "pyxis/region/circle_region.h"

#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

#include "boost/scoped_ptr.hpp"

// standard includes
#include <cassert>
#include <set>

// {DF99F5EE-4C75-41c2-AC08-E3F5DEB808D1}
PYXCOM_DEFINE_CLSID(ModifyFeaturePropertiesProcess, 
0xdf99f5ee, 0x4c75, 0x41c2, 0xac, 0x8, 0xe3, 0xf5, 0xde, 0xb8, 0x8, 0xd1);



PYXCOM_CLASS_INTERFACES(ModifyFeaturePropertiesProcess, IProcess::iid, IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ModifyFeaturePropertiesProcess, "Add Field", "Adds a field to a feature group", "Analysis/Features/Calculator",
					IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Feature Group", "A feature group.")
	IPROCESS_SPEC_PARAMETER(IFeatureCalculator::iid, 1, -1, "Attributes Calculator", "calculators for generating the attributes.")
IPROCESS_SPEC_END


////////////////////////////////////////////////////////////////////////////////
// ExtendedFeature
////////////////////////////////////////////////////////////////////////////////

class ExtendedFeature : public IFeature
{
public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IRecord)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

public:
	static boost::intrusive_ptr<IFeature> deserialize(
		const boost::intrusive_ptr<IFeature> baseFeature,
		const PYXPointer<PYXTableDefinition> featureDefinition,
		const PYXConstBufferSlice & in)
	{
		return new ExtendedFeature(baseFeature,featureDefinition,in);
	}



public: // IRecord
	IRECORD_IMPL();

public: // IFeature
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_baseFeature->getID();
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_baseFeature->getGeometry();
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_baseFeature->getGeometry();
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

	boost::intrusive_ptr<IFeature> m_baseFeature;

private:
	ExtendedFeature(
		const boost::intrusive_ptr<IFeature> baseFeature,
		const PYXPointer<PYXTableDefinition> featureDefinition, 
		const PYXConstBufferSlice & in)
		: m_baseFeature(baseFeature),
		m_spDefn(featureDefinition),
		m_vecValues(baseFeature->getFieldValues())
	{
		PYXConstWireBuffer metadataBuffer(in);

		for(int i=(int)m_vecValues.size(); i < featureDefinition->getFieldCount(); ++i)
		{
			unsigned char hasValue;

			metadataBuffer >> hasValue;

			if (!hasValue)
			{
				m_vecValues.push_back(PYXValue());
				continue;
			}

			const PYXFieldDefinition & fieldDefinition = featureDefinition->getFieldDefinition(i);

			if (fieldDefinition.getType() == PYXValue::knString && fieldDefinition.getCount()==1)
			{
				//HACK: if the type is string with count = 1, then fieldDefinition.getTypeCompatibleValue() fails...
				std::string temp;
				metadataBuffer >> temp;

				m_vecValues.push_back(PYXValue(temp));
				continue;
			}

			PYXValue value = fieldDefinition.getTypeCompatibleValue();

			for(int v=0;v<fieldDefinition.getCount();++v)
			{
				switch (fieldDefinition.getType())
				{
				case PYXValue::knBool:
					{
						unsigned char c;
						metadataBuffer >> c;
						value.setBool(v,c!=0);
					}
					break;
				case PYXValue::knChar:
					{
						unsigned char c;
						metadataBuffer >> c;
						value.setChar(v,static_cast<char>(c));
					}
					break;
				case PYXValue::knInt8:
				case PYXValue::knInt16:
				case PYXValue::knInt32:
					{
						int temp;
						metadataBuffer >> temp;
						value.setInt(v,temp);
					}
					break;

				case PYXValue::knUInt8:
				case PYXValue::knUInt16:
				case PYXValue::knUInt32:
					{
						int temp;
						metadataBuffer >> temp;
						value.setUInt(v,static_cast<unsigned int>(temp));
					}
					break;

				case PYXValue::knReservedForInt64:
				case PYXValue::knReservedForUInt64:
					PYXTHROW(PYXException,"unsupported format " << fieldDefinition.getType());

				case PYXValue::knFloat:
				case PYXValue::knDouble:
					{
						double temp;
						metadataBuffer >> temp;
						value.setDouble(v,temp);
					}
					break;

				case PYXValue::knString:
					{
						std::string temp;
						metadataBuffer >> temp;
						value.setString(v,temp);
					}
					break;
				}
			}

			m_vecValues.push_back(value);
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
// ChildrenFactory
////////////////////////////////////////////////////////////////////////////////

class ModifyFeaturePropertiesProcess::ChildrenFactory : public PYXDataCollection::DataItemFactory<GenericFeaturesGroup::ChildrenList>
{
private:
	const PYXPointer<PYXLocalStorage> & m_storage;
	const std::string & m_id;
	const boost::intrusive_ptr<IFeatureGroup> & m_inputGroup;
	const ModifyFeaturePropertiesProcess::State & m_state;
	static const std::string m_name;

protected:
	PYXPointer<PYXConstBufferSlice> calculateFeatureAttributes(const boost::intrusive_ptr<IFeature> & feature) const
	{
		PYXStringWireBuffer metadataBuffer;

		for(int c=0;c<(int)m_state.calculators.size();++c)
		{
			std::vector<PYXValue> values;

			values = m_state.calculators[c]->calculateValues(feature)->getFieldValues();

			PYXPointer<const PYXTableDefinition> definition = m_state.calculators[c]->getOutputDefinition();
			for(int i=0; i < definition->getFieldCount(); ++i)
			{
				const PYXFieldDefinition & fieldDefinition = definition->getFieldDefinition(i);
				const PYXValue & value = values[i];

				if (value.isNull())
				{
					metadataBuffer << (unsigned char)0;
					continue;
				}
				else
				{
					metadataBuffer << (unsigned char)1;
				}

				if (value.isArrayNullable())
				{
					//TODO: what we should do if the array is nullable?
					PYXTHROW(PYXException,"We don't support nullable array yet...");
				}

				for(int v=0;v<fieldDefinition.getCount();++v)
				{

					switch (fieldDefinition.getType())
					{
					case PYXValue::knBool:
						metadataBuffer << static_cast<unsigned char>(value.getBool(v));
						break;
					case PYXValue::knChar:
						metadataBuffer << static_cast<unsigned char>(value.getChar(v));
						break;
					case PYXValue::knInt8:
					case PYXValue::knInt16:
					case PYXValue::knInt32:
						metadataBuffer << value.getInt32(v);
						break;

					case PYXValue::knUInt8:
					case PYXValue::knUInt16:
					case PYXValue::knUInt32:
						metadataBuffer << static_cast<int>(value.getInt32(v));
						break;

					case PYXValue::knReservedForInt64:
					case PYXValue::knReservedForUInt64:
						PYXTHROW(PYXException,"unsupported format " << fieldDefinition.getType());

					case PYXValue::knFloat:
					case PYXValue::knDouble:
						metadataBuffer << value.getDouble(v);
						break;

					case PYXValue::knString:
						metadataBuffer <<value.getString(v);
					}
				}
			}
		}

		return metadataBuffer.getBuffer();
	}

public:
	ChildrenFactory(
		const PYXPointer<PYXLocalStorage> & storage,
		const boost::intrusive_ptr<IFeatureGroup> & inputGroup,
		const ModifyFeaturePropertiesProcess::State & state,
		const std::string & id) 
		: m_storage(storage), m_inputGroup(inputGroup), m_state(state), m_id(id)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<GenericFeaturesGroup::ChildrenList> & item,int & itemSize) const
	{		
		std::string key = "g:"+m_id+":cl";
		boost::scoped_ptr<PYXConstWireBuffer> dataBuffer(m_storage->get(key));
		if (dataBuffer)
		{
			PYXPointer<GenericFeaturesGroup::ChildrenList> cl = GenericFeaturesGroup::ChildrenList::create();

			(*dataBuffer) >> *cl;

			item = cl;
			itemSize = 2 * sizeof(GenericFeaturesGroup::NodeData) * cl->childrenNodes.size();
		}
		else
		{
			PYXPointer<FeatureIterator> iterator = m_inputGroup->getGroupIterator();
			PYXPointer<GenericFeaturesGroup::ChildrenList> cl = GenericFeaturesGroup::ChildrenList::create();

			for(;!iterator->end();iterator->next())
			{
				boost::intrusive_ptr<IFeature> feature = iterator->getFeature();
				boost::intrusive_ptr<IFeatureGroup> subGroup = feature->QueryInterface<IFeatureGroup>();

				if (subGroup)
				{
					PYXPointer<GenericFeaturesGroup::GroupData> groupData = GenericFeaturesGroup::GroupData::create();
					groupData->nodeType = GenericFeaturesGroup::knGroup;
					groupData->circle = subGroup->getGeometry()->getBoundingCircle();
					groupData->id = subGroup->getID();
					groupData->featuresCount = subGroup->getFeaturesCount();
					groupData->serializedGeometry = PYXConstBufferSlice();

					cl->childrenNodes.push_back(groupData);	
				}
				else
				{
					PYXPointer<GenericFeaturesGroup::NodeData> nodeData = GenericFeaturesGroup::NodeData::create();
					nodeData->nodeType = GenericFeaturesGroup::knFeature;
					nodeData->circle = feature->getGeometry()->getBoundingCircle();
					nodeData->id = feature->getID();
					nodeData->serializedData = *calculateFeatureAttributes(feature);

					cl->childrenNodes.push_back(nodeData);	
				}
			}

			PYXStringWireBuffer buffer;
			buffer << *cl;

			m_storage->set(key,buffer);

			item = cl;
			itemSize = 2 * sizeof(GenericFeaturesGroup::NodeData) * cl->childrenNodes.size();
		}
	}
};

const std::string ModifyFeaturePropertiesProcess::ChildrenFactory::m_name = "cl";


class ModifyFeaturePropertiesProcess::Context::GroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>>
{
private:
	const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & m_context;
	const PYXPointer<GenericFeaturesGroup::NodeData> & m_data;
	const std::string & m_name;
public:
	GroupFactory(const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & context,
		const PYXPointer<GenericFeaturesGroup::NodeData> & data,
		const std::string & name) : m_context(context), m_data(data), m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>> & item,int & itemSize) const
	{
		item = PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>::create(new GenericFeaturesGroup(m_context,m_data));
		itemSize = 20; //to indicate we allocated some memory...
	}
};

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	return mapAttr;
}

std::string STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"ModifyFeaturePropertiesProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
}

IProcess::eInitStatus ModifyFeaturePropertiesProcess::initImpl()
{
	PYXPointer<State> newState = State::create();

	newState->inputFG = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();

	if (!newState->inputFG)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input is not IFeatureGroup");
		return knFailedToInit;
	}

	if (!createOutputDefinition(*newState))
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Failed to generate new Features Definition");
		return knFailedToInit;
	}

	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	const boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);
	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}
	std::string strCacheDir = FileUtils::pathToString(pathCache);

	TRACE_INFO(getProcName() << " cache dir:" << strCacheDir);

	PYXPointer<PYXLocalStorage> storage = PYXProcessLocalStorage::create(getIdentity());

	newState->context = Context::create(newState,storage);
	newState->rootGroup = newState->context->getRootGroup();
	//m_rootGroup = GroupNode::create(m_spInputFG,storage,RangeFilter::create(m_range,m_fieldIndex));

	{
		boost::recursive_mutex::scoped_lock lock(m_stateMutex);
		m_state = newState;

		m_spGeom = m_state->inputFG->getGeometry();
		m_bWritable = m_state->inputFG->isWritable();
		m_strID = m_state->inputFG->getID();
		m_strStyle = m_state->inputFG->getStyle();
	}

	/*
	TEST CODE: use this to make sure things are working

	PYXPointer<FeatureIterator> iterator = newState->rootGroup->getIterator();

	while(!iterator->end())
	{
		boost::intrusive_ptr<IFeature> feature = iterator->getFeature();
		TRACE_INFO("feature " << feature->getID() << " value is: " << feature->getFieldValue(feature->getDefinition()->getFieldCount()-1));
		iterator->next();
	}
	*/

	return knInitialized;
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> ModifyFeaturePropertiesProcess::getIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getIterator();
}

PYXPointer<FeatureIterator> ModifyFeaturePropertiesProcess::getIterator(const PYXGeometry& geometry) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getIterator(geometry);
}


std::vector<FeatureStyle> ModifyFeaturePropertiesProcess::getFeatureStyles() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->inputFG->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> ModifyFeaturePropertiesProcess::getFeature(const std::string& strFeatureID) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return getFeatureGroupForFeature(strFeatureID)->getFeature(strFeatureID);
}

PYXPointer<const PYXTableDefinition> ModifyFeaturePropertiesProcess::getFeatureDefinition() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->outputDefinition;
}

PYXPointer<PYXTableDefinition> ModifyFeaturePropertiesProcess::getFeatureDefinition()
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->outputDefinition;
}

bool ModifyFeaturePropertiesProcess::canRasterize() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->inputFG->canRasterize();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureGroup
///////////////////////////////////////////////////////////////////////////////


Range<int> ModifyFeaturePropertiesProcess::getFeaturesCount() const 
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getFeaturesCount();
}

bool ModifyFeaturePropertiesProcess::moreDetailsAvailable() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->moreDetailsAvailable();
}

PYXPointer<PYXHistogram> ModifyFeaturePropertiesProcess::getFieldHistogram(int fieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getFieldHistogram(fieldIndex);
}

PYXPointer<PYXHistogram> ModifyFeaturePropertiesProcess::getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getFieldHistogram(geometry, fieldIndex);
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getGroupIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getGroupIterator();
}
	
PYXPointer<FeatureIterator> STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getGroupIterator(const PYXGeometry& geometry) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getGroupIterator(geometry);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getFeatureGroup(const std::string & groupId) const
{
	boost::recursive_mutex::scoped_lock lock(m_stateMutex);
	return m_state->rootGroup->getFeatureGroup(groupId);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE ModifyFeaturePropertiesProcess::getFeatureGroupForFeature(const std::string & featureId) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_state->rootGroup->getFeatureGroup(m_state->inputFG->getFeatureGroupForFeature(featureId)->getID());
}

bool ModifyFeaturePropertiesProcess::createOutputDefinition(State & state)
{
	std::set<std::string> usedNames;
	
	PYXPointer<const PYXTableDefinition> oldDefiniton = state.inputFG->getFeatureDefinition();

	state.outputDefinition = PYXTableDefinition::create(*oldDefiniton);

	for(int i=0;i<oldDefiniton->getFieldCount();++i)
	{
		usedNames.insert(oldDefiniton->getFieldDefinition(i).getName());
	}
	

	PYXPointer<Parameter> parameter = getParameter(1); 
	for(int i=0;i<parameter->getValueCount();++i)
	{
		boost::intrusive_ptr<IFeatureCalculator> calculator = 
			parameter->getValue(i)->getOutput()->QueryInterface<IFeatureCalculator>();

		if (!calculator)
		{
			return false;
		}

		state.calculators.push_back(calculator);

		PYXPointer<const PYXTableDefinition> definition = state.calculators.back()->getOutputDefinition();

		for(int i=0;i<definition->getFieldCount();++i)
		{			
			std::string name = definition->getFieldDefinition(i).getName();
			if (usedNames.find(name) != usedNames.end())
			{				
				int index = 0;
				std::string otherName = name + StringUtils::toString(index);
				while(usedNames.find(otherName) != usedNames.end())
				{
					index++;
					otherName = name + StringUtils::toString(index);
				}
				name = otherName;
			}
			usedNames.insert(name);
			const PYXFieldDefinition & fDef = definition->getFieldDefinition(i);
			state.outputDefinition->addFieldDefinition(name,fDef.getContext(), fDef.getType(), fDef.getCount());
		}
	}


	return true;
}

/*
///////////////////////////////////////////////////////////////////////////////
// FeaturesSummaryFilter::Context
///////////////////////////////////////////////////////////////////////////////

class ModifyFeaturePropertiesProcess::Context::GroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>>
{
private:
	const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & m_context;
	const PYXPointer<PYXLocalStorage> & m_storage;
	const std::string & m_id;
	const std::string & m_name;
public:
	GroupFactory(const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & context,
				 const PYXPointer<PYXLocalStorage> & storage,
				 const std::string & id,
				 const std::string & name) : m_context(context), m_storage(storage), m_id(id),m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>> & item,int & itemSize) const
	{
		boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get(m_id));
		if (dataBuffer)
		{
			GenericFeaturesGroup::GroupData data;
			(*dataBuffer) >> data;

			item = PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>::create(new GenericFeaturesGroup(m_context,data));
			itemSize = dataBuffer->size()*2;
		}
	}
};





class GeometryFactory : public PYXDataCollection::DataItemFactory<PYXTileCollection>
{
private:
	const std::string & m_id
	const std::string & m_id;
	const std::string & m_name;
public:
	GeometryFactory(const PYXPointer<PYXLocalStorage> & storage,const std::string & id,const std::string & geomName) : m_storage(storage), m_id(id),m_name(geomName)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXTileCollection> & item,int & itemSize) const
	{
		boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get(m_id+":geom"));
		if (dataBuffer)
		{
			PYXPointer<PYXGeometry> geom;
			(*dataBuffer) >> geom;

			item = boost::dynamic_pointer_cast<PYXTileCollection>(geom);
			itemSize = dataBuffer->size()*2;
		}
	}
};
/*
void ModifyFeaturePropertiesProcess::Context::mergeSubGroup(boost::intrusive_ptr<IFeatureGroup> group,GenericFeaturesGroup::GroupData & data,PYXPointer<PYXTileCollection> geom,boost::recursive_mutex & mutex) const
{
	boost::intrusive_ptr<GenericFeaturesGroup> filteredGroup = generateFilteredGroup(group);

	if (filteredGroup->getFeaturesCount().max != 0)
	{
		boost::recursive_mutex::scoped_lock lock(mutex);

		data.subGroups.push_back(GenericFeaturesGroup::ChildReference(filteredGroup->getBoundingCircle(),group->getID()));
		Range<int> groupCount = filteredGroup->getFeaturesCount();
		data.featuresCount.min += groupCount.min;
		data.featuresCount.max += groupCount.max;
		PYXPointer<PYXTileCollection> otherGeom = PYXTileCollection::create();
		filteredGroup->getGeometry()->copyTo(otherGeom.get(),geom->getCellResolution());

		geom->addGeometry(*otherGeom);
	}
}
*/
/*
boost::intrusive_ptr<GenericFeaturesGroup> ModifyFeaturePropertiesProcess::Context::generateFilteredGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup) const
{
	boost::recursive_mutex mutex;

	Range<int> featureCountEstimation = getFilter()->filter(m_inputFG);

	GenericFeaturesGroup::GroupData data;
	
	data.id = inputGroup->getID();
	data.featuresCount = 0;	

	PYXPointer<PYXTileCollection> geom = PYXTileCollection::create();
	geom->setCellResolution(inputGroup->getGeometry()->getCellResolution());

	PYXTaskGroup taskGroup;

	if (featureCountEstimation.max>0)
	{
		PYXPointer<FeatureIterator> it = inputGroup->getGroupIterator();
		
		while(!it->end())
		{
			boost::intrusive_ptr<IFeature> feature = it->getFeature();
			boost::intrusive_ptr<IFeatureGroup> group = feature->QueryInterface<IFeatureGroup>();

			if (group)
			{
				taskGroup.addTask(boost::bind(&ModifyFeaturePropertiesProcess::Context::mergeSubGroup,this,group,boost::ref(data),geom,boost::ref(mutex)));
			}
			else if (getFilter()->filter(feature).min>0)
			{
				boost::recursive_mutex::scoped_lock lock(mutex);

				data.features.push_back(GenericFeaturesGroup::ChildReference(feature->getGeometry()->getBoundingCircle(),feature->getID()));

				data.featuresCount.min++;
				data.featuresCount.max++;

				PYXPointer<PYXTileCollection> otherGeom = PYXTileCollection::create();
				feature->getGeometry()->copyTo(otherGeom.get(),geom->getCellResolution());

				geom->addGeometry(*otherGeom);
			}

			it->next();
		}
	}

	taskGroup.joinAll();

	if (data.featuresCount.max>0)
	{
		data.circle = geom->getBoundingCircle();
	}

	PYXStringWireBuffer buffer;
	buffer << *geom;

	m_storage->set(data.id+":geom",buffer);

	buffer.clear();
	buffer << data;
	m_storage->set(data.id,buffer);

	
	return new GenericFeaturesGroup(this,data);
}
*/

class ModifyFeaturePropertiesProcess::Context::InputGroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>>
{
private:
	const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & m_context;
	const std::string & m_id;
	const std::string & m_name;
public:
	InputGroupFactory(const PYXPointer<const ModifyFeaturePropertiesProcess::Context> & context,
		const std::string & id,
		const std::string & name) : m_context(context), m_id(id), m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>> & item,int & itemSize) const
	{
		item = PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>::create(m_context->m_state->inputFG->getFeatureGroup(m_id));
		assert(item->value);
		itemSize = 4;
	}
};


boost::intrusive_ptr<GenericFeaturesGroup> ModifyFeaturePropertiesProcess::Context::getRootGroup() const
{
	int version = 0;
	{
		boost::scoped_ptr<PYXWireBuffer> versionBuffer(m_storage->get("tree:version"));
		if (versionBuffer)
		{
			(*versionBuffer) >> version;
		}
	}

	if (version != knCurrentVersion )
	{
		m_storage->removeAll();

		PYXStringWireBuffer versionBuffer;
		versionBuffer << knCurrentVersion;
		m_storage->set("tree:version",versionBuffer);	
	}

	PYXPointer<GenericFeaturesGroup::GroupData> groupData = GenericFeaturesGroup::GroupData::create();
	//groupData->id = m_state->inputFG->getID();
	groupData->id = "";
	groupData->circle = m_state->inputFG->getGeometry()->getBoundingCircle();
	groupData->featuresCount = m_state->inputFG->getFeaturesCount();
	
	return new GenericFeaturesGroup(this,groupData);
}

PYXPointer<GenericFeaturesGroup::ChildrenList> ModifyFeaturePropertiesProcess::Context::getChildren(const GenericFeaturesGroup & parent) const
{	
	return getDataCollectionForGroup(parent).getItem(ChildrenFactory(m_storage,getInputGroup(parent),*m_state,parent.getID()));	
}

boost::intrusive_ptr<IFeature> ModifyFeaturePropertiesProcess::Context::getFeature(const GenericFeaturesGroup & parent, const std::string & featureId) const
{
	PYXPointer<GenericFeaturesGroup::NodeData> feature = getChildren(parent)->findFeature(featureId);

	if (feature)
	{
		boost::intrusive_ptr<IFeature> result = ExtendedFeature::deserialize(
			getInputGroup(parent)->getFeature(featureId),
			getFeatureDefinition(parent),
			feature->serializedData);

		return result;
	}

	PYXTHROW(PYXException,"Feature ID " << featureId << " wasn't found, we might need to search outside of the group children");
}

std::string ModifyFeaturePropertiesProcess::Context::getFeatureGroupIdForFeature(const std::string & featureId) const
{
	return m_state->inputFG->getFeatureGroupForFeature(featureId)->getID();
}

boost::intrusive_ptr<IFeature> ModifyFeaturePropertiesProcess::Context::getFeature(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & feature) const
{
	boost::intrusive_ptr<IFeature> result = ExtendedFeature::deserialize(
		getInputGroup(parent)->getFeature(feature->id),
		getFeatureDefinition(parent),	
		feature->serializedData);

	return result;
}

Range<int> ModifyFeaturePropertiesProcess::Context::getFeaturesCount(const GenericFeaturesGroup & parent) const
{
	return dynamic_cast<const GenericFeaturesGroup::GroupData*>(&getDataForGroup(parent))->featuresCount;
}

boost::intrusive_ptr<GenericFeaturesGroup> ModifyFeaturePropertiesProcess::Context::getGroup(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & groupData) const
{	
	PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>> groupItem = getDataCollectionForGroup(parent).getItem(GroupFactory(this,groupData,"g:"+groupData->id));
	return groupItem->value;
}

PYXPointer<PYXTableDefinition> ModifyFeaturePropertiesProcess::Context::getFeatureDefinition(const GenericFeaturesGroup & parent) const
{
	return m_state->outputDefinition;
}

PYXValue ModifyFeaturePropertiesProcess::Context::getFieldValue(const GenericFeaturesGroup & parent,int index) const
{
	return PYXValue();
}


PYXPointer<PYXGeometry> ModifyFeaturePropertiesProcess::Context::getGeometry(const GenericFeaturesGroup & parent) const
{
	return getInputGroup(parent)->getGeometry();
}

std::string ModifyFeaturePropertiesProcess::Context::getStyle(const GenericFeaturesGroup & parent) const
{
	return getInputGroup(parent)->getStyle();
}

boost::intrusive_ptr<IFeatureGroup> ModifyFeaturePropertiesProcess::Context::getInputGroup(const GenericFeaturesGroup & parent) const
{
	return getDataCollectionForGroup(parent).getItem(InputGroupFactory(this,parent.getID(),"i"))->value;
}

PYXPointer<PYXHistogram> ModifyFeaturePropertiesProcess::Context::getFieldHistogram(const GenericFeaturesGroup & parent,const PYXGeometry & geometry, int nFieldIndex) const
{
	if (nFieldIndex < m_state->inputFG->getFeatureDefinition()->getFieldCount())
	{
		return getInputGroup(parent)->getFieldHistogram(geometry,nFieldIndex);
	}
	return GenericFeaturesGroup::ContextWithHistograms::getFieldHistogram(parent,geometry,nFieldIndex);
}

PYXPointer<PYXHistogram> ModifyFeaturePropertiesProcess::Context::getFieldHistogram(const GenericFeaturesGroup & parent,int nFieldIndex) const
{
	if (nFieldIndex < m_state->inputFG->getFeatureDefinition()->getFieldCount())
	{
		return getInputGroup(parent)->getFieldHistogram(nFieldIndex);
	}
	return GenericFeaturesGroup::ContextWithHistograms::getFieldHistogram(parent,nFieldIndex);
}