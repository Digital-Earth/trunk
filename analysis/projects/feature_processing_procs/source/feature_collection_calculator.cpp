/******************************************************************************
feature_collection_calculator.cpp

begin      : 07/04/2008 8:42:06 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

// local includes
#include "feature_collection_calculator.h"
#include "feature_collection_process.h"

// {FED1DFF5-34C4-4a6e-94F1-02BEF327EE8D}
PYXCOM_DEFINE_CLSID(FeatureCollectionCalculator, 
0xfed1dff5, 0x34c4, 0x4a6e, 0x94, 0xf1, 0x2, 0xbe, 0xf3, 0x27, 0xee, 0x8d);

PYXCOM_CLASS_INTERFACES(FeatureCollectionCalculator, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionCalculator, "Feature Collection Calculator", "Add the result of a calculation to each feature in the calculation.", "Development/Broken",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to perform a calculation on.")
	IPROCESS_SPEC_PARAMETER(IFeatureCalculator::iid, 1, 1, "Input Calculation", "The calculation to perform on every feature in the collection.")
IPROCESS_SPEC_END

//! Constructor
FeatureCollectionCalculator::FeatureCollectionCalculator() 
{
}

//! Destructor
FeatureCollectionCalculator::~FeatureCollectionCalculator()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus FeatureCollectionCalculator::initImpl()
{
	// Set the ID.
	m_strID = "FeatureCollectionCalculator " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the feature collection process.
	m_spFeaturesInput = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	assert(m_spFeaturesInput);

	// acquire the calculator input
	m_spCalculator = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeatureCalculator>();
	assert(m_spCalculator);

	// examine the input and verify that the specified field name is unique
	if (m_strCalculatedFieldName == "")
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("The attribute 'field_name' can not be null.");
		return knFailedToInit;
	}

	m_spFeatureDefn = m_spFeaturesInput->getFeatureDefinition()->clone();
	std::vector <std::string> names = m_spFeatureDefn->getFieldNames();
	if (std::find(names.begin(), names.end(), m_strCalculatedFieldName) != names.end())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Calculated feature field name must be unique: " + 
			m_strCalculatedFieldName);
		return knFailedToInit;
	}

	// add the new field
	m_spFeatureDefn->addFieldDefinition(
		m_strCalculatedFieldName, 
		PYXFieldDefinition::knContextNone, 
		m_spCalculator->getOutputDefinition()->getFieldDefinition(0).getType());

	return knInitialized;
}

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string FeatureCollectionCalculator::getAttributeSchema() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"FeatureCollectionCalculator\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"field_name\" type=\"xs:string\" nillable=\"false\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Field Name</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> FeatureCollectionCalculator::getAttributes() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::map<std::string, std::string> mapAttr;	
	mapAttr["field_name"] = m_strCalculatedFieldName;
	return mapAttr;
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE FeatureCollectionCalculator::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it = mapAttr.find("field_name");
	if (it != mapAttr.end())
	{
		m_strCalculatedFieldName = it->second;
	}
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> FeatureCollectionCalculator::getIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return CalculatedFeatureIterator::create(this, m_spFeaturesInput->getIterator());
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> FeatureCollectionCalculator::getIterator(const PYXGeometry& geometry) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return CalculatedFeatureIterator::create(this, m_spFeaturesInput->getIterator(geometry));
}

//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> FeatureCollectionCalculator::getFeatureStyles() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->getFeatureStyles();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> FeatureCollectionCalculator::getFeature(const std::string& strFeatureID) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	boost::intrusive_ptr<IFeature> spInputFeature = m_spFeaturesInput->getFeature(strFeatureID);
	return createOutputFeature(spInputFeature);
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> FeatureCollectionCalculator::getFeatureDefinition() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeatureDefn;
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> FeatureCollectionCalculator::getFeatureDefinition()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeatureDefn;
}

bool FeatureCollectionCalculator::canRasterize() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_spFeaturesInput->canRasterize();
}

// FeatureCollectionCalculator

/*!
Take an input feature, perform the input calculation and append the result to a feature
that is then returned to the user.

\param spInputFeature	The feature from the input feature collection to perform a calculation on.
						This value can be null.

\return The newly created output feature with the appended data.
*/
boost::intrusive_ptr<IFeature> FeatureCollectionCalculator::createOutputFeature(
	boost::intrusive_ptr<IFeature> spInputFeature) const
{
	if (spInputFeature)
	{
		// perform the calculation on the input feature
		PYXValue value = m_spCalculator->calculateValue(spInputFeature,0);

		// create the new feature with the calculated value
		return new CalculatedFeature(getFeatureDefinition(), spInputFeature, m_strCalculatedFieldName, value);
	}

	// return the null pointer
	return spInputFeature;
}

