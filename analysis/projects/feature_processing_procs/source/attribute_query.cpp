/******************************************************************************
attribute_query.cpp

begin		: 2008-01-31
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "attribute_query.h"

#include "feature_collection_process.h"
#include "pyxis/utility/xml_transform.h"

// Required by tests
#include "pyxis/geometry/multi_geometry.h"
#include "pyxis/procs/default_feature.h"

// {5CA647A5-DA16-4a30-A6AC-0CA6369035FD}
PYXCOM_DEFINE_CLSID(AttributeQuery, 
0x5ca647a5, 0xda16, 0x4a30, 0xa6, 0xac, 0xc, 0xa6, 0x36, 0x90, 0x35, 0xfd);

PYXCOM_CLASS_INTERFACES(AttributeQuery, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(AttributeQuery, "Attribute Query", "A feature collection comprised of features that satisfy the query.", "Analysis/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to execute the query on.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<AttributeQuery> gTester;

}

//! Complex Queries will run with the .NET XPath query engine.
class XPathQuery : public AttributeQuery::FeatureQuery
{
private:
	std::string m_strQuery;

public:
	static PYXPointer<AttributeQuery::FeatureQuery> create(const std::string& strQuery)
	{
		return PYXNEW(XPathQuery,strQuery);
	};

	XPathQuery(const std::string& strQuery) : m_strQuery(strQuery)
	{
	};

	virtual bool match(const boost::intrusive_ptr<IFeature> & spFeature) const
	{
		return CSharpFunctionProvider::getCSharpFunctionProvider()->doesXPathMatch( 
			m_strQuery, RecordTools::getFieldsAsXml(*spFeature.get()));
	};
};

//! Simple queries "field=value" will run with a fast and simple implementation
class SimpleQuery : public AttributeQuery::FeatureQuery
{
private:
	std::string m_strFieldName;
	std::string m_strFieldValue;
public:
	static PYXPointer<AttributeQuery::FeatureQuery> create(const std::string& strQuery)
	{
		//try to parse the followling string: "//field[name='field' and value='value']"
		if (strQuery.substr(0,14) != "//field[name='")
		{
			return PYXPointer<SimpleQuery>();
		}

		//find second "'"
		size_t pos = strQuery.find("'",14);

		if (pos == std::string::npos)
		{
			return PYXPointer<SimpleQuery>();
		}
		
		std::string fieldName = strQuery.substr(14,pos-14);

		if (strQuery.substr(pos,13) != "' and value='")
		{
			return PYXPointer<SimpleQuery>();
		}

		pos += 13;

		size_t endPos = strQuery.find("'",pos);

		if (endPos == std::string::npos)
		{
			return PYXPointer<SimpleQuery>();
		}

		std::string fieldValue = strQuery.substr(pos,endPos-pos);

		if (strQuery.substr(endPos) != "']")
		{
			return PYXPointer<SimpleQuery>();
		}

		//everything was parsed ok.
		return PYXNEW(SimpleQuery,fieldName,fieldValue);
	};

	SimpleQuery(const std::string & field,const std::string & value) : m_strFieldName(field), m_strFieldValue(value)
	{
	};

	virtual bool match(const boost::intrusive_ptr<IFeature> & spFeature) const
	{
		return spFeature->getFieldValueByName(m_strFieldName).getString() == m_strFieldValue;
	};
};

//! Constructor
AttributeQuery::AttributeQuery() :
	m_bInputGeometry(true)
{
}

//! Destructor
AttributeQuery::~AttributeQuery()
{
}

void AttributeQuery::test()
{
	// Query string.
	// TODO: put a real xpath query in here.
	const std::string strTestQuery = "This is a test.";

	// Create the process.
	boost::intrusive_ptr<IProcess> spProcess;
	PYXCOMCreateInstance(AttributeQuery::clsid, 0, IProcess::iid, (void**)&spProcess);
	assert(spProcess);

	// Set the attribute.
	{
		std::map<std::string, std::string> mapAttr;
		mapAttr["query"] = strTestQuery;
		spProcess->setAttributes(mapAttr);
	}

	// Get the attribute and verify.
	{
		std::map<std::string, std::string> mapAttr = spProcess->getAttributes();
		TEST_ASSERT(strTestQuery == mapAttr["query"]);
	}

	// Create data feature collection.
	// TODO: Make each of these contain different data relevant to testing.
	boost::intrusive_ptr<DefaultFeatureCollection> spFC(new DefaultFeatureCollection);
	boost::intrusive_ptr<IFeature> spFeature1(new DefaultFeature);
	boost::intrusive_ptr<IFeature> spFeature2(new DefaultFeature);
	boost::intrusive_ptr<IFeature> spFeature3(new DefaultFeature);
	boost::intrusive_ptr<IFeature> spFeature4(new DefaultFeature);
	spFC->addFeature(spFeature1);
	spFC->addFeature(spFeature2);
	spFC->addFeature(spFeature3);
	spFC->addFeature(spFeature4);

	// Set the feature collection parameter.
	{
		// Create the parameter.
		PYXPointer<Parameter> spParameter = spProcess->getParameter(0);

		// Create the parameter process.
		boost::intrusive_ptr<IProcess> spDataProcess;
		spFC->QueryInterface(IProcess::iid, (void**)&spDataProcess);

		// Add the process to the parameter.
		spParameter->addValue(spDataProcess);
	}

	// Execute the process.
	spProcess->initProc();

	// TODO: Once the "match" function is implemented, ensure that only those that match are returned.
	{
		//// Get the output.
		//boost::intrusive_ptr<PYXCOM_IUnknown> spOutput = spProcess->getOutput();
		//assert(spOutput);

		//// Cast it.
		//boost::intrusive_ptr<IFeatureCollection> spFCOutput;
		//spOutput->QueryInterface(IFeatureCollection::iid, (void**)&spFCOutput);
		//assert(spFCOutput);

		//// Check it.
		//PYXPointer<FeatureIterator> spIt = spFCOutput->getIterator();
		//TEST_ASSERT(!spIt->end());
		//TEST_ASSERT(spIt->getFeature().get() == spFeature1.get());
		//spIt->next();
		//TEST_ASSERT(!spIt->end());
		//TEST_ASSERT(spIt->getFeature().get() == spFeature2.get());
		//spIt->next();
		//TEST_ASSERT(!spIt->end());
		//TEST_ASSERT(spIt->getFeature().get() == spFeature3.get());
		//spIt->next();
		//TEST_ASSERT(!spIt->end());
		//TEST_ASSERT(spIt->getFeature().get() == spFeature4.get());
		//spIt->next();
		//TEST_ASSERT(spIt->end());
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus AttributeQuery::initImpl()
{
	// Set the ID.
	m_strID = "AttributeQuery" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the feature collection process.
	m_spFeaturesInput = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	if (!m_spFeaturesInput)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not retrieve the input feature collection to filter.");
		return knFailedToInit;
	}

	m_query = createQuery(m_strQuery);

	return knInitialized;
}

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string AttributeQuery::getAttributeSchema() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::string strSchema = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">";
	
	// add the type for the dropdown
	strSchema +=
	"<xs:simpleType name=\"geometryType\"><xs:restriction base=\"xs:string\">" 
	"<xs:enumeration value=\"use_input_geometry\" />" 
	"<xs:enumeration value=\"use_result_geometry\" />" 
	"</xs:restriction></xs:simpleType>";

	strSchema += 
		"<xs:element name=\"AttributeQuery\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"geometry\" type=\"geometryType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Geometry</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"query\" type=\"xs:string\" nillable=\"false\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Query</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strSchema;
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> AttributeQuery::getAttributes() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::map<std::string, std::string> mapAttr;	
	mapAttr["query"] = m_strQuery;
	
	if (m_bInputGeometry)
	{
		mapAttr["geometry"] = "use_input_geometry";
	}
	else
	{
		mapAttr["geometry"] = "use_result_geometry";
	}
	return mapAttr;
}

PYXPointer<AttributeQuery::FeatureQuery> AttributeQuery::createQuery(const std::string& strQuery)
{
	PYXPointer<FeatureQuery> query = SimpleQuery::create(strQuery);

	if (query)
	{
		return query;
	}
	else
	{
		return XPathQuery::create(strQuery);
	}
}


/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE AttributeQuery::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::map<std::string, std::string>::const_iterator it = mapAttr.find("query");
	if (it != mapAttr.end())
	{
		m_strQuery = it->second;
	}
	it = mapAttr.find("geometry");
	if (it != mapAttr.end() && (it->second == "use_input_geometry"))
	{
		m_bInputGeometry = true;
	}
	else
	{
		m_bInputGeometry = false;
	}
	m_spGeom = 0;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> AttributeQuery::getIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// the XPath query not being specified equates to no filtering
	if(m_strQuery == "")
	{
		return m_spFeaturesInput->getIterator();
	}
	else
	{
		return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(), m_query);
	}
}

PYXPointer<FeatureIterator> AttributeQuery::getIterator(const PYXGeometry& geometry) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// the XPath query not being specified equates to no filtering
	if(m_strQuery == "")
	{
		return m_spFeaturesInput->getIterator(geometry);
	}
	else
	{
		return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(geometry), m_query);
	}
}

std::vector<FeatureStyle> AttributeQuery::getFeatureStyles() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> AttributeQuery::getFeature(const std::string& strFeatureID) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	boost::intrusive_ptr<IFeature> spPossibleFeature = m_spFeaturesInput->getFeature(strFeatureID);
	if (spPossibleFeature && m_query->match(spPossibleFeature))
	{
		return spPossibleFeature;
	}
	return boost::intrusive_ptr<IFeature>();
}

PYXPointer<const PYXTableDefinition> AttributeQuery::getFeatureDefinition() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> AttributeQuery::getFeatureDefinition()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->getFeatureDefinition();
}

bool AttributeQuery::canRasterize() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->canRasterize();
}

PYXPointer<PYXGeometry> AttributeQuery::getGeometry()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (m_bInputGeometry)
	{
		return m_spFeaturesInput->getGeometry();	
	}
	else
	{
		// should we be returning the exact geometry?
		// if so, this would need to be repeated in the const version
		if (!m_spGeom)
		{
			PYXPointer<PYXMultiGeometry<PYXGeometry> > aggGeom = PYXMultiGeometry<PYXGeometry>::create();

			// get the features at the user specified geometry resolution
			PYXPointer<FeatureIterator> it = getIterator();
			while (!it->end())
			{
				aggGeom->addGeometry(it->getFeature()->getGeometry());
				it->next();
			}

			m_spGeom = aggGeom;
		}
		return m_spGeom;
	}
}
