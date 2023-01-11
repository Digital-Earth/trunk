#pragma once
#ifndef VIEW_MODEL__ANNOTATION_H
#define VIEW_MODEL__ANNOTATION_H
/******************************************************************************
annotation.h

begin		: 2010-22-06
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model_api.h"
#include "surface_memento.h"

//Helper class to save some memory
class AnnotationRef; 
class STileMesh;

/*!
Annotation - Base class for Annotation on the globe.

Annotation is abstraction for displaying annotation on the globe. the creation of a specific annotation is done by AnnotationsMementoFiller.
This abstraction allow us to generate different annotations (Icons,Polygons,3D models).

In order to let the globe deal with all king of Annotations, the Annotation class extend the IAnnotation with the following API:
1. Events handling : onMouse*(PYXPointer<AnnotationMouseEvent> eventData) - handle mouse interaction.
2. UI generation: the globe would call canGenerateVisualization before call generateVisualization.
3. UI destruction: when the Annotation get out of screen or become hidden. the globe would request it Visualization be destoried.
4. dynamic UI: if the Annotation declare itself as Dynamic, the updateVisualization would be called before every frame.

*/
class Annotation : public IAnnotation
{
protected:
	//Used as a container for Annotations
	friend class PatchAnnotations;

public:
	static void closeAllResources();

public:
	Annotation(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const std::string & style,const std::string & featureID,const std::string & groupId = std::string());
	virtual ~Annotation();

	//IAnnotation interface
public:
	virtual const ProcRef & getProcRef() const;

	virtual const boost::intrusive_ptr<IProcess> & getProcess() const;

	virtual const std::string & getFeatureID() const;

	virtual PYXPointer<IViewModel> getViewModel();

	virtual boost::intrusive_ptr<IFeature> getFeature();

	const std::string & getStyle() const;

protected:
	//! To save up some memory pointing to the procRef,viewModel and Style.
	PYXPointer<AnnotationRef>	   m_ref;
	
	//! the feature ID
	std::string				       m_featureID;

	//! the group ID if this is feature Group input
	std::string				       m_groupID;
	
	//! If the actual feature was requested, keep a pointer to it.
	boost::intrusive_ptr<IFeature> m_feature;
	
	//! Where the visualization is dynamic or not
	bool						   m_isDynamic;
	
	//! The Tile that contains the annotation
	PatchAnnotations *			   m_container;

	//Event handling
public:
	virtual void onMouseEnter(PYXPointer<AnnotationMouseEvent> eventData) {};
	virtual void onMouseLeave(PYXPointer<AnnotationMouseEvent> eventData) {};
	virtual void onMouseMove(PYXPointer<AnnotationMouseEvent> eventData) {};
	virtual void onMouseClick(PYXPointer<AnnotationMouseEvent> eventData) {};
	virtual void onMouseDoubleClick(PYXPointer<AnnotationMouseEvent> eventData) {};

	//UI creation;
public:

	//! Return true if the visualization was created
	virtual bool wasVisualizationGenerated() = 0;
	
	//! Return true if the Annotation has all needed resources to create the visualization
	virtual bool canGenerateVisualization() = 0;
	
	//! generate the visualization, this operation should be quick because it run under OpenGLThread
	virtual void generateVisualization() = 0;
	
	//! destroy the visualization this operation should be quick because it run under OpenGLThread
	virtual void destroyVisualization() = 0;
		
	//! called by the AnnoationController before every frame to update the annotation
	virtual void updateVisualization() = 0;

	//! set the Dynamic flag for the Annotation. the globe would call updateVisualization before every frame for a Dynamic Annotation.
	void setDynamicVisualization(const bool & dynamic);
	bool isDynamicVisualization() const;

	//! called when a new ElevationMesh was created for the Annotation Tile
	virtual void validateElevation() {};
};

typedef std::set<PYXPointer<Annotation>> AnnotationsSet;

/*!

AnnotationsTile - container of Annotations inside a Tile.

The AnnotationsTile keeps 3 list of annotations:
1. not generated annotations - all new annotation that haven't generate their visualization
2. generated annotations - all annotation that generate their visualization
3. dynamic annotations - all annotations that needed to be updated before every frame

the Annotation controller is responsible for creating, update, and destroy annotations.

*/
class PatchAnnotations : public PYXObject, ObjectMemoryUsageCounter<PatchAnnotations>
{
protected:
	friend class Annotation;

public:
	PatchAnnotations();	
	virtual ~PatchAnnotations();
	
	static PYXPointer<PatchAnnotations> create()
	{
		return PYXNEW(PatchAnnotations);
	}

protected:
	AnnotationsSet m_notGeneratedAnnotations;
	AnnotationsSet m_generatedAnnotations;
	AnnotationsSet m_dynamicAnnotations;

public:
	void addAnnotation(PYXPointer<Annotation> annotation);
	void removeAllAnnotationOfPipeline(const ProcRef & procRef);
	void generateAnnotations();
	void updateAnnotations();
	void destroyAnnotations();

