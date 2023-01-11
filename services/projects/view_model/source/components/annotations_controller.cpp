/******************************************************************************
annotations_controller.cpp

begin		: 2010-March-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "annotations_controller.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"
#include "performance_counter.h"

#include "pyxis/utility/bitmap_server_provider.h"
#include "pyxis/utility/xml_utils.h"
#include "pyxis/utility/app_services.h"

#include <boost/scoped_array.hpp>

#include <cassert>
#include <map>
#include <vector>



AnnotationsController::AnnotationsController(Component & rootComponent) : 
	Component(rootComponent.getViewThread()),
	m_rootComponent(rootComponent)
{
}

AnnotationsController::~AnnotationsController(void)
{
}

void AnnotationsController::render()
{
	PerformanceCounter::getTimePerformanceCounter("Start Picking Annotations",0.5f,0.5f,0.5f)->makeMeasurement();
	//perform a ray picking...
	m_pickedAnnotations.clear();
	try 
	{
		m_rootComponent.pickAnnotations(getViewThread().getMouseRay(),m_pickedAnnotations);
	}	
	catch(PYXException& ex) 
	{
		TRACE_ERROR("Failed to pick annotations with error: " << ex.getFullErrorString());
	}
	catch(std::exception& ex) 
	{
		TRACE_ERROR("Failed to pick annotations with error: " << ex.what());
	}
	catch(...) 
	{
		TRACE_ERROR("Failed to pick annotations with error: " << "unknown");
	}

	std::sort(m_pickedAnnotations.begin(),m_pickedAnnotations.end());

	PerformanceCounter::getTimePerformanceCounter("End AnnotationsController",0.5f,0.5f,0.5f)->makeMeasurement();

	getViewThread().setFrameTimeMeasurement("mouse-pick");
}

void AnnotationsController::onMouseMove(PYXPointer<UIMouseEvent> event)
{
	//PickedIAnnotationVector picked;
	//pickAnnotations(getViewThread().getMouseRay(),picked);

	//TODO: sort the picked annotation vector...
	
	if (m_pickedAnnotations.size()>0)
	{
		PYXPointer<IAnnotation> firstAnnotation = m_pickedAnnotations[0].second;
		Annotation * currentAnnotation = dynamic_cast<Annotation*>(m_currentAnnotation.get());

		//did we switch annotation?
		if (firstAnnotation != m_currentAnnotation || ! m_currentAnnotation )
		{
			//Trigger mouse leave
			if (m_currentAnnotation)
			{
				PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

				//let the currentAnnotation to handle the event.
				if (currentAnnotation != NULL)
				{
					currentAnnotation->onMouseLeave(annotationEvent);
				}

				//notify the view about this event
				getViewThread().getView().getAnnotationMouseLeaveNotifier().notify(annotationEvent);
			}			

			m_currentAnnotation = firstAnnotation;
			currentAnnotation = dynamic_cast<Annotation*>(m_currentAnnotation.get());

			//Trigger mouse enter
			{
				PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

				//let the currentAnnotation to handle the event.
				if (currentAnnotation != NULL)
				{
					currentAnnotation->onMouseEnter(annotationEvent);
				}

				//notify the view about this event
				getViewThread().getView().getAnnotationMouseEnterNotifier().notify(annotationEvent);
			}
		}

		//Trigger mouse move
		PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

		//let the currentAnnotation to handle the event.
		if (currentAnnotation != NULL)
		{
			currentAnnotation->onMouseMove(annotationEvent);
		}

		//notify the view about this event
		getViewThread().getView().getAnnotationMouseMoveNotifier().notify(annotationEvent);

		if (annotationEvent->isConsumed())
		{
			event->setConsumed();
		}
	}
	else if (m_currentAnnotation)
	{
		Annotation * currentAnnotation = dynamic_cast<Annotation*>(m_currentAnnotation.get());

		PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

		//let the currentAnnotation to handle the event.
		if (currentAnnotation != NULL)
		{
			currentAnnotation->onMouseLeave(annotationEvent);
		}

		//notify the view about this event
		getViewThread().getView().getAnnotationMouseLeaveNotifier().notify(annotationEvent);

		m_currentAnnotation = NULL;
	}
}


void AnnotationsController::onMouseClick(PYXPointer<UIMouseEvent> event)
{
	if (m_currentAnnotation)
	{
		Annotation * currentAnnotation = dynamic_cast<Annotation*>(m_currentAnnotation.get());

		PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

		//let the currentAnnotation to handle the event.
		if (currentAnnotation != NULL)
		{
			currentAnnotation->onMouseClick(annotationEvent);
		}

		//notify the view about this event
		getViewThread().getView().getAnnotationClickNotifier().notify(annotationEvent);

		if (annotationEvent->isConsumed())
		{
			event->setConsumed();
		}
	}
}

void AnnotationsController::onMouseDoubleClick(PYXPointer<UIMouseEvent> event)
{
	if (m_currentAnnotation)
	{
		Annotation * currentAnnotation = dynamic_cast<Annotation*>(m_currentAnnotation.get());

		PYXPointer<AnnotationMouseEvent> annotationEvent = AnnotationMouseEvent::create(event,m_currentAnnotation,getViewThread().getViewHandle());

		//let the currentAnnotation to handle the event.
		if (currentAnnotation != NULL)
		{
			currentAnnotation->onMouseDoubleClick(annotationEvent);
		}

		//notify the view about this event
		getViewThread().getView().getAnnotationDoubleClickNotifier().notify(annotationEvent);

		if (annotationEvent->isConsumed())
		{
			event->setConsumed();
		}
	}
}