/******************************************************************************
pyxilization_feature.cpp

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/


#include "pyxilization_feature.h"
#include "tester.h"

TesterUnit<PyxilizationFeature> pyxilizationFeatureTest;

void PyxilizationFeature::test()
{
	boost::shared_ptr<PYXCell> spCell(new PYXCell(PYXIcosIndex("A-010203")));
	boost::shared_ptr<PyxilizationFeature> spFeature(new PyxilizationFeature(spCell));

	assert(spFeature != 0);

	//test all getting and setting
	spFeature->setFieldValue(PYXValue((uint8_t)123), PyxilizationFeature::eFieldHealth);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() == 123 );

	spFeature->setFieldValue(PYXValue((uint8_t) 122), PyxilizationFeature::eFieldPlayerID);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == 122 );

	spFeature->setFieldValue(PYXValue((uint8_t) 121), PyxilizationFeature::eFieldMovesLeft);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() == 121 );

	spFeature->setFieldValue(PYXValue((uint8_t) 120), PyxilizationFeature::eFieldLoaded);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() == 120 );

	spFeature->setFieldValue(PYXValue((uint8_t) 119), PyxilizationFeature::eFieldFuel);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8() == 119 );

	spFeature->setFieldValue(PYXValue((uint8_t) PyxilizationFeature::eTypeUnit), PyxilizationFeature::eFieldType);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeUnit );

	spFeature->setFieldValue(PYXValue((uint8_t) PyxilizationFeature::eTypeUnit), PyxilizationFeature::eFieldBuildType);
	assert( spFeature->getFieldValue(PyxilizationFeature::eFieldBuildType).getUInt8() == PyxilizationFeature::eTypeUnit );
}

PyxilizationFeature::PyxilizationFeature(boost::shared_ptr<PYXGeometry> spGeometry)
{
	static int m_nLastID = 0;
	//boost::shared_ptr<PYXCell> spCell = boost::dynamic_pointer_cast<PYXCell, PYXGeometry>(spGeometry);
	m_nID = m_nLastID;
	m_nLastID++;
	m_spGeometry = spGeometry;

	char szTemp[64];
	_itoa_s(m_nID, szTemp, 64, 10);
	m_strID = szTemp;

	//set up field definitions
	PYXFieldDefinition typefield("type", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition healthfield("health", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition movesleftfield("movesleft", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition playeridfield("playerid", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition loadedfield("loaded", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition fuelfield("fuel", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXFieldDefinition buildtypefield("buildtype", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXTableDefinition::SPtr table = PYXTableDefinition::create();
	table->addFieldDefinition(typefield);
	table->addFieldDefinition(healthfield);
	table->addFieldDefinition(movesleftfield);
	table->addFieldDefinition(playeridfield);
	table->addFieldDefinition(loadedfield);
	table->addFieldDefinition(fuelfield);
	table->addFieldDefinition(buildtypefield);
	setDefinition(table);
}


void PyxilizationFeature::setFieldValue(PYXValue value, int nFieldIndex)
{
	if(nFieldIndex == PyxilizationFeature::eFieldHealth)
	{
		m_nHealth = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldType)
	{
		m_nType = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldMovesLeft)
	{
		m_nMovesLeft = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldPlayerID)
	{
		m_nPlayerID = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldLoaded)
	{
		m_nLoaded = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldFuel)
	{
		m_nFuel = value.getUInt8();
	}
	if(nFieldIndex == PyxilizationFeature::eFieldBuildType)
	{
		m_nBuildType = value.getUInt8();
	}
}

PYXValue PyxilizationFeature::getFieldValue(int nFieldIndex) const
{
	if(nFieldIndex==PyxilizationFeature::eFieldHealth)
	{
		return PYXValue((uint8_t)m_nHealth);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldType)
	{
		return PYXValue((uint8_t)m_nType);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldMovesLeft)
	{
		return PYXValue((uint8_t)m_nMovesLeft);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldPlayerID)
	{
		return PYXValue((uint8_t)m_nPlayerID);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldLoaded)
	{
		return PYXValue((uint8_t)m_nLoaded);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldFuel)
	{
		return PYXValue((uint8_t)m_nFuel);
	}
	if(nFieldIndex==PyxilizationFeature::eFieldBuildType)
	{
		return PYXValue((uint8_t)m_nBuildType);
	}
	return PYXValue();
}