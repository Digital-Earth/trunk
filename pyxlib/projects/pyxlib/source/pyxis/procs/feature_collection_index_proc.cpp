/******************************************************************************
feature_collection_index_proc.cpp

begin		: 2013-05-28
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/feature_collection_index_proc.h"
#include "pyxis/data/feature_iterator_linq.h"
#include "pyxis/pipe/pipe_utils.h"

// pyxlib includes
#include "pyxis/utility/thread_pool.h"
#include "pyxis/utility/tester.h"

#include "boost/algorithm/string.hpp"


// store the 10 most populate values
class SuggestionList
{
private:
	static const int s_maxSuggestions = 10;

	std::map<std::string,int> m_Suggestions;
	int m_minCount;

public:
	SuggestionList() : m_minCount(0)
	{
	}

	void insert(const std::string & value,int count)
	{
		if (count <= m_minCount)
		{
			return;
		}

		auto it = m_Suggestions.find(value);
		if (it != m_Suggestions.end() && count > it->second)
		{
			it->second = count;
		}
		else //we are adding new item
		{
			m_Suggestions[value] = count;			
		
			//keep the best 10
			if (m_Suggestions.size() > s_maxSuggestions)
			{
				auto item = std::min_element(m_Suggestions.begin(),m_Suggestions.end(),
											[](const std::pair<std::string,int> & a,const std::pair<std::string,int> & b){ return a.second < b.second; });
				m_Suggestions.erase(item);

				item = std::min_element(m_Suggestions.begin(),m_Suggestions.end(),
											[](const std::pair<std::string,int> & a,const std::pair<std::string,int> & b){ return a.second < b.second; });

				m_minCount = item->second;
			}
		}
	}

	std::vector<std::string> toVector() const
	{
		//order items by count..
		std::vector<std::pair<std::string,int>> items(m_Suggestions.begin(),m_Suggestions.end());
		std::sort(items.begin(),items.end(),[](const std::pair<std::string,int> & a,const std::pair<std::string,int> & b) { return a.second > b.second; });

		//get only the values
		std::vector<std::string> result;
		for(auto & item : items)
		{
			result.push_back(item.first);
		}
		return result;
	}
};

//store list of features
class FeaturesIds : public PYXObject
{
private:
	std::vector<std::string> m_ids;

public:
	unsigned int size() const { return m_ids.size(); }
	const std::vector<std::string> & ids() { return m_ids; }
	void add(const std::string & id) { m_ids.push_back(id); }

public:
	FeaturesIds() {}

	static PYXPointer<FeaturesIds> create()
	{
		return PYXNEW(FeaturesIds);
	}

private:
	friend PYXWireBuffer & operator <<(PYXWireBuffer & buffer, const FeaturesIds & ids);
	friend PYXWireBuffer & operator >>(PYXWireBuffer & buffer, FeaturesIds & ids);
};

PYXWireBuffer & operator <<(PYXWireBuffer & buffer, const FeaturesIds & ids)
{
	buffer << ids.m_ids;
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer, FeaturesIds & ids)
{
	buffer >> ids.m_ids;
	return buffer;
}

//hold completion information for a specific prefix in the ConstIndex
class ConstIndexNode : public PYXObject
{	
private:
	unsigned int m_totalSize;
	unsigned int m_nodeSize;
	std::string m_children;
	std::vector<std::string> m_completions;

public:
	ConstIndexNode() : m_totalSize(0),m_nodeSize(0)
	{
	}

	ConstIndexNode(const FeaturesIds & ids,const std::string & children,int totalSize,const std::vector<std::string> completions) 
		: m_totalSize(totalSize),m_nodeSize(ids.size()), m_children(children), m_completions(completions)
	{		
	}

	static PYXPointer<ConstIndexNode> create()
	{
		return PYXNEW(ConstIndexNode);
	}

	unsigned int getTotalSize() const { return m_totalSize; };
	unsigned int getNodeSize() const { return m_nodeSize; };
	bool hasChild(char child) const { return m_children.find(child) != m_children.npos; };

	void addSuggestions(const std::string & prefix,std::vector<std::string> & Suggestions)
	{
		for(auto & completion : m_completions)
		{
			Suggestions.push_back(prefix + completion);
		}
	}

public:
	friend PYXWireBuffer & operator <<(PYXWireBuffer & buffer, const ConstIndexNode & node);
	friend PYXWireBuffer & operator >>(PYXWireBuffer & buffer, ConstIndexNode & node);
};

PYXWireBuffer & operator <<(PYXWireBuffer & buffer, const ConstIndexNode & node)
{
	buffer << node.m_totalSize << node.m_nodeSize << node.m_children << node.m_completions;
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer, ConstIndexNode & node)
{
	buffer >> node.m_totalSize >> node.m_nodeSize >> node.m_children >> node.m_completions;
	return buffer;
}


class IndexKeyFactory
{
private:
	static std::string encodeValue(const std::string & value)
	{
		if (value.size() <= 5)
		{
			return value;
		} 
		else 
		{
			auto size = value.size() - 1;
			auto pos = size - size % 5;
			return value.substr(0,pos) + ":" + value.substr(pos);
		}
	}


public:
	static std::string getIds(const std::string & value)
	{
		return "ids:"+encodeValue(value);
	}

	static std::string getNode(const std::string & value)
	{
		return "node:"+encodeValue(value);
	}
};

//provide a reader for persistence reader.
class ConstIndex
{
private:
	PYXPointer<PYXLocalStorage> m_storage;

public:
	PYXPointer<FeaturesIds> ids(const std::string & value)
	{
		auto buffer = m_storage->get(IndexKeyFactory::getIds(value));

		if (buffer.get()==NULL)
		{
			return FeaturesIds::create();
		}

		PYXPointer<FeaturesIds> ids = FeaturesIds::create();
		*buffer >> *ids;
		return ids;		
	}

	std::vector<std::string> suggest(const std::string & value)
	{
		std::vector<std::string> result;
		auto buffer = m_storage->get(IndexKeyFactory::getNode(value));

		if (buffer.get()!=NULL)
		{		
			PYXPointer<ConstIndexNode> node = ConstIndexNode::create();
			*buffer >> *node;
			if (node->getNodeSize()>0)
			{
				result.push_back(value);
			}
			node->addSuggestions(value,result);
		}

		return result;		
	}

public:
	ConstIndex (const PYXPointer<PYXLocalStorage> storage) : m_storage(storage)
	{
	}
};

//a node index used to generate the index
class IndexNode : public PYXObject
{
	int m_totalSize;
	FeaturesIds m_ids;
	SuggestionList m_popularValues;
	std::map<char,PYXPointer<IndexNode>> m_children;

public:
	IndexNode() : m_totalSize(0)
	{
	}

public:
	PYXPointer<ConstIndexNode> toConstIndexNode()
	{
		std::string children;
		for(auto item : m_children)
		{
			children += item.first;
		}		
		return PYXNEW(ConstIndexNode,m_ids,children,m_totalSize,m_popularValues.toVector());
	}

public:
	unsigned int add(const std::string & value,const std::string & id)
	{
		m_totalSize++;
		if (value.empty())
		{			
			m_ids.add(id);
			return m_ids.size();
		}

		char begin = value[0];
		auto it = m_children.find(begin );
		if (it == m_children.end())
		{
			m_children[begin] = PYXNEW(IndexNode);
			it = m_children.find(begin );
		}

		unsigned int valueCount = it->second->add(value.substr(1,value.size()-1),id);

		m_popularValues.insert(value,valueCount);
		
		return valueCount;
	}

public:
	void save(const std::string & prefix,const PYXPointer<PYXLocalStorage> & storage)
	{
		{
			PYXPointer<ConstIndexNode> node = toConstIndexNode();
			PYXStringWireBuffer buffer;
			buffer << *node;
			storage->set(IndexKeyFactory::getNode(prefix),buffer);
		}

		if (m_ids.size()>0)
		{
			PYXStringWireBuffer buffer;
			buffer << m_ids;
			storage->set(IndexKeyFactory::getIds(prefix),buffer);
		}
		for(auto child : m_children)
		{
			 child.second->save(prefix+child.first,storage);
		}
	}
};



// {AFE6F82A-8E82-41CA-9764-B45DA5264D76}
PYXCOM_DEFINE_CLSID(FeatureCollectionIndexProcess, 
0xafe6f82a, 0x8e82, 0x41ca, 0x97, 0x64, 0xb4, 0x5d, 0xa5, 0x26, 0x4d, 0x76);

PYXCOM_CLASS_INTERFACES(FeatureCollectionIndexProcess, IFeatureCollectionIndex::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionIndexProcess, "Index a Feature Collection", "A process that Index a Feature Collection.", "Unknown",
							IFeatureCollectionIndex::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Feature Collection", "A feature collection.")
IPROCESS_SPEC_END

namespace
{
	//! The unit test class.
	Tester<FeatureCollectionIndexProcess> gTester;
}

//! Unit test.
void FeatureCollectionIndexProcess::test()
{
	//TODO: this
}


FeatureCollectionIndexProcess::FeatureCollectionIndexProcess()
{
}

//convenience constuctores
FeatureCollectionIndexProcess::FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::string & fieldName)
{
	getParameter(0)->addValue(inputProcess);
	boost::intrusive_ptr<IFeatureCollection> fc = inputProcess->getOutput()->QueryInterface<IFeatureCollection>();
	m_fieldIndices.push_back(fc->getFeatureDefinition()->getFieldIndex(fieldName));
}

FeatureCollectionIndexProcess::FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::vector<std::string> & fieldsName)
{
	getParameter(0)->addValue(inputProcess);
	boost::intrusive_ptr<IFeatureCollection> fc = inputProcess->getOutput()->QueryInterface<IFeatureCollection>();
	for(auto & field : fieldsName)
	{
		m_fieldIndices.push_back(fc->getFeatureDefinition()->getFieldIndex(field));
	}
}

FeatureCollectionIndexProcess::FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, int fieldIndex)
{
	getParameter(0)->addValue(inputProcess);
	m_fieldIndices.push_back(fieldIndex);
}

FeatureCollectionIndexProcess::FeatureCollectionIndexProcess(boost::intrusive_ptr<IProcess> inputProcess, const std::vector<int> & fieldsIndices)
	: m_fieldIndices(fieldsIndices)
{
	getParameter(0)->addValue(inputProcess);	
}


std::string STDMETHODCALLTYPE FeatureCollectionIndexProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"FeatureCollectionIndexProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"

			   "<xs:element name=\"FieldsIndices\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Fields Indices</friendlyName>"
						"<description>List of Fields Indices</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureCollectionIndexProcess::getAttributes() const
{
	std::string fields;

	for(auto & fieldIndex : m_fieldIndices)
	{
		fields += StringUtils::toString(fieldIndex) + " ";
	}
	fields = StringUtils::trim(fields);

	std::map<std::string, std::string> mapAttr;

	mapAttr["FieldsIndices"] = fields;
	
	return mapAttr;
}


void STDMETHODCALLTYPE FeatureCollectionIndexProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;

	std::string fields;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FieldsIndices",fields);

	m_fieldIndices.clear();

	std::stringstream fieldsStream(fields);

	while(fieldsStream.good())
	{
		int field = -1;
		fieldsStream >> field;
		
		if (field>=0)
		{
			m_fieldIndices.push_back(field);
		}
	}
}


IProcess::eInitStatus FeatureCollectionIndexProcess::initImpl()
{
	m_inputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();

	if (!m_inputFC)
	{
		setInitProcError<InputInitError>("input is not IFeatureCollection");
		return knFailedToInit;
	}

	//don't init the pipeline line before we have a stable identity
	PipeUtils::waitUntilPipelineIdentityStable(this);	
	m_storage = PYXProcessLocalStorage::create(getIdentity());

	const int neededVersion = 2;
	int version = 0;

	auto versionBuffer = m_storage->get("index:version");	
	if (versionBuffer.get()!=0)
	{
		*versionBuffer >> version;
	}
	if (version != neededVersion)
	{
		m_storage->removeAll();

		try
		{
			initIndex();
		}
		catch(...)
		{
			setInitProcError<InputInitError>("Failed to generate index");
			return knFailedToInit;
		}

		{
			PYXStringWireBuffer buffer;		
			buffer << neededVersion ;
			m_storage->set("index:version",buffer);
		}
	}


	return knInitialized;
}

void FeatureCollectionIndexProcess::initIndex()
{
	IndexNode index;
	
	if (m_fieldIndices.empty())
	{
		//there is no index to do
		return;
	}

	PYXPointer<FeatureIterator> iterator = m_inputFC->getIterator();
	std::string str;
	std::string id;
	while(!iterator->end())
	{
		boost::intrusive_ptr<IFeature> feature = iterator->getFeature();		
		id = feature->getID();

		//collect all unique words..
		std::set<std::string> values;
		for(auto field : m_fieldIndices)
		{
			str = StringUtils::trim(feature->getFieldValue(field).getString());

			if (str.empty()) 
			{
				continue;
			}

			boost::algorithm::to_lower(str);

			if (str.find(' ') != str.npos)
			{
				//if we found ' ' in the string - break into words and sanitize the string from helper symbols.
				boost::algorithm::split(values,str,boost::is_any_of(" .,-()[]{}!?:;'\""), boost::algorithm::token_compress_on);			
			}
			else
			{
				values.insert(str);
			}
		}

		//add the feature to all found unique words
		for(auto & value : values)
		{
			if (value.empty())
			{
				continue;
			}

			index.add(value,id);
		}

		iterator->next();
	}
	
	index.save("",PYXBufferedLocalStorage::create(m_storage));
}

class IndexedFeatureIterator : public FeatureIterator
{
private:
	boost::intrusive_ptr<IFeatureCollection> m_fc;
	FeatureCollectionIndexProcess::FeaturesIDList m_featuresId;
	FeatureCollectionIndexProcess::FeaturesIDList::iterator m_current;	

public:
	IndexedFeatureIterator()
	{
		m_current = m_featuresId.end();
	}

	IndexedFeatureIterator(const boost::intrusive_ptr<IFeatureCollection> & fc, const FeatureCollectionIndexProcess::FeaturesIDList & ids)
		: m_fc(fc), m_featuresId(ids)
	{
		m_current = m_featuresId.begin();
	}

	static PYXPointer<IndexedFeatureIterator> create()
	{
		return PYXNEW(IndexedFeatureIterator);
	}

	static PYXPointer<IndexedFeatureIterator> create(const boost::intrusive_ptr<IFeatureCollection> & fc, const FeatureCollectionIndexProcess::FeaturesIDList & ids)
	{
		return PYXNEW(IndexedFeatureIterator,fc,ids);
	}

public:
	
	virtual bool end() const 
	{
		return m_current == m_featuresId.end();
	}

	virtual void next()
	{
		++m_current;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_fc->getFeature(*m_current);
	}
};

FeatureCollectionIndexProcess::FeaturesIDList FeatureCollectionIndexProcess::findMatchingFeatures(const std::string & words) const
{
	std::set<std::string> splitWords;

	//split and lower case the input string
	boost::algorithm::split(splitWords,boost::algorithm::to_lower_copy(words),boost::is_any_of(" "),boost::algorithm::token_compress_on);

	return findMatchingFeatures(splitWords);
}

FeatureCollectionIndexProcess::FeaturesIDList FeatureCollectionIndexProcess::findMatchingFeatures(const std::set<std::string> & words) const
{
	std::map<std::string,int> idsCount;
	
	int maxCount = 1;

	ConstIndex index(m_storage);

	//step 1: search each word in the index
	for(auto & word : words)
	{
		//++ for each id that we found		
		auto ids = index.ids(word);
		for(auto & id : ids->ids())
		{
			auto it = idsCount.find(id);
			if (it == idsCount.end())
			{
				idsCount[id]=1;
			}
			else
			{
				it->second++;
				if (it->second > maxCount)
				{
					maxCount = it->second;
				}
			}
		}
	}

	//step 2: return only features with count = maxCount
	FeaturesIDList result;
	for(auto & word : idsCount)
	{
		if (word.second == maxCount)
		{
			result.push_back(word.first);
		}
	}

	return result;
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeatureCollectionIndexProcess::getIterator(const PYXValue & value) const
{
	return IndexedFeatureIterator::create(m_inputFC,findMatchingFeatures(value.getString()));
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeatureCollectionIndexProcess::getIterator(const PYXGeometry& geometry,const PYXValue & value) const
{
	return PYXFeatureIteratorLinq(getIterator(value)).filter(geometry);
}

std::vector<PYXValue> STDMETHODCALLTYPE FeatureCollectionIndexProcess::suggest(const PYXValue & value) const
{
	//split input value into list of words
	std::vector<std::string> words;
	
	boost::algorithm::split(words,boost::algorithm::to_lower_copy(value.getString()),boost::is_any_of(" "),boost::algorithm::token_compress_on);

	//sometimes the last token is empty string.
	if (words.size()>1 && words.back().empty())
	{
		words.pop_back();
	}

	//get Suggestsions for the last word
	ConstIndex index(m_storage);
	auto suggestions = index.suggest(words.back());
	
	words.pop_back();

	std::string prefix = "";
	for(auto & word : words)
	{
		prefix += word + " ";
	}

	//create result
	std::vector<PYXValue> result;

	for(auto & suggestion : suggestions)
	{
		result.push_back(PYXValue(prefix + suggestion));
	}

	return result;
}