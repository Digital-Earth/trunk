#pragma once
#ifndef VIEW_MODEL__ANIMATION_H
#define VIEW_MODEL__ANIMATION_H
/******************************************************************************
open_gl_animation.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis\utility\object.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <set>

/*!

Animation class - base class for animating values smoothly. 

the Animation class is a virtual utility class for creating animation. Its responsibility is to update the Animation value in a millisecond resolution.
the Animation class should not be created, only derived class should be created.

The animation class has a virtual method "update" that is been called by the AnimationUpdater every frame (View.cpp is responsible for that).
However, in order to support huge number of animation values, the AnimationUpdater will stop call the "update" for an animation once it declare itself constant.
Animation object can call setConstant(true) to declare it self as constant.

Note, the Animation class is added automatically to the AnimationUpdater on creation and removed on destruction.

*/

//! virtual base class for animating values smoothly. 
class Animation : public PYXObject
{
	friend class AnimationUpdater;
private:
	bool m_constant;

protected:
	Animation(void); 
	virtual ~Animation(void);

//protected assigment and copy construtor
private:
	Animation(const Animation & animation) {};
	Animation & operator=(const Animation & animation) {};


public:
	//! called by Animation Updater. millisecond is the time passed between the last update call
	virtual void update(int millisecond) {};

	//! set the Animation constant object
	void setConstant(bool constant);

	const bool & isConstant() const { return m_constant; }
};


/*!
LinearAnimation<T> - create a linear value animation between two values

LinearAnimation is creating an animation between "startValue" and "endValue". 
The animation would take "period" in milliseconds. After the period, the animation would become constant.
Optional parameter is "start" that would delay the animation in given amount of milliseconds.

Use the static "create" method to create LinearAnimations.
*/

//! LinearAnimation<T> - create a linear value animation between two values
template<typename T>
class LinearAnimation : public Animation
{
protected:
	T m_startValue;
	T m_endValue;
	int m_start;
	int m_period;

	//! current value
	T m_value;

	//! current position of the animation
	int m_position;

public:
	//! create an LinearAnimation object
	LinearAnimation(T startValue,T endValue,int period) : 
		m_startValue(startValue), 
		m_endValue(endValue),
		m_start(0), 
		m_period(period), 
		m_value(startValue), 
		m_position(0) 
		{};
	
	//! create an LinearAnimation object with delay
	LinearAnimation(T startValue,T endValue,int period,int start) : 
		m_startValue(startValue), 
		m_endValue(endValue), 
		m_start(start), 
		m_period(period), 
		m_value(startValue), 
		m_position(0)
		{};

	virtual void update(int millisecond)
	{
		//update the position
		m_position+=millisecond;

		//check if animation begans
		if (m_position > m_start)
		{
			//check if animation ends.
			if (m_position > m_start+m_period)
			{
				//set as the value as end value and declare this as constant 
				m_value = m_endValue;
				setConstant(true);
				return;
			}

			//interpulate
			m_value = static_cast<T>((m_position-m_start)*(m_endValue-m_startValue)/m_period + m_startValue );
		}
	}

	static PYXPointer<LinearAnimation> create(T startValue,T endValue,int period) {
		return PYXNEW(LinearAnimation,startValue,endValue,period);
	}
	static PYXPointer<LinearAnimation> create(T startValue,T endValue,int period,int start) {
		return PYXNEW(LinearAnimation,startValue,endValue,period,start);
	}

	//! retrive the current animation value
	const T & getValue() { return m_value; };
};


/*!
LinearWithEasingAnimation<T> - create a linear value animation between two values with easing

LinearAnimation is creating an animation between "startValue" and "endValue". 
The animation would take "period" in milliseconds. After the period, the animation would become constant.
The Easing factor define the a "slowing" to the betining and the end of the animation. 
Easing factor = 1 - no easing. the animation is linear.
Easing factor = 2 - square easing. the function start like time^2. and end like sqrt(time)
Optional parameter is "start" that would delay the animation in given amount of milliseconds.

Use the static "create" method to create LinearAnimations.
*/

//! LinearWithEasingAnimation<T> - create a linear value animation between two values with easing
template<typename T>
class LinearWithEasingAnimation : public Animation
{
protected:
	T m_startValue;
	T m_endValue;
	int m_start;
	int m_period;
	double m_easing;

	//! current value
	T m_value;

	//! current position of the animation
	int m_position;

public:
	//! create an LinearWithEasingAnimation object
	LinearWithEasingAnimation(T startValue,T endValue,int period,double easing) : 
		m_startValue(startValue), 
		m_endValue(endValue),
		m_easing(easing),
		m_start(0), 
		m_period(period), 
		m_value(startValue), 
		m_position(0) 
		{};
	
	//! create an LinearWithEasingAnimation object with delay
	LinearWithEasingAnimation(T startValue,T endValue,int period,double easing, int start) : 
		m_startValue(startValue), 
		m_endValue(endValue), 
		m_easing(easing),
		m_start(start), 
		m_period(period), 
		m_value(startValue), 
		m_position(0)
		{};

	virtual void update(int millisecond)
	{
		//update the position
		m_position+=millisecond;

		//check if animation begans
		if (m_position > m_start)
		{
			//check if animation ends.
			if (m_position > m_start+m_period)
			{
				//set as the value as end value and declare this as constant 
				m_value = m_endValue;
				setConstant(true);
				return;
			}

			//interpulate
			double timeOffset = static_cast<double>(m_position-m_start)/m_period;

			if (timeOffset < 0.5)
			{
				//easing in
				timeOffset = pow(timeOffset*2,m_easing)/2;
			}
			else
			{
				//easing out
				timeOffset = 1 - pow(2-timeOffset*2,m_easing)/2;
			}

			m_value = static_cast<T>((m_endValue-m_startValue)*timeOffset + m_startValue );
		}
	}

	static PYXPointer<LinearWithEasingAnimation> create(T startValue,T endValue,int period,double easing) {
		return PYXNEW(LinearWithEasingAnimation,startValue,endValue,period,easing);
	}
	static PYXPointer<LinearWithEasingAnimation> create(T startValue,T endValue,int period,double easing,int start) {
		return PYXNEW(LinearWithEasingAnimation,startValue,endValue,period,easing,start);
	}

	//! retrive the current animation value
	const T & getValue() { return m_value; };
};


/*!
AnimationUpdater - resposible to update all Animations objects every frame
*/
//! AnimationUpdater - resposible to update all Animations objects every frame
class AnimationUpdater
{
	friend class Animation;

private:
	//! threadsafe mutex
	static boost::recursive_mutex m_listMutex;

	//! singleton
	static AnimationUpdater * m_instance;

protected:
	//! set of all animations (Note: we dont' keep a "reference" to the Animation objects)
	typedef std::set<Animation*> AnimationSet;
	AnimationSet m_animations;
	//! last frame
	boost::posix_time::ptime m_lastTime;

	AnimationUpdater();
	virtual ~AnimationUpdater();

public:
	void update(double speed=1.0);

	static AnimationUpdater & getInstance();
	
};

#endif