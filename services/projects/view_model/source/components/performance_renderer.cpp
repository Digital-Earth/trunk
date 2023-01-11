/******************************************************************************
performance_renderer.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "performance_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"


bool PerformanceRenderer::initialize()
{
	return true;
}

PerformanceRenderer::PerformanceRenderer(ViewOpenGLThread & viewThread) : Component(viewThread)
{	
}

PerformanceRenderer::~PerformanceRenderer(void)
{
	forgetPendingRequests(m_texturePacker);
}

void PerformanceRenderer::render()
{
	getViewThread().setState("render Performance");

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	std::string text;
	int mouseX = getViewThread().getMouseX();
	int mouseY = getViewThread().getMouseY();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	glColor3f(5.0f,0.0f,0.0f);

	glBegin(GL_LINE_STRIP);
	glVertex2i(viewport[2]-PerformanceCounter::knMesurmentsHistorySize,333);
	glVertex2i(viewport[2],333);
	glEnd();

	glColor3f(5.0f,5.0f,0.0f);

	glBegin(GL_LINE_STRIP);
	glVertex2i(viewport[2]-PerformanceCounter::knMesurmentsHistorySize,222);
	glVertex2i(viewport[2],222);
	glEnd();

	glColor3f(0.0f,5.0f,0.0f);

	glBegin(GL_LINE_STRIP);
	glVertex2i(viewport[2]-PerformanceCounter::knMesurmentsHistorySize,111);
	glVertex2i(viewport[2],111);
	glEnd();

	glColor3f(1.0f,1.0f,1.0f);


	{
		std::map<std::string,PYXPointer<TimePerformanceCounter>>::iterator it = PerformanceCounter::m_timePerformanceCounters.begin();

		while (it != PerformanceCounter::m_timePerformanceCounters.end())
		{	
			TimePerformanceCounter & counter = *(it->second);
			
			glColor3f(counter.m_color[0],counter.m_color[1],counter.m_color[2]);

			bool hit = false;
			int value = 0;

			glBegin(GL_LINE_STRIP);
			for(int j=0;j<PerformanceCounter::knMesurmentsHistorySize-1;j++)
			{
				float x = static_cast<float>(viewport[2]-PerformanceCounter::knMesurmentsHistorySize+j);
				float y = static_cast<float>(counter.getMeasurement(j))/counter.getSampleRatio()*10000.0f;

				if ( abs(mouseX - x) < 2 && abs(mouseY - y) < 10 )
				{
					hit = true;	
					value = counter.getMeasurement(j);
				}
				glVertex2f(x,y);
			}
			glEnd();

			if (hit)
			{
				text += it->first + "(" + StringUtils::toString(value) + ") ";
			}

			++it;
		}
	}

	{
		std::map<std::string,PYXPointer<ValuePerformanceCounter>>::iterator it = PerformanceCounter::m_valuePerformanceCounters.begin();

		while (it != PerformanceCounter::m_valuePerformanceCounters.end())
		{	
			ValuePerformanceCounter & counter = *(it->second);
			
			glColor3f(counter.m_color[0],counter.m_color[1],counter.m_color[2]);

			bool hit=false;
			int  value = 0;

			glBegin(GL_LINE_STRIP);
			for(int j=0;j<PerformanceCounter::knMesurmentsHistorySize-1;j++)
			{
				float x = static_cast<float>(viewport[2]-PerformanceCounter::knMesurmentsHistorySize*2-10+j);
				float y = static_cast<float>(counter.getMeasurement(j))/counter.getSampleRatio();

				//make sure values are in displayable area...
				if (y>800)
				{
					y=800;
				}

				//make some room for error
				if ( abs(mouseX - x) < 2 && abs(mouseY - y) < 10 )
				{
					hit = true;
					value = counter.getMeasurement(j);
				}

				glVertex2f(x,y);
			}
			glEnd();

			if (hit)
			{
				text += it->first + "(" + StringUtils::toString(value) + ") ";
			}

			++it;
		}
	}	

//#ifdef INSTANCE_COUNTING
	InstanceCounter::InstanceCountMap snapshot = InstanceCounter::takeSnapShot();
	
	glColor3f(1.0f,1.0f,1.0f);
	int y = 0;
	int maxLineLength = viewport[2]-PerformanceCounter::knMesurmentsHistorySize*4;
	size_t maxValue = maxLineLength;
	for (InstanceCounter::InstanceCountMap::iterator it = snapshot.begin(); it != snapshot.end(); ++it)
	{
		if (maxValue < it->second)
		{
			maxValue = it->second;
		}
	}

	for (InstanceCounter::InstanceCountMap::iterator it = snapshot.begin(); it != snapshot.end(); ++it)
	{
		if (it->second > 0)
		{
			glBegin(GL_LINE_STRIP);
			glVertex2i(5,viewport[3]-10-y*3);
			int maxX = (int)(6+maxLineLength*(it->second)/maxValue);
			glVertex2i(maxX,viewport[3]-10-y*3);
			glEnd();

			if ( mouseX < maxX && abs(mouseY - (viewport[3]-10-y*3)) < 2 )
			{
				if (it->first.size()>20)
				{
					std::string shortName = it->first;
					shortName.resize(20);
					shortName += "...";
					text += shortName + " " + StringUtils::toString(it->second);
				}
				else
				{
					text += it->first + " " + StringUtils::toString(it->second);
				}
			}
			++y;
		}
	}
//#endif

	if (text.length()>0)
	{
		if (text.length() > 100)
		{
			text = text.substr(0,97) + "...";
		}

		//create new text texture
		if (m_text != text)
		{
			m_currentText = getTextureFromBitmapDefinition(m_texturePacker,"<Text>" + XMLUtils::toSafeXMLText(text) + "</Text>");
			
			if (m_currentText)
			{
				forgetPendingRequests(m_texturePacker);
				m_text = text;
			}
		}

		if (m_currentText)
		{
			if (m_currentText->getWidth()/2 > mouseX)
			{
				m_currentText->draw(vec3(0+m_currentText->getWidth()/2,mouseY,0),vec3(1,0,0),vec3(0,1,0));
			}
			else
			{
				m_currentText->draw(vec3(mouseX,mouseY,0),vec3(1,0,0),vec3(0,1,0));
			}
		}
		
	}
}
