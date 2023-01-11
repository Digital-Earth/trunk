/******************************************************************************
annotation.cpp

begin		: 2010-22-06
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "annotation.h"

#include "pyxis/data/feature_group.h"
#include "pyxis/utility/trace.h"

#include <list>
#include <map>

///////////////////////////////////////////////////////////////////////////////
// AnnotationRef 
///////////////////////////////////////////////////////////////////////////////

class AnnotationRef : public PYXObject
{
protected:
	IViewModel * m_viewModel;
	boost::intrusive_ptr<IProcess> m_process;
	ProcRef m_procRef;
	std::string m_style;

	typedef std::list<PYXPointer<AnnotationRef>> List;

	static List s_list;
	static boost::recursive_mutex m_staticMapMutex;

public:
	AnnotationRef(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const std::string & style) 
		: m_viewModel(view), m_process(process), m_procRef(process), m_style(style) 
	{
	}

	static PYXPointer<AnnotationRef> create(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const std::string & style) 
	{
		boost::recursive_mutex::scoped_lock lock(m_staticMapMutex);

		List::iterator it = s_list.begin();

		ProcRef procRef(process);

		while (it!=s_list.end())
		{
			if ((*it)->m_viewModel == view && 
				(*it)->m_procRef == procRef && 
				(*it)->m_style == style)
			{
				//make sure next time we don't need to search the whole list
				if (it!=s_list.begin())
				{
					std::swap(s_list.front(),*it);
				}
				return s_list.front();
			}

			List::iterator removeIt = it;
			++it;

			if ((*removeIt)->getRefCount()==1)
			{
				s_list.erase(removeIt);	
			}
		}

		PYXPointer<AnnotationRef> newRef = PYXNEW(AnnotationRef,view,process,style);
		s_list.push_front(newRef);

		return newRef;
	}

	static void closeAllResources() {
		boost::recursive_mutex::scoped_lock lock(m_staticMapMutex);
		s_list.clear();
	}

	virtual ~AnnotationRef() 
	{
	}

	const boost::intrusive_ptr<IProcess> & getProcess() const
	{
		return m_process;
	}

	const ProcRef & getProcRef() const
	{
		return m_procRef;
	}

	const std::string & getStyle() const
	{
		return m_style;
	}

	PYXPointer<IViewModel> getViewModel()
	{
		return m_viewModel;
	}

	bool operator ==(const AnnotationRef & other)
	{
		return m_style == other.m_style && m_viewModel == other.m_viewModel && m_procRef == other.m_procRef;
	}
};


AnnotationRef::List AnnotationRef::s_list;
boost::recursive_mutex AnnotationRef::m_staticMapMutex;


///////////////////////////////////////////////////////////////////////////////
// Annotation
///////////////////////////////////////////////////////////////////////////////

Annotation::Annotation(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const std::string & style,const std::string & featureID,const std::string & groupId)
	: m_featureID(featureID), m_groupID(groupId), m_isDynamic(false)
{	
	m_ref = AnnotationRef::create(view,process,style);
}

Annotation::~Annotation()
{
}

void  Annotation::closeAllResources()
{
	AnnotationRef::closeAllResources();
}

const boost::intrusive_ptr<IProcess> & Annotation::getProcess() const
{
	return m_ref->getProcess();
}

const ProcRef & Annotation::getProcRef() const
{
	return m_ref->getProcRef();
}

const std::string & Annotation::getFeatureID() const
{
	return m_featureID;
}

boost::intrusive_ptr<IFeature> Annotation::getFeature()
{
	if (! m_feature)
	{
		if (m_groupID.empty())
		{
			boost::intrusive_ptr<IFeatureCollection> spFC = getProcess()->getOutput()->QueryInterface<IFeatureCollection>();

			if (spFC)
			{
				m_feature = spFC->getFeature(getFeatureID());
			}
		}
		else
		{
			boost::intrusive_ptr<IFeatureGroup> spFG = getProcess()->getOutput()->QueryInterface<IFeatureGroup>();

			if (spFG)
			{
				auto group = spFG->getFeatureGroup(m_groupID);

				if (group) 
				{
					if (m_featureID.empty()) 
					{
						m_feature = group;
					}
					else 
					{
						m_feature = group->getFeature(getFeatureID());
					}
				}
			}
		}
	}

	return m_feature;
}

PYXPointer<IViewModel> Annotation::getViewModel()
{
	return m_ref->getViewModel();
}

const std::string & Annotation::getStyle() const
{
	return m_ref->getStyle();
}




void Annotation::setDynamicVisualization(const bool & dynamic)
{
	m_isDynamic = dynamic;

	if (m_container != NULL)
	{			
		if (m_isDynamic)
		{
			m_container->m_dynamicAnnotations.insert(this);
		}
		else
		{
			m_container->m_dynamicAnnotations.erase(this);
		}
	}
}

bool Annotation::isDynamicVisualization() const
{
	return m_isDynamic;
}


//////////////////////////////////////////////////////////////////
// PatchAnnotations
//////////////////////////////////////////////////////////////////

PatchAnnotations::PatchAnnotations()
{
}

PatchAnnotations::~PatchAnnotations()
{
	destroyAnnotations();
}


void PatchAnnotations::addAnnotation(PYXPointer<Annotation> annotation)
{
	if (annotation->wasVisualizationGenerated())
	{
		m_generatedAnnotations.insert(annotation);

		if (annotation->isDynamicVisualization())
		{
			m_dynamicAnnotations.insert(annotation);
		}
	}
	else
	{
		m_notGeneratedAnnotations.insert(annotation);
	}

	//let the annnotation notify to this container
	annotation->m_container = this;
}

void PatchAnnotations::generateAnnotations()
{
	AnnotationsSet::iterator itSafe = m_notGeneratedAnnotations.begin();

	while(itSafe != m_notGeneratedAnnotations.end() )
	{
		//get the current iterator and step the safe iterator to the next item.
		//this allow us to remove the current iterator from the list.
		AnnotationsSet::iterator it = itSafe;
		++itSafe;

		//try to create visualization
		if ((*it)->canGenerateVisualization())
		{
			(*it)->generateVisualization();

			(*it)->validateElevation();
		}

		//if we generate visualization...
		if ((*it)->wasVisualizationGenerated())
		{				
			//add it to generated list
			m_generatedAnnotations.insert(*it);
			
			if ((*it)->isDynamicVisualization())
			{
				//add it to the dynamic list
				m_dynamicAnnotations.insert(*it);
			}

			//remove from not generated
			m_notGeneratedAnnotations.erase(it);
		}
	}
}

void PatchAnnotations::updateAnnotations()
{
	AnnotationsSet::iterator itSafe = m_dynamicAnnotations.begin();

	while(itSafe != m_dynamicAnnotations.end() )
	{
		//get the current iterator and step the safe iterator to the next item.
		//this allow us to remove the current iterator from the list.
		AnnotationsSet::iterator it = itSafe;
		++itSafe;

		if ((*it)->isDynamicVisualization())
		{
			(*it)->updateVisualization();
		} 
		else
		{
			m_dynamicAnnotations.erase(it);
		}
	}
}

void PatchAnnotations::destroyAnnotations()
{
	AnnotationsSet::iterator itSafe = m_generatedAnnotations.begin();

	while(itSafe != m_generatedAnnotations.end() )
	{
		//get the current iterator and step the safe iterator to the next item.
		//this allow us to remove the current iterator from the list.
		AnnotationsSet::iterator it = itSafe;
		++itSafe;

		(*it)->destroyVisualization();

		//this annotation is no longer contained
		(*it)->m_container = NULL;
	}

	m_generatedAnnotations.clear();
	m_dynamicAnnotations.clear();
	m_notGeneratedAnnotations.clear();
}


void PatchAnnotations::validateElevation()
{
	AnnotationsSet::iterator itSafe = m_generatedAnnotations.begin();

	while(itSafe != m_generatedAnnotations.end() )
	{
		(*itSafe)->validateElevation();
		++itSafe;
	}
}

void PatchAnnotations::removeAllAnnotationOfPipeline(const ProcRef & procRef)
{
	//destory generated annotations
	{
		AnnotationsSet::iterator itSafe = m_generatedAnnotations.begin();

		while(itSafe != m_generatedAnnotations.end() )
		{
			//get the current iterator and step the safe iterator to the next item.
			//this allow us to remove the current iterator from the list.
			AnnotationsSet::iterator it = itSafe;
			++itSafe;

			if ((*it)->getProcRef() == procRef)
			{
				(*it)->destroyVisualization();

				//this annotation is no longer contained
				(*it)->m_container = NULL;

				m_dynamicAnnotations.erase(*it);
				m_generatedAnnotations.erase(it);
			}
		}
	}

	//destory not generated annotations
	{
		AnnotationsSet::iterator itSafe = m_notGeneratedAnnotations.begin();

		while(itSafe != m_notGeneratedAnnotations.end() )
		{
			//get the current iterator and step the safe iterator to the next item.
			//this allow us to remove the current iterator from the list.
			AnnotationsSet::iterator it = itSafe;
			++itSafe;

			if ((*it)->getProcRef() == procRef)
			{
				m_notGeneratedAnnotations.erase(it);
			}
		}
	}
}

int PatchAnnotations::getAnnotationsCount()
{
	return m_generatedAnnotations.size() + m_notGeneratedAnnotations.size();
}

//////////////////////////////////////////////////////////////////
// AnnotationCache
//////////////////////////////////////////////////////////////////


AnnotationCache::Node::Node() : parent(0),maxChild(0),depth(0)
{
	for(int i=0;i<7;i++)
	{
		subNodes[i] = 0;
	}
}

AnnotationCache::Node::Node(int aDepth) : parent(0),maxChild(0),depth(aDepth)
{
	for(int i=0;i<7;i++)
	{
		subNodes[i] = 0;
	}
}

AnnotationCache::Node::~Node()
{
	for(int i=0;i<7;i++)
	{
		delete subNodes[i];
	}
}

AnnotationCache::Node * AnnotationCache::Node::getNode(const PYXIndex & index,bool autoGenerate)
{
	return getNode(index,0,autoGenerate);
}

AnnotationCache::Node * AnnotationCache::Node::getNode(const PYXIndex & index,int digitIndex,bool autoGenerate)
{
	if (index.getDigitCount()==digitIndex)
	{
		return this;
	}
	int digit = index.getDigit(digitIndex);

	if (subNodes[digit] == 0)
	{
		if (autoGenerate)
		{
			subNodes[digit] = new Node(depth+1);
			subNodes[digit]->parent = this;
		}
		else
		{
			//try to check if our requested index is our centroid child, if so, return this node.			
			digitIndex++;
			while(digitIndex < index.getDigitCount() && index.getDigit(digitIndex)== 0)
			{
				digitIndex++;
			}
			if (digitIndex == index.getDigitCount())
			{
				return this;
			}

			//else - return
			return NULL;
		}
	}
	return subNodes[digit]->getNode(index,digitIndex+1,autoGenerate);
}

bool AnnotationCache::Node::operator>(const AnnotationCache::Node & node) const
{
	if (depth == node.depth)
	{
		return annotations.front().m_weight > node.annotations.front().m_weight;
	}
	return depth < node.depth;
}

void AnnotationCache::Node::addAnnotation(const PYXIndex & index,int topResolutionDelta, AnnotationInfo & info)
{
	getNode(index)->addAnnotation(topResolutionDelta,info);
}

void AnnotationCache::Node::addAnnotation(int topResolutionDelta,AnnotationInfo & info)
{
	bool propogateUp = false;

	if (annotations.size() == 0)
	{
		annotations.push_back(info);
		propogateUp = true;
	}
	else if (annotations.front().m_weight < info.m_weight )
	{
		annotations.push_front(info);
		propogateUp = true;
	}
	else
	{
		std::list<AnnotationCache::AnnotationInfo>::iterator it = annotations.begin();
		while (it != annotations.end() && it->m_weight >= info.m_weight )
		{
			it++;
		}
		annotations.insert(it,info);
	}

	Node * nodeParent = this->parent;


	if (propogateUp)
	{
		int propogateCount =0;
		while (nodeParent != 0 && propogateCount < topResolutionDelta)
		{
			if (nodeParent->maxChild == 0)
			{
				nodeParent->maxChild = this;
			}
			else if (nodeParent->maxChild == this)
			{
				//great, keep going up
			}
			else if (*this > *(nodeParent->maxChild)) //compare depth, and the compare annotations weight
			{
				nodeParent->maxChild = this;
			}
			else
			{
				break;
			}
			nodeParent = nodeParent->parent;
			propogateCount++;
		}
	}
}

const std::list<AnnotationCache::AnnotationInfo> & AnnotationCache::Node::getAnnotation(const PYXIndex & index) 
{
	Node * node = getNode(index,false);

	if (node == 0)
	{
		return s_emptyAnnotations;
	}

	if (node->annotations.size()==0)
	{
		if (node->maxChild != 0)
		{
			return node->maxChild->annotations;
		}
	}
	return node->annotations;
}

std::list<AnnotationCache::AnnotationInfo> AnnotationCache::Node::s_emptyAnnotations;

//! Tester class.
Tester<AnnotationCache> gTester;

void AnnotationCache::test()
{
	AnnotationCache cache;

	cache.addAnnotation(PYXIcosIndex("A-00100"),PYXIcosIndex("A"),AnnotationInfo("A",PYXCoord3DDouble()));

	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00100")).size() == 1);
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00")).size() == 1);

	cache.addAnnotation(PYXIcosIndex("A-00010"),PYXIcosIndex("A"),AnnotationInfo("B",PYXCoord3DDouble()));

	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00100")).size() == 1);
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00001")).size() == 0);
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00010")).size() == 1 && cache.getAnnotation(PYXIcosIndex("A-00010")).front().m_featureID == "B");

	cache.addAnnotation(PYXIcosIndex("A-00010"),PYXIcosIndex("A"),AnnotationInfo("B2",PYXCoord3DDouble()));

	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00")).size() == 1 && cache.getAnnotation(PYXIcosIndex("A-00")).front().m_featureID == "A");	
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00010")).size() == 2 && cache.getAnnotation(PYXIcosIndex("A-00010")).front().m_featureID == "B");

	cache.addAnnotation(PYXIcosIndex("A-00010"),PYXIcosIndex("A"),AnnotationInfo("C",PYXCoord3DDouble(),1));

	//check if the weight as change the order between B can C
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00010")).size() == 3 && cache.getAnnotation(PYXIcosIndex("A-00010")).front().m_featureID == "C");
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-001")).size() == 1 && cache.getAnnotation(PYXIcosIndex("A-001")).front().m_featureID == "A");	
	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("A-00")).size() == 3 && cache.getAnnotation(PYXIcosIndex("A-00")).front().m_featureID == "C");	


	cache.addAnnotation(PYXIcosIndex("1-00040"),PYXIcosIndex("A"),AnnotationInfo("D",PYXCoord3DDouble()));
	cache.addAnnotation(PYXIcosIndex("1-00040"),PYXIcosIndex("A"),AnnotationInfo("E",PYXCoord3DDouble()));

	TEST_ASSERT(cache.getAnnotation(PYXIcosIndex("1-00040")).size() == 2 && cache.getAnnotation(PYXIcosIndex("1-00040")).front().m_featureID == "D");
}