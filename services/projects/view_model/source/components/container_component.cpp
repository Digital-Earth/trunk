/******************************************************************************
container_component.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "container_component.h"
#include "view_open_gl_thread.h"


ContainerComponent::ContainerComponent(ViewOpenGLThread & thread) : Component(thread)
{
}

ContainerComponent::~ContainerComponent()
{
}


bool ContainerComponent::initialize()
{	
	return true;
}

void ContainerComponent::releaseOpenGLResources()
{
	for(ComponentsVector::iterator it = m_children.begin();it != m_children.end();++it)
	{
		(*it)->releaseOpenGLResources();
	}

}


void ContainerComponent::render()
{
	for(ComponentsVector::iterator it = m_children.begin();it != m_children.end();++it)
	{
		if ((*it)->isVisible())
		{
			(*it)->render();
		}
	}
}


void ContainerComponent::pickAnnotations(const Ray & ray,PickedIAnnotationVector & resultVector)
{
	for(ComponentsVector::iterator it = m_children.begin();it != m_children.end();++it)
	{
		if ((*it)->isVisible() && (*it)->isEnabled())
		{
			(*it)->pickAnnotations(ray,resultVector);
		}
	}
}

void ContainerComponent::addChild(PYXPointer<Component> component)
{
	ComponentsVector::iterator it = std::find(m_children.begin(),m_children.end(),component);

	if (it == m_children.end())
	{
		if (component->getParent())
		{
			component->getParent()->removeChild(component);
		}

		m_children.push_back(component);
		
		component->setParent(this);
	}
}

void ContainerComponent::removeChild(PYXPointer<Component> component)
{
	ComponentsVector::iterator it = std::find(m_children.begin(),m_children.end(),component);

	if (it != m_children.end())
	{
		m_children.erase(it);

		component->setParent(NULL);
	}
}

void ContainerComponent::clearChildren()
{
	ComponentsVector::iterator it = m_children.begin();

	while (it != m_children.end())
	{
		(*it)->setParent(NULL);
		++it;
	}
	m_children.clear();
}

PYXPointer<UIMouseEvent> ContainerComponent::propogateMouseEvent(PYXPointer<UIMouseEvent> event)
{
	if (getParent())
	{
		return getParent()->propogateMouseEvent(event);
	}
	return event;
}

void ContainerComponent::onMouseClick(PYXPointer<UIMouseEvent> event)
{	
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseClick(event);

			if (event->isConsumed())
			{
				return;
			}
		}		
	}
}

void ContainerComponent::onMouseDoubleClick(PYXPointer<UIMouseEvent> event)
{	
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseDoubleClick(event);

			if (event->isConsumed())
			{
				return;
			}
		}		
	}	
}

void ContainerComponent::onMouseMove(PYXPointer<UIMouseEvent> event)
{
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseMove(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}
}

void ContainerComponent::onMouseUp(PYXPointer<UIMouseEvent> event)
{	
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseUp(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}	
}

void ContainerComponent::onMouseDown(PYXPointer<UIMouseEvent> event)
{	
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseDown(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}
}

void ContainerComponent::onMouseWheel(PYXPointer<UIMouseEvent> event)
{
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onMouseWheel(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}	
}

void ContainerComponent::onKeyPressed(PYXPointer<UIKeyEvent> event)
{
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onKeyPressed(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}
}

void ContainerComponent::onKeyDown(PYXPointer<UIKeyEvent> event)
{
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onKeyDown(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}
}

void ContainerComponent::onKeyUp(PYXPointer<UIKeyEvent> event)
{
	for(unsigned int i=m_children.size();i>0;i--)
	{
		Component & child = *m_children[i-1];
		if (child.isVisible() && child.isEnabled())
		{
			child.onKeyUp(event);

			if (event->isConsumed())
			{
				return;
			}
		}
	}	
}
