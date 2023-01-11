#ifndef PYXIS__DATA__FEATURE_STYLE_H
#define PYXIS__DATA__FEATURE_STYLE_H
/******************************************************************************
feature_style.h

begin		: August 21, 2007
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/pointer.h"
#include "pyxis/utility/sxs.h"

// windows includes
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
#define _WIN32_WINNT 0x0502 // to get SetDllDirectory (requires Windows XP SP1 or greater)
#include <windows.h>
#pragma warning(pop)

// standard includes
#include <string>
#include <vector>

/*!
A Feature's style.
*/
class PYXLIB_DECL FeatureStyle
{

public:
	FeatureStyle();

	//! Initialize a style with the style string.
	FeatureStyle(const std::string& strSLDStyle);

	//! Returns the style in SLD.
	std::string getStyle() const;
	
	/*! Returns the text to display in the Legend when a feature is open or 
	when the camera is on a feature.
	*/
	std::vector<std::string> getLegendText() const;

	//! Returns a file handle to the legend icon.
	/*HBITMAP getLegendIcon() const;*/

	//! Set the text to display for this style in Legend Control.
	void setLegendText(const std::string& strText);	

	//! Set the SLD style.
	void setStyle(const std::string& strSLDStyle);

	/*! Returns the field name, field value and 
	the style to apply.	
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	strStyleToApply	Whether looking for a particular style or 
	all styles.
	*/
	void getRulesFromStyle(std::vector<std::string>& vecFieldNames, 
		std::vector<std::string>& vecFieldValues, 
		std::vector<std::string>& vecStylesToApply, 
		const std::string& strStyleToApply);	

private:
	/*! Parses a style and returns the fields, values and how to style.
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	isxs	ISXS object generated from SLD style.
	*/
	void parseStyle(std::vector<std::string>& vecFieldNames, 
		std::vector<std::string>& vecFieldValues, 
		std::vector<std::string>& vecStylesToApply, 
		ISXS& isxs);

	/*! Parses a style and returns the fields, values for a particular style.
	\param	vecFieldNames	The feature's field to apply the style to (this is 
	always modified by this method).
	\param	vecFieldValues	The field value on which to apply the style to (this is 
	always modified by this method).
	\param	vecStylesToApply	The style (colour, icon etc) to apply (this is 
	always modified by this method).
	\param	isxs	ISXS object generated from SLD style.
	\param	strStyleToApply	Whether looking for a particular style or 
	all styles.
	*/
	void parseStyle(std::vector<std::string>& vecFieldNames, 
		std::vector<std::string>& vecFieldValues, 
		std::vector<std::string>& vecStylesToApply, 
		ISXS& isxs, const std::string& strStyleToApply);

	/*! Parses a style and sets the legend text.
	\param	strFeatureType	The feature type the style applies to.
	\param	isxs	ISXS object generated from SLD style.
	*/
	void parseStyle(std::string strFeatureType, ISXS& isxs);

	//! The list of text (1 per rule) to display for the style.
	std::vector<std::string> m_vecLegendText;
	
	//! The style in SLD.
	std::string m_strSLDStyle;
};

#endif // guard