	//update annotation elevation if needed
	void validateElevation();

	int getAnnotationsCount();

protected:
	int m_tag;

public:
	int getTag() const { return m_tag; } 
	void setTag(int tag) { m_tag = tag; }
};

/*!
AnnotationCache is an storage calls that aggregate all given annotations by a given weight and cell resolutions.
the AnnotationCache can be used to fast query which Annotation should be generated for a specifc cell.

This class is not thread safe. 
*/
//! AnnotationCache used to generate an aggregation cache to store all annotations
class AnnotationCache : public PYXObject
{
public:
	struct AnnotationInfo : public ObjectMemoryUsageCounter<AnnotationInfo>
	{
		double m_weight;
		std::string m_featureID;
		std::string m_groupID;
		PYXCoord3DDouble m_location;
		bool m_isGroup;

		AnnotationInfo(const std::string & featureID, const PYXCoord3DDouble  & location) : m_featureID(featureID), m_location(location), m_weight(0), m_isGroup(false)
		{
		}

		AnnotationInfo(const std::string & featureID, const PYXCoord3DDouble  & location, double weight) : m_featureID(featureID), m_location(location), m_weight(weight), m_isGroup(false)
		{
		}

		AnnotationInfo(const std::string & featureID, const std::string & groupId,PYXCoord3DDouble  & location, double weight) : m_featureID(featureID), m_groupID(groupId), m_location(location), m_weight(weight), m_isGroup(false)
		{
		}

		AnnotationInfo(const std::string & featureID, const std::string & groupId, const PYXCoord3DDouble  & location, double weight, bool group) : m_featureID(featureID), m_groupID(groupId), m_location(location), m_weight(weight), m_isGroup(group)
		{
		}

		AnnotationInfo(const AnnotationInfo & other) : ObjectMemoryUsageCounter(other), m_featureID(other.m_featureID) ,m_location(other.m_location) ,m_weight(other.m_weight), m_isGroup(other.m_isGroup), m_groupID(other.m_groupID)
		{
		}

		~AnnotationInfo()
		{
		}
	};

protected:
	class Node : ObjectMemoryUsageCounter<Node>
	{
	protected:
		Node * subNodes[7];
		Node * parent;
		Node * maxChild;
		int    depth;

		std::list<AnnotationInfo> annotations;

		static std::list<AnnotationInfo> s_emptyAnnotations;

	public:
		//! create a node with depth 0
		Node();

		//! create a node with a specific depth
		Node(int aDepth);

		//! dtor
		~Node();

		//! get a specific node by a given (sub) index. if autoGenetate is ture, generate the node if the node doesn't exists
		Node * getNode(const PYXIndex & index,bool autoGenerate = true);

	protected:
		//! internal use function
		Node * getNode(const PYXIndex & index,int digitIndex,bool autoGenerate);
		
		//! compare between two nodes. first check the depth, if depth is equal compate the annotations max weight
		bool operator>(const Node & node) const;
		
	public:
		//! add an annotation info to a specific node (by the given index)
		void addAnnotation(const PYXIndex & index,int topResolutionDelta, AnnotationInfo & info);
		
		//! add an annotation info to this node - will propoage up if node weight is big enough
		void addAnnotation(int topResolutionDelta, AnnotationInfo & info);
		
		//! get annotation for this node. if no annotations, will find a child node with annotations with maxium weight/depth
		const std::list<AnnotationInfo> & getAnnotation(const PYXIndex & index);
	};

protected:
	std::vector<Node> roots;
	int annotationCount;

	int getRootIndex(const PYXIcosIndex & index) const
	{
		if (index.isVertex())
		{
			return index.getPrimaryResolution() - index.knFirstVertex;
		}
		else
		{
			return index.getPrimaryResolution() - index.kcFaceFirstChar + index.knLastVertex - index.knFirstVertex + 1;
		}
	}

public:
	AnnotationCache() : annotationCount(0)
	{
		roots.resize(32);
	}

	static PYXPointer<AnnotationCache> create()
	{
		return PYXNEW(AnnotationCache);
	}

	void addAnnotation(const PYXIcosIndex & index,const PYXIcosIndex & topLevelIndex,AnnotationInfo & info)
	{
		annotationCount++;
		return roots[getRootIndex(index)].addAnnotation(index.getSubIndex(),index.getResolution()-topLevelIndex.getResolution(),info);
	}

	const std::list<AnnotationInfo> & getAnnotation(const PYXIcosIndex & index) 
	{
		return roots[getRootIndex(index)].getAnnotation(index.getSubIndex());
	}

	int getAnnotationCount() const { return annotationCount; }

	static void test();
};

#endif