/******************************************************************************
animation.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "animation.h"

#include "pyxis/utility/trace.h"



Animation::Animation(void) : m_constant(false)
{
	//add this object into the AnimationUpdater list
	{
		boost::recursive_mutex::scoped_lock lock(AnimationUpdater::m_listMutex);
		AnimationUpdater::getInstance().m_animations.insert(this);		
	}
}

Animation::~Animation(void)
{
	//remove this object from the AnimationUpdater list. Note, if this object was declared constant - we don't need to do so.
	if (! m_constant)
	{
		boost::recursive_mutex::scoped_lock lock(AnimationUpdater::m_listMutex);
		AnimationUpdater::getInstance().m_animations.erase(this);
	}
}

void Animation::setConstant(bool constant)
{	
	m_constant = constant;
}


/////////////////////////////////////////////////////////////////////////////////////////

boost::recursive_mutex AnimationUpdater::m_listMutex;

AnimationUpdater::AnimationUpdater()
{
	//timer reset
	m_lastTime = boost::posix_time::microsec_clock::local_time();
}

AnimationUpdater::~AnimationUpdater()
{
}

void AnimationUpdater::update(double speed)
{	
	//lock the animations list
	boost::recursive_mutex::scoped_lock lock(AnimationUpdater::m_listMutex);

	//check time delta
	boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = currentTime - m_lastTime;
	int msec = static_cast<int>(td.total_milliseconds()*speed);
	m_lastTime = currentTime;

	//all animations objects that become constant after this update
	AnimationSet constantAnimations;

	AnimationSet::iterator it = m_animations.begin();	
	while(it != m_animations.end())
	{	
		//update the animation object
		(**it).update(msec);

		//check if it's constant
		if ((**it).m_constant)
		{
			constantAnimations.insert(*it);
		}

		//next!
		++it;
	}

	//remove all constant animations from the animations list
	it = constantAnimations.begin();

	while(it != constantAnimations.end())
	{	
		m_animations.erase(*it);
		++it;
	}

	//we can release the animation list
}

AnimationUpdater * AnimationUpdater::m_instance = NULL;

// TODO: Make this thread-safe.  See ReferenceSphere::getInstance() as an example.
AnimationUpdater & AnimationUpdater::getInstance()
{
	if (AnimationUpdater::m_instance == NULL)
	{
		AnimationUpdater::m_instance = new AnimationUpdater();
	}
	return *AnimationUpdater::m_instance;
}