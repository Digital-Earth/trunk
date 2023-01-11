/******************************************************************************
feature_style.cpp

begin		: August 23, 2007
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "feature_style.h"

FeatureStyle::FeatureStyle()
{
}

FeatureStyle::FeatureStyle(const std::string& strSLDStyle)
{
	m_strSLDStyle = strSLDStyle;
	
	boost::intrusive_ptr<ISXS> spObject;		
	int nStyleSize = static_cast<int>(m_strSLDStyle.size()) + 1;
		
	SXSParser::getDefaultParser().readObjectFromString(
		m_strSLDStyle.c_str(), nStyleSize, spObject);

	// TODO: Add error handling.  XSD-based.  Should this be on .Net side?
	assert(spObject->getSXSName() == "StyledLayerDescriptor");

	parseStyle("", *spObject);	
}

std::string FeatureStyle::getStyle() const
{
	return m_strSLDStyle;
}

std::vector<std::string> FeatureStyle::getLegendText() const
{
	return m_vecLegendText;
}

//HBITMAP FeatureStyle::getLegendIcon() const
//{
//	// TODO: need to test that HBITMAP works on the C# side
//	LPCTSTR fileName = (LPCTSTR)"c:\red.bmp";
//	HBITMAP iconHandle = (HBITMAP)::LoadImage( 
//		NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
//
//	if(iconHandle->unused)
//	{
//				
//	}
//	
//	return iconHandle;
//}

void FeatureStyle::setStyle(const std::string& strSLDStyle)
{
	m_vecLegendText.push_back(strSLDStyle);
}

void FeatureStyle::setLegendText(const std::string& strText)
{
	m_vecLegendText.push_back(strText);
}


// TODO: Right now, we only handle the OGC PropertyIsEqualTo filter.  There 
// are other filters we need to handle, such as PropertyIsGreaterThan, 
// PropertyIsLessThan, 'Else' filters and so on.
// TODO: Modify parsing logic as we continue supporting more and more SLD tags

/*! Accepts an SLD style string and returns the field name, field value and 
	the style to apply.	
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecFieldStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	strStyleToApply	Whether looking for a particular style or 
	all styles.
*/
void FeatureStyle::getRulesFromStyle(std::vector<std::string>& vecFieldNames, 
		std::vector<std::string>& vecFieldValues, 
		std::vector<std::string>& vecStylesToApply, 
		const std::string& strStyleToApply)
{
	if (!m_strSLDStyle.empty())
	{
		boost::intrusive_ptr<ISXS> spObject;		
		int nStyleSize = static_cast<int>(m_strSLDStyle.size()) + 1;
		
		SXSParser::getDefaultParser().readObjectFromString(
			m_strSLDStyle.c_str(), nStyleSize, spObject);

		assert(spObject->getSXSName() == "StyledLayerDescriptor");

		if (strStyleToApply.empty())
		{
			parseStyle(vecFieldNames, vecFieldValues, 
				vecStylesToApply, *spObject);
		}		
		else
		{
			parseStyle(vecFieldNames, vecFieldValues, 
				vecStylesToApply, *spObject, strStyleToApply);
		}
	}
}

/*! Parses a style and returns the fields, values and how to style.
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecFieldStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	isxs	ISXS object generated from SLD style.
*/
void FeatureStyle::parseStyle(std::vector<std::string>& vecFieldNames, 
	std::vector<std::string>& vecFieldValues, 
	std::vector<std::string>& vecStylesToApply, 
	ISXS& isxs)
{
	int nChildCount = isxs.getNumberOfSXSChildren();
	for (int nChild = 0; nChild < nChildCount; ++nChild)
	{
		boost::intrusive_ptr<ISXS> spChild = isxs.getSXSChild(nChild);
		
		if (spChild->getSXSName() == "Filter")
		{
			const std::string strFieldName = 
				spChild->getSXSChild(0)->getSXSChild(0)->getSXSCharacters();
			
			const std::string strFieldValue = 
				spChild->getSXSChild(0)->getSXSChild(1)->getSXSCharacters();

			vecFieldNames.push_back(strFieldName);
			vecFieldValues.push_back(strFieldValue);			
		}
		else if (spChild->getSXSName() == "LineSymbolizer")
		{
			const std::string strStyle = 
				spChild->getSXSChild(0)->getSXSChild(0)->getSXSCharacters();

			vecStylesToApply.push_back(strStyle);
		}
		else
		{
			parseStyle(vecFieldNames, vecFieldValues, vecStylesToApply, 
				*spChild);
		}
	}
}

/*! Parses a style and returns the fields, values for a particular style.
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecFieldStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	isxs	ISXS object generated from SLD style.
*/
void FeatureStyle::parseStyle(std::vector<std::string>& vecFieldNames, 
	std::vector<std::string>& vecFieldValues, 
	std::vector<std::string>& vecStylesToApply, 
	ISXS& isxs, const std::string& strStyleToApply)
{
	int nChildCount = isxs.getNumberOfSXSChildren();
	for (int nChild = 0; nChild < nChildCount; ++nChild)
	{
		boost::intrusive_ptr<ISXS> spChild = isxs.getSXSChild(nChild);
		
		if (spChild->getSXSName() == "Filter")
		{
			const std::string strFieldName = 
				spChild->getSXSChild(0)->getSXSChild(0)->getSXSCharacters();
			
			const std::string strFieldValue = 
				spChild->getSXSChild(0)->getSXSChild(1)->getSXSCharacters();

			vecFieldNames.push_back(strFieldName);
			vecFieldValues.push_back(strFieldValue);			
		}
		else if (spChild->getSXSName() == strStyleToApply)
		{
			const std::string strStyle = 
				spChild->getSXSChild(0)->getSXSChild(0)->getSXSCharacters();

			vecStylesToApply.push_back(strStyle);
		}
		else
		{
			parseStyle(vecFieldNames, vecFieldValues, vecStylesToApply, *spChild);
		}
	}	
}

/*! Parses a style and sets the legend text.
	\param	strFeatureType	The feature type the style applies to.
	\param	isxs	ISXS object generated from SLD style.
*/
void FeatureStyle::parseStyle(std::string strFeatureType, ISXS& isxs)
{
	int nChildCount = isxs.getNumberOfSXSChildren();
	for (int nChild = 0; nChild < nChildCount; ++nChild)
	{
		boost::intrusive_ptr<ISXS> spChild = isxs.getSXSChild(nChild);
		
		if (spChild->getSXSName() == "UserStyle")
		{
			strFeatureType = spChild->getSXSChild(0)->getSXSCharacters();
			parseStyle(strFeatureType, *spChild);
		}
		else if (spChild->getSXSName() == "Rule")
		{
			const std::string strRule = 
				spChild->getSXSChild(0)->getSXSCharacters();

			std::string strLegend = "";
			strLegend.append(strFeatureType).append(".").append(strRule);

			m_vecLegendText.push_back(strLegend);
		}
		else
		{
			parseStyle(strFeatureType, *spChild);
		}
	}
}