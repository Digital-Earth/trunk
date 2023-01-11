/******************************************************************************
rhombus_bitmap.cpp

begin		: 2011-02-01
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "rhombus_bitmap.h"
#include "performance_counter.h"
#include "pyxis/utility/profile.h"

///////////////////////////////////////////////////////////////////////
// RhombusRGBA
///////////////////////////////////////////////////////////////////////

RhombusRGBA::RhombusRGBA(ResolutionType resolutionType)
	: 	m_allOpaque(false),
		m_allTransparent(false),
		m_type(resolutionType)
{
	m_buffers.reset(new RGBABuffer[getBufferCount()]);
}

RhombusRGBA::RhombusRGBA(const RhombusRGBA & other)
	: 	m_allOpaque(other.m_allOpaque),
		m_allTransparent(other.m_allTransparent),
		m_type(other.m_type)
{
	m_buffers.reset(new RGBABuffer[getBufferCount()]);
	for(int b=0;b<getBufferCount();b++)
	{
		memcpy(&getBuffer(b),&other.getBuffer(b),sizeof(RGBABuffer));
	}
}

int RhombusRGBA::getBufferCount() const
{
	if (m_type==knOddResolution)
	{
		return 3;
	}
	else
	{
		return 1;
	}
}

const RhombusRGBA::RGBABuffer & RhombusRGBA::getBuffer(int bufferIndex) const
{
	assert(bufferIndex < getBufferCount() && "BufferIndex is out of range");
	assert(m_buffers.get() != 0 && "buffers were not allocated");
	return m_buffers[bufferIndex];
}


RhombusRGBA::RGBABuffer & RhombusRGBA::getBuffer(int bufferIndex)
{
	assert(bufferIndex < getBufferCount() && "BufferIndex is out of range");
	assert(m_buffers.get() != 0 && "buffers were not allocated");
	return m_buffers[bufferIndex];
}

unsigned char * RhombusRGBA::getPixel(int u,int v,int bufferIndex)
{
	assert(u >=0 && u < width && v >= 0 && v < height && "UV is out of range");
	if (bufferIndex>0)
	{
		assert(u < width-1 && v < height-1 && "UV is out of range");
	}
	return getBuffer(bufferIndex).rgba[v][u];
}

unsigned char * RhombusRGBA::getPixel(double u,double v)
{
	assert(u >=0 && u <= width-1 && v >= 0 && v <= height-1 && "UV is out of range");

	int finalU = static_cast<int>(floor(u));
	int finalV = static_cast<int>(floor(v));
	int bufferIndex = 0;

	if (m_type == knEvenResolution)
	{
		//make u,v between [0,1)
		u-=finalU;
		v-=finalV;
		if (u>v)
		{
			if (u<0.5-v*0.5)
			{
			}
			else if (v>1.0-u*0.5)
			{
				finalU++;
				finalV++;
			}
			else
			{
				finalU++;
			}
		}
		else
		{
			if (v<0.5-u*0.5)
			{
			}
			else if (u>1.0-v*0.5)
			{
				finalU++;
				finalV++;
			}
			else
			{
				finalV++;
			}
		}
	}
	else //m_type == knOddResolution
	{
		//make u,v between [0,1)
		u-=finalU;
		v-=finalV;
		u*=3;
		v*=3;

		int subU = static_cast<int>(floor(u));
		int subV = static_cast<int>(floor(v));

		if (subU == subV)
		{
			double sum = u+v;
			if (sum>5-DBL_EPSILON)
			{
				finalU++;
				finalV++;
			}
			else if (sum>3+DBL_EPSILON)
			{
				bufferIndex = 2;
			}
			else if (sum>1+DBL_EPSILON)
			{
				bufferIndex = 1;
			}
		}
		else
		{
			switch (subU+subV)
			{
			case 1:
				if (u+v>1)
				{
					bufferIndex = 1;
					if (finalV==81)
					{
						//check for V overflow, and move back to this layer boundaries
						finalV--;
						bufferIndex = 2;
					}
					if (finalU==81)
					{
						//check for U overflow, and move back to this layer boundaries
						finalU--;
						bufferIndex = 2;
					}
				}
				break;
			case 3:
				if (u>1 && v>=2 || v>1 && u>=2 )
				{
					bufferIndex = 2;
					break;
				}
				//Yep, we don't break, we fall off the next case because it the one we need
			case 2:
				if (subU==2)
				{
					finalU++;
				}
				if (subV==2)
				{
					finalV++;
				}
				break;
			}
		}
	}
	return getPixel(finalU,finalV,bufferIndex);
}


void RhombusRGBA::samplePixel(double u,double v,unsigned char * outputColor)
{
	const double knEps = DBL_EPSILON*1000;
	assert(u >=0 && u <= width-1 && v >= 0 && v <= height-1 && "UV is out of range");

	int finalU = static_cast<int>(floor(u));
	int finalV = static_cast<int>(floor(v));
	int bufferIndex = 0;

	//make u,v between [0,1)
	u-=finalU;
	v-=finalV;

	int partialU = static_cast<int>(floor(u*3));
	int partialV = static_cast<int>(floor(v*3));

	int roundU = static_cast<int>(floor(u*3+0.5)); //round((u*3)
	int roundV = static_cast<int>(floor(v*3+0.5)); //round((v*3)
	bool intersectionPoint = abs(u*3-roundU)<=knEps && abs(v*3-roundV)<=knEps;

	if (m_type == knEvenResolution)
	{
		if (intersectionPoint)
		{
			if (roundU==roundV)
			{
				switch (roundU)
				{
				case 0:
					memcpy(outputColor,getPixel(finalU,finalV),4);
					return;
				case 1:
					mix(outputColor,getPixel(finalU,finalV),getPixel(finalU+1,finalV),getPixel(finalU,finalV+1));
					return;
				case 2:
					mix(outputColor,getPixel(finalU+1,finalV+1),getPixel(finalU+1,finalV),getPixel(finalU,finalV+1));
					return;
				case 3:
					memcpy(outputColor,getPixel(finalU+1,finalV+1),4);
					return;
				}
			}
		}

		switch(partialU)
		{
		case 0:
			switch(partialV)
			{
			case 0:
				memcpy(outputColor,getPixel(finalU,finalV),4);
				return;
			case 1:
				{
					double distance = 0.5-u*0.5-v;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV),getPixel(finalU,finalV+1));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU,finalV+1),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV),4);
					}
				}
				return;
			case 2:
				memcpy(outputColor,getPixel(finalU,finalV+1),4);
				return;
			}
			break;
		case 1:
			switch(partialV)
			{
			case 0:
				{
					double distance = 0.5-v*0.5-u;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV),getPixel(finalU+1,finalV));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU+1,finalV),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV),4);
					}
				}
				return;
			case 1:
				if (abs(u-v)<knEps)
				{
					mix(outputColor,getPixel(finalU+1,finalV),getPixel(finalU,finalV+1));
				}
				else if (u<v)
				{
					memcpy(outputColor,getPixel(finalU,finalV+1),4);
				}
				else
				{
					memcpy(outputColor,getPixel(finalU+1,finalV),4);
				}
				return;
			case 2:
				{
					double distance = 1.0-v*0.5-u;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV+1),getPixel(finalU+1,finalV+1));
					}
					else if (distance>0)
					{
						memcpy(outputColor,getPixel(finalU,finalV+1),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU+1,finalV+1),4);
					}
				}
				return;
			}
			break;
		case 2:
			switch(partialV)
			{
			case 0:
				memcpy(outputColor,getPixel(finalU+1,finalV),4);
				return;
			case 1:
				{
					double distance = 1.0-u*0.5-v;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU+1,finalV),getPixel(finalU+1,finalV+1));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU+1,finalV+1),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU+1,finalV),4);
					}
				}
				return;
			case 2:
				memcpy(outputColor,getPixel(finalU+1,finalV+1),4);
				return;
			}
			break;
		}
	}
	else //m_type == knOddResolution
	{
		if (intersectionPoint)
		{
			switch(roundU)
			{
			case 0:
				switch(roundV)
				{
				case 0:
					memcpy(outputColor,getPixel(finalU,finalV,0),4);
					return;
				case 1:
					if (finalU==width-1)
					{
						mix(outputColor,getPixel(finalU,finalV,0),getPixel(finalU-1,finalV,2));
					}
					else
					{
						mix(outputColor,getPixel(finalU,finalV,0),getPixel(finalU,finalV,1));
					}
					return;
				case 2:
					if (finalU==width-1)
					{
						mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU-1,finalV,2));
					}
					else
					{
						mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,1));
					}
					return;
				case 3:
					memcpy(outputColor,getPixel(finalU,finalV+1,0),4);
					return;
				}
				return;
			case 1:
				switch(roundV)
				{
				case 0:
					if (finalV==height-1)
					{
						mix(outputColor,getPixel(finalU,finalV,0),getPixel(finalU,finalV-1,2));
					}
					else
					{
						mix(outputColor,getPixel(finalU,finalV,0),getPixel(finalU,finalV,1));
					}
					return;
				case 1:
					memcpy(outputColor,getPixel(finalU,finalV,1),4);
					return;
				case 2:
					mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,1),getPixel(finalU,finalV,2));
					return;
				case 3:
					mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,2));
					return;
				}
				return;
			case 2:
				switch(roundV)
				{
				case 0:
					if (finalV==height-1)
					{
						mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV-1,2));
					}
					else
					{
						mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,1));
					}
					return;
				case 1:
					mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,1),getPixel(finalU,finalV,2));
					return;
				case 2:
					memcpy(outputColor,getPixel(finalU,finalV,2),4);
					return;
				case 3:
					mix(outputColor,getPixel(finalU+1,finalV+1,0),getPixel(finalU,finalV,2));
					return;
				}
				return;
			case 3:
				switch(roundV)
				{
				case 0:
					memcpy(outputColor,getPixel(finalU+1,finalV,0),4);
					return;
				case 1:
					mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,2));
					return;
				case 2:
					mix(outputColor,getPixel(finalU+1,finalV+1,0),getPixel(finalU,finalV,2));
					return;
				case 3:
					memcpy(outputColor,getPixel(finalU+1,finalV+1,0),4);
					return;
				}
				return;
			}
		}

		u*=3;
		v*=3;

		switch(partialU)
		{
		case 0:
			switch(partialV)
			{
			case 0:
				{
					double distance = u+v-1;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,0),getPixel(finalU,finalV,1));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU,finalV,0),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV,1),4);
					}
				}
				return;
			case 1:
				{
					if (finalU > 0 && finalU < width-1 && u<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,1),getPixel(finalU-1,finalV,2));
					}
					else if (v<=2-knEps)
					{
						if(finalU==width-1)
						{
							memcpy(outputColor,getPixel(finalU-1,finalV,2),4);
						}
						else
						{
							memcpy(outputColor,getPixel(finalU,finalV,1),4);
						}
					}
					else
					{
						mix(outputColor,getPixel(finalU,finalV,1),getPixel(finalU,finalV+1,0));
					}
				}
				return;
			case 2:
				{
					if (v<=2+knEps)
					{
						if(finalU==width-1)
						{
							mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU-1,finalV,2));
						}
						else
						{
							mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,1));
						}
					}
					else if (u>=1-knEps)
					{
						mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,2));
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV+1,0),4);
					}
				}
				return;
			}
			break;
		case 1:
			switch(partialV)
			{
			case 0:
				{
					if (finalV > 0 && finalV < height-1 && v<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,1),getPixel(finalU,finalV-1,2));
					}
					else if (u<=2-knEps)
					{
						if(finalV==height-1)
						{
							memcpy(outputColor,getPixel(finalU,finalV-1,2),4);
						}
						else
						{
							memcpy(outputColor,getPixel(finalU,finalV,1),4);
						}
					}
					else
					{
						mix(outputColor,getPixel(finalU,finalV,1),getPixel(finalU+1,finalV,0));
					}
				}
				return;
			case 1:
				{
					double distance = u+v-3;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,1),getPixel(finalU,finalV,2));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU,finalV,1),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV,2),4);
					}
				}
				return;
			case 2:
				{
					if (u<=1+knEps)
					{
						mix(outputColor,getPixel(finalU,finalV+1,0),getPixel(finalU,finalV,2));
					}
					else if (finalV < height-1 && v>=3-knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,2),getPixel(finalU,finalV+1,1));
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV,2),4);
					}
				}
				return;
			}
			break;
		case 2:
			switch(partialV)
			{
			case 0:
				{
					if (u<=2+knEps)
					{
						if (finalV==width-1)
						{
							mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV-1,2));
						}
						else
						{
							mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,1));
						}
					}
					else if (v>=1-knEps)
					{
						mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,2));
					}
					else
					{
						memcpy(outputColor,getPixel(finalU+1,finalV,0),4);
					}
				}
				return;
			case 1:
				{
					if  (v<=1+knEps)
					{
						mix(outputColor,getPixel(finalU+1,finalV,0),getPixel(finalU,finalV,2));
					}
					else if (finalU < width-1 && u>=3-knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,2),getPixel(finalU+1,finalV,1));
					}
					else
					{
						memcpy(outputColor,getPixel(finalU,finalV,2),4);
					}
				}
				return;
			case 2:
				{
					double distance = u+v-5;
					if (abs(distance)<=knEps)
					{
						mix(outputColor,getPixel(finalU,finalV,2),getPixel(finalU+1,finalV+1,0));
					}
					else if (distance<0)
					{
						memcpy(outputColor,getPixel(finalU,finalV,2),4);
					}
					else
					{
						memcpy(outputColor,getPixel(finalU+1,finalV+1,0),4);
					}
				}
				return;
			}
			break;
		}
	}
}

void RhombusRGBA::mix(unsigned char * outputColor,unsigned char * color1,unsigned char * color2)
{
	int finalColor[4];

	finalColor[0] = color1[0]*color1[3];
	finalColor[1] = color1[1]*color1[3];
	finalColor[2] = color1[2]*color1[3];
	finalColor[3] = color1[3];

	if (color2[3]>0)
	{
		finalColor[0] += color2[0]*color2[3];
		finalColor[1] += color2[1]*color2[3];
		finalColor[2] += color2[2]*color2[3];
		finalColor[3] += color2[3];
	}

	if (finalColor[3]==0)
	{
		memset(outputColor,0,4);
		finalColor[3]=1;
	}
	else
	{
		outputColor[0] = static_cast<unsigned char>(finalColor[0]/finalColor[3]);
		outputColor[1] = static_cast<unsigned char>(finalColor[1]/finalColor[3]);
		outputColor[2] = static_cast<unsigned char>(finalColor[2]/finalColor[3]);
		outputColor[3] = static_cast<unsigned char>(finalColor[3]/2);
	}
}

void RhombusRGBA::mix(unsigned char * outputColor,unsigned char * color1,unsigned char * color2,unsigned char * color3)
{
	int finalColor[4];

	finalColor[0] = color1[0]*color1[3];
	finalColor[1] = color1[1]*color1[3];
	finalColor[2] = color1[2]*color1[3];
	finalColor[3] = color1[3];

	if (color2[3]>0)
	{
		finalColor[0] += color2[0]*color2[3];
		finalColor[1] += color2[1]*color2[3];
		finalColor[2] += color2[2]*color2[3];
		finalColor[3] += color2[3];
	}

	if (color3[3]>0)
	{
		finalColor[0] += color3[0]*color3[3];
		finalColor[1] += color3[1]*color3[3];
		finalColor[2] += color3[2]*color3[3];
		finalColor[3] += color3[3];
	}

	if (finalColor[3]==0)
	{
		memset(outputColor,0,4);
		finalColor[3]=1;
	}
	else
	{
		outputColor[0] = static_cast<unsigned char>(finalColor[0]/finalColor[3]);
		outputColor[1] = static_cast<unsigned char>(finalColor[1]/finalColor[3]);
		outputColor[2] = static_cast<unsigned char>(finalColor[2]/finalColor[3]);
		outputColor[3] = static_cast<unsigned char>(finalColor[3]/3);
	}
}

PYXPointer<RhombusRGBA> RhombusRGBA::expandToOddResolution()
{
	PYXPointer<RhombusRGBA> layer = RhombusRGBA::create(knOddResolution);

	//center buffer is the same
	memcpy(&(layer->getBuffer(0)),&(getBuffer(0)),sizeof(RGBABuffer));
	layer->m_allTransparent = m_allTransparent;
	layer->m_allOpaque = m_allOpaque;

	//first vertex layer
	for(int y=0;y<layer->height-1;y++)
	{
		for(int x=0;x<layer->width-1;x++)
		{
			unsigned char * destPixel = layer->getPixel(x,y,1);

			mix(destPixel,getPixel(x,y),getPixel(x+1,y),getPixel(x,y+1));
		}
	}

	//first vertex layer
	for(int y=0;y<layer->height-1;y++)
	{
		for(int x=0;x<layer->width-1;x++)
		{
			unsigned char * destPixel = layer->getPixel(x,y,2);
			
			mix(destPixel,getPixel(x+1,y+1),getPixel(x+1,y),getPixel(x,y+1));
		}
	}

	return layer;
}

PYXPointer<RhombusRGBA> RhombusRGBA::zoomIn(const Surface::Patch::Key & destination,const Surface::Patch::Key & source,ResolutionType resolutionType)
{
	if (destination==source && m_type == knEvenResolution && resolutionType == knOddResolution)
	{
		return expandToOddResolution();
	}

	Surface::UVOffset uv(destination,source);

	PYXPointer<RhombusRGBA> layer = RhombusRGBA::create(resolutionType);

	layer->m_allOpaque = true;
	layer->m_allTransparent = true;

	double uOffset = 81*uv.getUOffset();
	double vOffset = 81*uv.getVOffset();

	int bufferCount = layer->getBufferCount();

	int totalAlpha = 0;
	int totalPixels = 0;

	for(int y=0;y<layer->height;y++)
	{
		for(int x=0;x<layer->width;x++)
		{
			//we scale x and y by using our offset.
			//+buffer/3 - is get "sub pixel transformation to get the actual location of the sub pixel
			double realX = uOffset+uv.getUscale()*x;
			double realY = vOffset+uv.getVscale()*y;
			unsigned char * destPixel = layer->getPixel(x,y);

			samplePixel(realX,realY,destPixel);

			totalAlpha += destPixel[3];
			totalPixels++;
		}
	}

	if (bufferCount>1)
	{
		for(int y=0;y<layer->height-1;y++)
		{
			for(int x=0;x<layer->width-1;x++)
			{
				for (int buffer=1;buffer<bufferCount;buffer++)
				{
					//we scale x and y by using our offset.
					//+buffer/3 - is get "sub pixel transformation to get the actual location of the sub pixel
					double realX = uOffset+uv.getUscale()*(x+(buffer)/3.0);
					double realY = vOffset+uv.getVscale()*(y+(buffer)/3.0);
					unsigned char * destPixel = layer->getPixel(x,y,buffer);

					samplePixel(realX,realY,destPixel);

					totalAlpha += destPixel[3];
					totalPixels++;
				}
			}
		}
	}

	//set opaque and transparent channels
	layer->setAllTransparent(totalAlpha==0);
	layer->setAllOpaque(totalAlpha==255*totalPixels);

	return layer;
}

void RhombusRGBA::fillWith(int color,int alpha)
{
	unsigned char buffer[channels];
	memcpy(buffer,&color,channels-1);
	buffer[channels-1] = static_cast<unsigned char>(alpha);

	for(int bufferIndex=0;bufferIndex<getBufferCount();bufferIndex++)
	{
		for(int y=0;y<height;y++)
		{
			for(int x=0;x<width;x++)
			{
				memcpy(m_buffers[bufferIndex].rgba[y][x],buffer,channels);
			}
		}
	}

	m_allTransparent = (alpha == 0);
	m_allOpaque = (alpha == 255);
}

void RhombusRGBA::overlay(RhombusRGBA & otherLayer,const unsigned char globalAlpha)
{
	int totalAlpha = 0;
	int totalPixels = 0;

	for(int bufferIndex=0;bufferIndex<getBufferCount();bufferIndex++)
	{
		int bufferHeight = height - (bufferIndex==0?0:1);
		int bufferWidth  = width - (bufferIndex==0?0:1);

		for(int y=0;y<bufferHeight;y++)
		{
			for(int x=0;x<bufferWidth;x++)
			{
				unsigned char * resultPixel = getPixel(x,y,bufferIndex);
				unsigned char * inputPixel = otherLayer.getPixel(x,y,bufferIndex);

				int alpha = ((int)globalAlpha)*inputPixel[3]>>8;

				if (alpha>0)
				{
					//overlay the input on the result with alpha blending
					resultPixel[0] = static_cast<unsigned char>((((resultPixel[0])<<8) + alpha*(inputPixel[0]-resultPixel[0]))>>8);
					resultPixel[1] = static_cast<unsigned char>((((resultPixel[1])<<8) + alpha*(inputPixel[1]-resultPixel[1]))>>8);
					resultPixel[2] = static_cast<unsigned char>((((resultPixel[2])<<8) + alpha*(inputPixel[2]-resultPixel[2]))>>8);
					//make the result output the max between both alphas
					resultPixel[3] = std::max(resultPixel[3],static_cast<unsigned char>(alpha));
				}

				totalAlpha += resultPixel[3];
				totalPixels++;
			}
		}
	}

	//set opaque and transparent channels
	setAllTransparent(totalAlpha==0);
	setAllOpaque(totalAlpha==255*totalPixels);
}

///////////////////////////////////////////////////////////////////////
// RhombusRGBAFiller
///////////////////////////////////////////////////////////////////////

RhombusRGBAFiller::RhombusRGBAFiller(const std::string & name, const boost::intrusive_ptr<ICoverage> & coverage,const RhombusBitmapColorizer::IColorizer & colorizer)
	:	m_name(name),
		m_spCoverage(coverage),
		m_colorizer(colorizer)
{
	assert(m_spCoverage && "Coverage is null. can't raster that !");
}

//load the layer
PYXPointer<RhombusRGBA> RhombusRGBAFiller::load(const PYXRhombus & rhombus)
{
	PYXRhombusFiller filler(rhombus,8,11);

	while ( ! filler.isReady())
	{
		bool wasError=false;
		PYXPointer<PYXTile> tile = filler.getNeededTile();

		try
		{
			PYXPointer<PYXValueTile> valueTile;
			
			valueTile = m_spCoverage->getFieldTile(tile->getRootIndex(),tile->getCellResolution(),0);

			if (valueTile)
			{
				assert(valueTile->getNumberOfDataChannels() );
			}

			filler.addTile(tile,valueTile);
		}
		catch (PYXException& ex)
		{
			TRACE_INFO("failed to receive tile (" << m_name << "), try again. error was: " << ex.getFullErrorString());
			wasError=true;
		}
		catch(...)
		{
			TRACE_INFO("failed to receive tile (" << m_name << "), try again. unknown error");
			wasError=true;
		}

		if (wasError)
		{
			return PYXPointer<RhombusRGBA>();
		}
	}

	return fill(filler);
}

//load the layer only if all rhombuses cost is small
PYXPointer<RhombusRGBA> RhombusRGBAFiller::loadFast(const PYXRhombus & rhombus)
{
	PYXTimeLimitWarning warning("loadFast("+this->m_name+")",0.5);

	PYXRhombusFiller filler(rhombus,8,11);

	while ( ! filler.isReady())
	{		
		bool wasError=false;
		PYXPointer<PYXTile> tile = filler.getNeededTile();
		
		PYXCost cost = m_spCoverage->getFieldTileCost(tile->getRootIndex(),tile->getCellResolution(),0);

		if (cost.getMaximumCost() >= PYXCost::knDefaultCost.getMaximumCost())
		{
			return PYXPointer<RhombusRGBA>();
		}
		
		try
		{
			PYXTimeLimitWarning warning("loadFast.coverage.getFieldTile("+this->m_name+")",0.5);

			PYXPointer<PYXValueTile> valueTile;
			
			valueTile = m_spCoverage->getFieldTile(tile->getRootIndex(),tile->getCellResolution(),0);			

			filler.addTile(tile,valueTile);
		}
		catch (PYXException& ex)
		{
			TRACE_INFO("failed to receive tile, try again. error was: " << ex.getFullErrorString());
			wasError=true;
		}
		catch(...)
		{
			TRACE_INFO("failed to receive tile, try again. unknown error");
			wasError=true;
		}

		if (wasError)
		{
			return PYXPointer<RhombusRGBA>();
		}
	}

	return fill(filler);
}


PYXPointer<RhombusRGBA> RhombusRGBAFiller::fill(PYXRhombusFiller & filler)
{
	PYXPointer<RhombusRGBA> layer = RhombusRGBA::create(RhombusRGBA::knEvenResolution);

	//don't spend too much time on empty tiles
	if (filler.allValuesTilesAreNull())
	{
		layer->fillWith(0,0);
		return layer;
	}

	PYXRhombusFiller::IteratorWithLUT it = filler.getIteratorWithLUT(0);

	int totalAlpha = 0;
	int totalPixels = 0;

	while(!it.end())
	{
		unsigned char * pixel = layer->getPixel(it.getUCoord(),it.getVCoord(),it.getOffsetCoord());

		if (it.hasValue())
		{
			m_colorizer.colorPixel(it.getValue(),pixel);

			totalAlpha += pixel[3];
			totalPixels++;
		}
		else
		{
			memset(pixel,0,4);
			layer->setAllOpaque(false);
		}
		++it;
	}

	//set opaque and transparent channels
	layer->setAllTransparent(totalAlpha==0);
	layer->setAllOpaque(totalAlpha==255*totalPixels);

	return layer;
}


///////////////////////////////////////////////////////////////////////
// RhombusBitmapColorizer::GrayScaleColorizer
///////////////////////////////////////////////////////////////////////

RhombusBitmapColorizer::GrayScaleColorizer::GrayScaleColorizer(double minValue,double maxValue,unsigned char alpha)
	:	m_minValue(minValue),
		m_maxValue(maxValue),
		m_alpha(alpha)
{
	m_useInt = (m_maxValue-m_minValue) > 500;

	if (m_useInt)
	{
		m_intFactor = static_cast<int>(m_maxValue-m_minValue);
		m_intMinValue = static_cast<int>(m_minValue);
	}
	else
	{
		m_doubleFactor = 255/(m_maxValue-m_minValue);
	}
}

void RhombusBitmapColorizer::GrayScaleColorizer::colorPixel(const PYXValue & value,unsigned char * rgba) const
{
	if (m_useInt)
	{
		int val = value.getInt32(0)-m_intMinValue;	
		unsigned char color = cml::clamp(255*val/m_intFactor,0,255);
		memset(rgba,color,3);
		rgba[3] = m_alpha;
	}
	else
	{
		double val = value.getDouble(0)-m_minValue;	
		unsigned char color = cml::clamp((int)(val*m_doubleFactor),0,255);
		memset(rgba,color,3);
		rgba[3] = m_alpha;
	}
}


///////////////////////////////////////////////////////////////////////
// RhombusBitmapColorizer::PaletteColorizer
///////////////////////////////////////////////////////////////////////

RhombusBitmapColorizer::PaletteColorizer::PaletteColorizer(const std::string palette)
	:	m_palette(palette)
{
}

void RhombusBitmapColorizer::PaletteColorizer::colorPixel(const PYXValue & value,unsigned char * rgba) const
{
	m_palette.convert(value.getDouble(),rgba);
}


///////////////////////////////////////////////////////////////////////
// RhombusBitmapColorizer::RGBWithAlphaColorizer
///////////////////////////////////////////////////////////////////////

RhombusBitmapColorizer::RGBWithAlphaColorizer::RGBWithAlphaColorizer(unsigned char alpha)
	: m_alpha(alpha)
{
}

void RhombusBitmapColorizer::RGBWithAlphaColorizer::colorPixel(const PYXValue & value,unsigned char * rgba) const
{
	memcpy(rgba,value.getUInt8Ptr(0),4);
	rgba[3] = (((int)m_alpha)*rgba[3])/255;
}


///////////////////////////////////////////////////////////////////////
// RhombusBitmapColorizer::RGBConstAlphaColorizer
///////////////////////////////////////////////////////////////////////

RhombusBitmapColorizer::RGBConstAlphaColorizer::RGBConstAlphaColorizer(unsigned char alpha)
	: m_alpha(alpha)
{
}

void RhombusBitmapColorizer::RGBConstAlphaColorizer::colorPixel(const PYXValue & value,unsigned char * rgba) const
{
	memcpy(rgba,value.getUInt8Ptr(0),3);
	rgba[3] = m_alpha;
}

///////////////////////////////////////////////////////////////////////
// RhombusBitmapColorizer::RGB16BitConstAlphaColorizer
///////////////////////////////////////////////////////////////////////

RhombusBitmapColorizer::RGB16BitConstAlphaColorizer::RGB16BitConstAlphaColorizer(unsigned char alpha)
	: m_alpha(alpha)
{
}

void RhombusBitmapColorizer::RGB16BitConstAlphaColorizer::colorPixel(const PYXValue & value,unsigned char * rgba) const
{
	const uint16_t * buffer = value.getUInt16Ptr(0);
	rgba[0] = buffer[0]/256;
	rgba[1] = buffer[1]/256;
	rgba[2] = buffer[2]/256;
	rgba[3] = m_alpha;
}


///////////////////////////////////////////////////////////////////////
// RhombusRGBABlender
///////////////////////////////////////////////////////////////////////

RhombusRGBABlender::RhombusRGBABlender(RhombusRGBA::ResolutionType resolutionType) : m_type(resolutionType)
{
	m_buffers.reset(new RGBAIntegerBuffer[getBufferCount()]);
	memset(m_buffers.get(),0,sizeof(RGBAIntegerBuffer)*getBufferCount());
}

int RhombusRGBABlender::getBufferCount() const
{
	if (m_type==RhombusRGBA::knOddResolution)
	{
		return 3;
	}
	else
	{
		return 1;
	}
}

//! get the first pixel of the given buffer
RhombusRGBABlender::RGBAIntegerBuffer & RhombusRGBABlender::getBuffer(int bufferIndex)
{
	assert(bufferIndex < getBufferCount() && "BufferIndex is out of range");
	assert(m_buffers.get() != 0 && "buffers were not allocated");
	return m_buffers[bufferIndex];
}

//! fetch a single pixel from a buffer
unsigned int * RhombusRGBABlender::getPixel(int u,int v,int bufferIndex)
{
	assert(u >=0 && u < RhombusRGBA::width && v >= 0 && v < RhombusRGBA::height && "UV is out of range");
	if (bufferIndex>0)
	{
		assert(u < RhombusRGBA::width-1 && v < RhombusRGBA::height-1 && "UV is out of range");
	}
	return getBuffer(bufferIndex).rgba[v][u];
}


void RhombusRGBABlender::addRhombusRGBA(RhombusRGBA & rgba,unsigned char & globalAlpha)
{
	assert(getBufferCount() == rgba.getBufferCount());

	for(int bufferIndex=0;bufferIndex<rgba.getBufferCount();bufferIndex++)
	{
		int bufferHeight = rgba.height - (bufferIndex==0?0:1);
		int bufferWidth  = rgba.width - (bufferIndex==0?0:1);

		for(int y=0;y<bufferHeight;y++)
		{
			for(int x=0;x<bufferWidth;x++)
			{
				unsigned int * resultPixel = getPixel(x,y,bufferIndex);
				unsigned char * inputPixel = rgba.getPixel(x,y,bufferIndex);

				int alpha = ((int)globalAlpha)*inputPixel[3]>>8;

				if (alpha>0)
				{
					//overlay the input on the result with alpha blending
					resultPixel[0] += (alpha*inputPixel[0]);
					resultPixel[1] += (alpha*inputPixel[1]);
					resultPixel[2] += (alpha*inputPixel[2]);
					resultPixel[3] += alpha;
				}
			}
		}
	}
}

void RhombusRGBABlender::toRhombusRGBA(RhombusRGBA & rgba)
{
	assert(getBufferCount() == rgba.getBufferCount());

	rgba.setAllOpaque(true);
	rgba.setAllTransparent(true);

	for(int bufferIndex=0;bufferIndex<rgba.getBufferCount();bufferIndex++)
	{
		int bufferHeight = rgba.height - (bufferIndex==0?0:1);
		int bufferWidth  = rgba.width - (bufferIndex==0?0:1);

		for(int y=0;y<bufferHeight;y++)
		{
			for(int x=0;x<bufferWidth;x++)
			{
				unsigned int * inputPixel = getPixel(x,y,bufferIndex);
				unsigned char * resultPixel = rgba.getPixel(x,y,bufferIndex);
				
				int alpha = inputPixel[3];
				
				if (alpha>0)
				{
					//overlay the input on the result with alpha blending
					resultPixel[0] = static_cast<unsigned char>((inputPixel[0])/alpha);
					resultPixel[1] = static_cast<unsigned char>((inputPixel[1])/alpha);
					resultPixel[2] = static_cast<unsigned char>((inputPixel[2])/alpha);
					resultPixel[3] = 255;

					rgba.setAllTransparent(false);
				}
				else
				{
					memset(resultPixel,0,4);
					rgba.setAllOpaque(false);
				}
			}
		}
	}
}
