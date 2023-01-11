/******************************************************************************
mem_utils.cpp

begin		: 2004-06-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/mem_utils.h"

// pyxlib includes
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#ifdef WIN32
#	include <windows.h>
#endif

//!Various Memory Utility Methods.

//! Tester class.
Tester<MemUtils> gTester;

//! Test method.
void MemUtils::test()
{
	MemUtils::MemStatus theStatus;

	bool bStatus = false;
	bStatus = MemUtils::getMemoryStatus(&theStatus);
	if(!bStatus)
	{
		return;
	}

	int nVirtualRemaining = -1;
	nVirtualRemaining = theStatus.getAvailVirtual();
}

/*!
get the memory status.

\return true if able to load memory status, false if not able.
*/
bool MemUtils::getMemoryStatus(MemStatus* pStatus)
{
	assert(pStatus);
	if(!pStatus)
	{
		return false;
	}

#ifdef WIN32
	
	MEMORYSTATUSEX theMemoryStatus;

	theMemoryStatus.dwLength = sizeof (theMemoryStatus);

	GlobalMemoryStatusEx(&theMemoryStatus);
	
	pStatus->setTotalPhys(static_cast<unsigned int>(theMemoryStatus.ullTotalPhys));

	pStatus->setTotalVirtual(static_cast<unsigned int>(theMemoryStatus.ullTotalVirtual));
	
	pStatus->setTotalPageFiles(static_cast<unsigned int>(theMemoryStatus.ullTotalPageFile));

	pStatus->setAvailPhys(static_cast<unsigned int>(theMemoryStatus.ullAvailPhys));	

	pStatus->setAvailVirtual(static_cast<unsigned int>(theMemoryStatus.ullAvailVirtual));	
	
	pStatus->setAvailPageFiles(static_cast<unsigned int>(theMemoryStatus.ullAvailPageFile));

	pStatus->setMemoryLoad(static_cast<unsigned int>(theMemoryStatus.dwMemoryLoad));

#endif

	return true;
}


bool MemUtils::getHeapStatus(size_t * allocated,size_t * unallocated)
{
	size_t free = 0;
	size_t used = 0;

#ifdef WIN32

	HANDLE heap = GetProcessHeap();
	PROCESS_HEAP_ENTRY entry;
	entry.lpData = 0;

	struct HeapLocker
	{
		HANDLE m_heap;

		HeapLocker(HANDLE heap) : m_heap(heap)
		{
			HeapLock(m_heap);
		}

		~HeapLocker()
		{
			HeapUnlock(m_heap);
		}
	};
	
	{
		//Note: freezing the heap freezes the aplication while the function is running
		HeapLocker heapLocker(heap);

		while(HeapWalk(heap,&entry))
		{
			if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
			{
				used += entry.cbData + entry.cbOverhead;
			}
			else if (entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
			{
				free += entry.cbData;
			}
			else if (entry.wFlags & PROCESS_HEAP_REGION)
			{
				//do nothing...
			}
			else 
			{
				//free block
				free += entry.cbData;
			}
		}
	}

#endif

	*allocated = used;
	*unallocated = free;
	
	return true;
}
