#ifndef styled_features_summary_H
#define styled_features_summary_H
/******************************************************************************
styled_features_summary.h

begin		: March 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"
#include "bitmap_process.h"

// pyxlib includes
#include "pyxis/data/feature_group.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/embedded_resource_holder.h"


/*!
Input: Features Group
Output: Styled Feature Group
*/
//! Applies a user-specified style to a feature collection.
class MODULE_FEATURE_PROCESSING_PROCS_DECL StyledFeaturesSummaryProcess : 
	public ProcessImpl<StyledFeaturesSummaryProcess>, public IFeatureGroup, public IEmbeddedResourceHolder
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	StyledFeaturesSummaryProcess();

	//! Destructor
	~StyledFeaturesSummaryProcess();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IEmbeddedResourceHolder)
		IUNKNOWN_QI_CASE(IFeatureGroup)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(StyledFeaturesSummaryProcess, IProcess);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const;

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput();

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getIdentity() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

	//IFeatureGroup
public:
	virtual Range<int> STDMETHODCALLTYPE getFeaturesCount() const;

	virtual bool STDMETHODCALLTYPE moreDetailsAvailable() const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(int fieldIndex) const;

	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroup(const std::string & groupId) const;

	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroupForFeature(const std::string & featureId) const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator(const PYXGeometry& geometry) const;

public: //IEmbeddedResourceHolder

	virtual int getEmbeddedResourceCount() const;

	virtual boost::intrusive_ptr<IProcess> getEmbeddedResource(int index) const;

public:

	static void test();

public :

	//! The attribute name for use in Process Editor.
	static const std::string kstrLineColour;

	//! The attribute name for use in Process Editor.
	static const std::string kstrIcon;

private:

	//! The input to this process.
	boost::intrusive_ptr<IFeatureGroup> m_spInputFG;

	PYXPointer<IBitmap> m_spBitmap;

	//! The bitmap pipeline definition
	std::string m_bitmapPipelineDefinition;

	//! The icon index specify by the user.
	int m_iconIndex;

	//! which text field to use as a text
	std::string m_textField;

	//! alignment around the icon
	std::string m_textAlignment;

	//! text appearing mode (Always/OnMouseOver)
	std::string m_textAppearanceMode;

	//! font settings
	std::string m_fontStyle;
	std::string m_fontColor;
	std::string m_fontFamily;
	int m_fontSize;

	int m_iconScaling;
	std::string m_iconStyle;
	std::string m_iconColour;	
	std::string m_iconColourPalette;
	std::string m_iconColourPaletteScale;
	std::string m_iconColourPaletteField;


	//! line styling
	std::string m_borderStyle;
	std::string m_lineColour;
	int m_lineWidth;
	int m_lineOpacity;

	//! fill styling
	std::string m_areaStyle;
	std::string m_areaColour;
	int m_areaOpacity;
	std::string m_areaPalette;
	std::string m_areaPaletteScale;
	std::string m_areaPaletteField;


	//! make sure the attirbutes are valid and fix to default settings if not.
	void sanitizeAttributes();

	//! update the style string
	void updateStyle();
};

#endif // guard
