/******************************************************************************
default_feature.cpp

begin		: 2007-05-29
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/default_feature.h"

// pyxlib includes
#include "pyxis/utility/tester.h"

// {27BDE94F-02F9-41d6-8766-9E6355DB09E2}
PYXCOM_DEFINE_CLSID(DefaultFeature, 
0x27bde94f, 0x2f9, 0x41d6, 0x87, 0x66, 0x9e, 0x63, 0x55, 0xdb, 0x9, 0xe2);
PYXCOM_CLASS_INTERFACES(DefaultFeature, IFeature::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(DefaultFeature, "Generic Feature", "A generic feature object. Used mainly for testing.", "Unknown",
							IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// {B081C169-869B-483e-B85B-4AF25B4104B6}
PYXCOM_DEFINE_CLSID(DefaultFeatureCollection, 
0xb081c169, 0x869b, 0x483e, 0xb8, 0x5b, 0x4a, 0xf2, 0x5b, 0x41, 0x4, 0xb6);
PYXCOM_CLASS_INTERFACES(DefaultFeatureCollection, IFeatureCollection::iid, IFeature::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(DefaultFeatureCollection, "Generic Feature Collection", "A generic collection of feature objects. Used mainly for testing.", "Unknown",
					IFeatureCollection::iid, IFeature::iid, IProcess::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

namespace
{

//! The unit test class.
Tester<DefaultFeature> gTester;

}

//! Unit test.
void DefaultFeature::test()
{
	{
		DefaultFeature f;
		f.addField("rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
		unsigned char buf[3] = { 255, 128, 0 };
		PYXValue v(buf, 3);
		f.setFieldValueByName(v, "rgb");
	}

	{
		boost::intrusive_ptr<IFeature> spF = boost::intrusive_ptr<IFeature>(new DefaultFeature);
		TEST_ASSERT(spF);

		spF->addField("rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
		unsigned char buf[3] = { 255, 128, 0 };
		PYXValue v(buf, 3);
		spF->setFieldValueByName(v, "rgb");
	}

	{
		boost::intrusive_ptr<DefaultFeatureCollection> fc(new DefaultFeatureCollection());

		boost::intrusive_ptr<DefaultFeature> f1(new DefaultFeature());
		boost::intrusive_ptr<DefaultFeature> f2(new DefaultFeature());
		boost::intrusive_ptr<DefaultFeature> f3(new DefaultFeature());

		fc->addFeature(f1);
		fc->addFeature(f2);
		fc->addFeature(f3);

		PYXPointer<FeatureIterator> spIt = fc->getIterator();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getFeature().get() == f1.get());
		spIt->next();
		TEST_ASSERT(spIt->getFeature().get() == f2.get());
		spIt->next();
		TEST_ASSERT(spIt->getFeature().get() == f3.get());
		spIt->next();
		TEST_ASSERT(spIt->end());
	}
}
