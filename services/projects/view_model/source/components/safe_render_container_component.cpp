/******************************************************************************
safe_render_container_component.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "safe_render_container_component.h"
#include "view_open_gl_thread.h"


SafeRenderContainerComponent::SafeRenderContainerComponent(ViewOpenGLThread & thread) : ContainerComponent(thread)
{
}

SafeRenderContainerComponent::~SafeRenderContainerComponent()
{
}


void SafeRenderContainerComponent::render()
{
	for(unsigned int i=0;i<m_children.size();i++)
	{
		if (m_children[i]->isVisible())
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);	
		
			try
			{
				m_children[i]->render();
			}
			catch(PYXException& ex)
			{
				std::string className = typeid(*m_children[i]).name();
				TRACE_ERROR("Exception occured while Rendering (" + className + "): " + ex.getFullErrorString());
			}
			catch(...)
			{
				std::string className = typeid(*m_children[i]).name();
				TRACE_ERROR("Exception occured while Rendering (" + className + "). ");
			}

			glPopAttrib();
		}		
	}
}


HotKeySafeRenderContainerComponent::HotKeySafeRenderContainerComponent(ViewOpenGLThread & thread) : SafeRenderContainerComponent(thread)
{
};

HotKeySafeRenderContainerComponent::~HotKeySafeRenderContainerComponent ()
{
}

void HotKeySafeRenderContainerComponent::onKeyPressed(PYXPointer<UIKeyEvent> event)
{
	//first, let children handle the key pressed event
	SafeRenderContainerComponent::onKeyPressed(event);

	//now, check if a HotKey was pressed
	if (! event->isConsumed())
	{
		char chr = event->getKeyChar();
		if (m_hotKeyMap.find(chr) != m_hotKeyMap.end())
		{
			event->setConsumed();
			m_hotKeyNotifier.notify(HotKeyEvent::create(chr,m_hotKeyMap[chr]));		
		}
	}
}

void HotKeySafeRenderContainerComponent::addHotKey(const char & chr, PYXPointer<Component> component)
{
	m_hotKeyMap[chr] = component;
}

void HotKeySafeRenderContainerComponent::removeHotKey(const char & chr)
{
	m_hotKeyMap.erase(chr);
}
