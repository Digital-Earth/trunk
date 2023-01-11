#ifndef PYXIS__UTILITY__MEM_UTILS_H
#define PYXIS__UTILITY__MEM_UTILS_H
/******************************************************************************
mem_utils.h

begin		: 2004-06-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

/*!
Various memory methods 
*/
//! Various memory methods
class PYXLIB_DECL MemUtils
{
public:

	//! Test method
	static void test();

	//! stores attributes from the MemUtils::getMemoryStatus() call.
	/*!
	stores attributes from the MemUtils::getMemoryStatus() call. 
	*/
	class MemStatus
	{
	
	public:
	
		//!get the amount of virtual memory remaining.
		virtual unsigned int getAvailVirtual(){ return m_nAvailVirtual; }

		//!get the total amount of virtual memory.
		virtual unsigned int getTotalVirtual(){ return m_nTotalVirtual; }

		//!get the amount of page files remaining.
		virtual unsigned int getAvailPageFiles(){ return m_nAvailPageFile; }

		//!get the total amount of page files.
		virtual unsigned int getTotalPageFiles(){ return m_nTotalPageFile; }

		//!get the amount of physical memory remaining.
		virtual unsigned int getAvailPhys(){ return m_nAvailPhys; }

		//!get the total amount of physical memory.
		virtual unsigned int getTotalPhys(){ return m_nTotalPhys; }

		//!get the memory load.
		virtual unsigned int getMemoryLoad(){ return m_nMemoryLoad; }

	private:

		//!set the amount of virtual memory remaining.
		void setAvailVirtual(unsigned int nAvailVirtual){ m_nAvailVirtual = nAvailVirtual; }

		//!set the total amount of virtual memory.
		void setTotalVirtual(unsigned int nTotalVirtual){ m_nTotalVirtual = nTotalVirtual; }

		//!set the amount of page files remaining.
		void setAvailPageFiles(unsigned int nAvailPageFile){ m_nAvailPageFile = nAvailPageFile; }

		//!set the total amount of page files.
		void setTotalPageFiles(unsigned int nTotalPageFile){ m_nTotalPageFile = nTotalPageFile; }

		//!set the amount of physical memory remaining.
		void setAvailPhys(unsigned int nAvailPhys){ m_nAvailPhys = nAvailPhys; }

		//!set the total amount of physical memory.
		void setTotalPhys(unsigned int nTotalPhys){ m_nTotalPhys = nTotalPhys; }

		//!set the total amount of physical memory.
		void setMemoryLoad(unsigned int nMemoryLoad){ m_nMemoryLoad = nMemoryLoad; }

		//!the Memory Load.
		unsigned int m_nMemoryLoad;
    
		//!the total physical memory.
		unsigned int m_nTotalPhys;
    
		//!the available physical memory.
		unsigned int m_nAvailPhys;
    
		//!the total number of page files.
		unsigned int m_nTotalPageFile;
    
		//!the available number of page files.
		unsigned int m_nAvailPageFile;
    
		//!the total virtual memory.
		unsigned int m_nTotalVirtual;
    
		//!the virtual memory available.
		unsigned int m_nAvailVirtual;

		//! the MemUtils class can set the values
		friend class MemUtils;
	};

	//! gets the memory status.
	static bool getMemoryStatus(MemStatus* pMemStatus);

	//! get the heap status
	static bool getHeapStatus(size_t * allocated,size_t * unallocated);
};

#endif // guard
