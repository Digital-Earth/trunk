#pragma once

#ifndef XY_COVERAGE_TRANSLATOR_H
#define XY_COVERAGE_TRANSLATOR_H
/******************************************************************************
xy_coverage_translator.h

begin		: 2010-04-19
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_sampling.h"

#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/value_translation.h"
#include "pyxis/pipe/process.h"

#include "boost/scoped_ptr.hpp"

class MODULE_SAMPLING_DECL XYCoverageTranslator : public IXYCoverage
{

//Internall class
	class XYCoverageValueGetterTranslator : public XYCoverageValueGetter
	{
	protected:
		XYCoverageValueGetter * m_getter;
		const XYCoverageTranslator & m_instance;

	public:
		XYCoverageValueGetterTranslator(const XYCoverageTranslator & instance) : m_instance(instance),m_getter(instance.m_coverage->getCoverageValueGetter())
		{
		}

		virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
														PYXValue* pValue) const 
		{
			boost::recursive_mutex::scoped_lock lock(m_instance.m_mutex);

			//get original value and place it in m_inputValues[0]
			if (m_getter->getCoverageValue(native,&m_instance.m_singleInputValue))
			{
				//if we got a value, translate it.
				*pValue = m_instance.m_valueTranslator->translateFieldByIndex(0,m_instance.m_singleInputValue);
				return true;
			}

			return false;
		}
	};

	class XYAsyncValueGetterTranslator : public XYAsyncValueGetter, public XYAsyncValueConsumer
	{
	protected:
		std::vector<PYXValue> m_values;
		PYXPointer<XYAsyncValueGetter> m_getter;
		const XYCoverageTranslator & m_instance;
		const XYAsyncValueConsumer & m_consumer;

	public:
		XYAsyncValueGetterTranslator(const XYCoverageTranslator & instance,const XYAsyncValueConsumer & consumer,int width,int height) : m_instance(instance), m_consumer(consumer)
		{
			m_values.resize(width*height);
			m_getter = instance.m_coverage->getAsyncCoverageValueGetter(*this,width,height);
		}

		static PYXPointer<XYAsyncValueGetterTranslator> create(const XYCoverageTranslator & instance,const XYAsyncValueConsumer & consumer,int width,int height)
		{
			return PYXNEW(XYAsyncValueGetterTranslator,instance,consumer,width,height);
		}

		virtual void addAsyncRequests(const PYXTile & tile)
		{
			m_getter->addAsyncRequests(tile);
		}

		virtual void addAsyncRequest(const PYXIcosIndex & index)
		{
			m_getter->addAsyncRequest(index);
		}

		virtual bool join()
		{
			return m_getter->join();
		}

		void onRequestCompleted(const PYXIcosIndex & index,
								const PYXCoord2DDouble & nativeCoord,
								bool * hasValues,
								PYXValue * values,
								int width,int height) const
		{
			if (hasValues != 0 && values != 0)
			{
				int count = width*height;

				std::vector<PYXValue> translatedValues(count);

				for(int i=0;i<count;++i)
				{
					if (hasValues[i])
					{
						translatedValues[i] = m_instance.m_valueTranslator->translateFieldByIndex(0,values[i]);
					}
				}
				m_consumer.onRequestCompleted(index,nativeCoord,hasValues,&translatedValues[0],width,height);
			}
			else
			{
				m_consumer.onRequestCompleted(index,nativeCoord,0,0,width,height);
			}
		}
	};

//Class definition
	PYXCOM_DECLARE_CLASS();

//PXCOM_IUnknown
public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
	IUNKNOWN_QI_END
	
	IUNKNOWN_RC_IMPL();

	IUNKNOWN_DEFAULT_CAST( XYCoverageTranslator, IXYCoverage);

protected:
	boost::intrusive_ptr<IXYCoverage> m_coverage;
	PYXPointer<IDefinitionTranslator> m_valueTranslator;

	//! Mutex to serialize concurrent access by multiple threads.
	mutable boost::recursive_mutex m_mutex;

	//! Private PYXValue for pointer translations (protected by m_mutex)
	mutable std::vector<PYXValue> m_inputValues;

	mutable PYXValue m_singleInputValue;

	mutable boost::scoped_ptr<XYCoverageValueGetterTranslator> m_valueGetter;

	void resizeInputValuesVector(size_t newSize) const
	{
		//create compatable input definition values
		while (m_inputValues.size() < newSize)
		{
			m_inputValues.push_back(
				m_coverage->getCoverageDefinition()->getFieldDefinition(0).getTypeCompatibleValue() );
		}
	}

public:
	XYCoverageTranslator(boost::intrusive_ptr<IXYCoverage> coverage,
						 boost::intrusive_ptr<IDefinitionTranslator> valueTranslator
						 ) : m_coverage(coverage),m_valueTranslator(valueTranslator)
	{
		resizeInputValuesVector(1);
		m_singleInputValue = m_inputValues[0];
	}

	virtual ~XYCoverageTranslator()
	{
	}


//IRecrod - not translated
public:
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const 
	{
		//retrun the TableDefinition of the coverted valued
		return m_coverage->getDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition() 
	{
		//retrun the TableDefinition of the coverted valued
		return m_coverage->getDefinition();
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		return m_coverage->getFieldValue(nFieldIndex);
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		m_coverage->setFieldValue(value,nFieldIndex);
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		return m_coverage->getFieldValueByName(strName);
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		m_coverage->setFieldValueByName(value,strName);
	}
	
	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_coverage->getFieldValues();
	}
	
	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{		
		m_coverage->setFieldValues(vecValues);
	}
	
	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		m_coverage->addField(strName,nContext,nType,nCount,value);		
	}

//IFeature - not translated
public:	
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return m_coverage->isWritable();
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_coverage->getID();
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_coverage->getGeometry();
	}
	
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_coverage->getGeometry();
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_coverage->getStyle();
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_coverage->getStyle(strStyleToGet);
	}

//IFeatueCollection - not translated
public:
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const
	{
		return m_coverage->getIterator();
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const
	{
		return m_coverage->getIterator(geometry);
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_coverage->getFeatureDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_coverage->getFeatureDefinition();
	}

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const
	{
		return m_coverage->getFeatureStyles();
	}

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const
	{
		return m_coverage->getFeature(strFeatureID);
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return m_coverage->canRasterize();
	}

	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom)
	{
		m_coverage->geometryHint(spGeom);
	}

	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom)
	{
		m_coverage->endGeometryHint(spGeom);
	}

//IXYCoverage - translated
public:
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		//translated from m_coverage->getCoverageDefinition();
		return m_valueTranslator->getOutputDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		//translated from m_coverage->getCoverageDefinition();
		return m_valueTranslator->getOutputDefinition();
	}

	virtual XYCoverageValueGetter* STDMETHODCALLTYPE getCoverageValueGetter() const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		if (!m_valueGetter)
		{
			m_valueGetter.reset(new XYCoverageValueGetterTranslator(*this));
		}

		return m_valueGetter.get();
	}

	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter(
			const XYAsyncValueConsumer & consumer,
			int matrixWidth,
			int matrixHeight
		) const 
	{
		return XYAsyncValueGetterTranslator::create(*this,consumer,matrixWidth,matrixHeight);
	}

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		//get original value and place it in m_inputValues[0]
		if (m_coverage->getCoverageValue(native,&(m_singleInputValue)))
		{
			//if we got a value, translate it.
			*pValue = m_valueTranslator->translateFieldByIndex(0,m_singleInputValue);			
			return true;
		}

		return false;
	}

	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
													 PYXValue* pValues,
													 int sizeX,
													 int sizeY) const
	{		
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		//make sure we have enought PYXValues
		resizeInputValuesVector(sizeX*sizeY);

		m_coverage->getMatrixOfValues(nativeCentre,&(m_inputValues[0]),sizeX,sizeY);

		for (int i=0;i<sizeX*sizeY;i++)
		{
			if (!m_inputValues[i].isNull())
			{
				pValues[i] = m_valueTranslator->translateFieldByIndex(0,m_inputValues[i]);
			}
			else
			{
				//set it to null
				pValues[i] = PYXValue();
			}
		}
	}

	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const
	{
		return m_coverage->hasSpatialReferenceSystem();
	}

	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
	{
		return m_coverage->setSpatialReferenceSystem(spSRS);
	}

	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const
	{
		return m_coverage->getCoordConverter();
	}

	virtual double STDMETHODCALLTYPE getSpatialPrecision() const
	{
		return m_coverage->getSpatialPrecision();
	}

	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const
	{
		return m_coverage->getBounds();
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const
	{
		return m_coverage->getStepSize();
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
	{
		return m_coverage->nativeToRasterSubPixel(native);
	}

	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const
	{
		return m_coverage->tileLoadHint(tile);
	}

	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const
	{
		return m_coverage->tileLoadDoneHint(tile);
	}
};



class MODULE_SAMPLING_DECL XYCoverageTranslatorProcess : public ProcessImpl<XYCoverageTranslatorProcess>
{
	PYXCOM_DECLARE_CLASS();

public:
	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IProcess)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IFeature

	IFEATURE_IMPL();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return m_spXYCoverage;
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return m_spXYCoverage;
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:

	//! The GDAL XY Coverage.
	boost::intrusive_ptr<XYCoverageTranslator> m_spXYCoverage;

	PYXValue::eType m_outputType;

	std::string m_translateOperation;

public:

	XYCoverageTranslatorProcess();

};



#endif // guard
