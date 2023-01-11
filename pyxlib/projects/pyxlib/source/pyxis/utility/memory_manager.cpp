/******************************************************************************
memory_manager.cpp

begin		: 2004-09-02
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/memory_manager.h"

// pyxlib includes
#include "pyxis/utility/mem_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/thread_pool.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "sqlite3.h"

// standard includes
#include <cassert>

//! The singleton instance of the memory manager
MemoryManager* MemoryManager::m_pInstance = 0;

boost::recursive_mutex s_memoryManagerMutex;

_PNH MemoryManager::m_oldNewHandler = 0;

// TODO: Why are we casting from a double?
//! The required buffer size in bytes beyond the memory request
const size_t knMemoryBufferSize = static_cast<size_t>(20.0E6);

/*!
Constructor.  Register us as a memory consumer with the memory manager.
*/
MemoryConsumer::MemoryConsumer()
{
	MemoryManager::getInstance()->registerConsumer(this);
}

/*!
Destructor.  Unregister us from the memory manager.
*/
MemoryConsumer::~MemoryConsumer()
{
	MemoryManager::getInstance()->unregisterConsumer(this);
}

/*!
Unregister this memory consumer.
*/
void MemoryConsumer::unregisterConsumer()
{
	MemoryManager::getInstance()->unregisterConsumer(this);
}


VaryingMemoryUsed::VaryingMemoryUsed(size_t nNumBytes) : m_nNumBytes(0)
{
	memoryChangedDelta(nNumBytes);
}

VaryingMemoryUsed::~VaryingMemoryUsed()
{
	memoryChangedDelta(-m_nNumBytes);
}

//Report the memory manage about memory change
void VaryingMemoryUsed::memoryChangedDelta(int changeDelta)
{
	static boost::recursive_mutex s_mutex;
	{
		boost::recursive_mutex::scoped_lock(s_mutex);
		m_nNumBytes += changeDelta;

		assert(m_nNumBytes >= 0);
	}

	auto manager = MemoryManager::getInstance();
	if (manager == 0)
	{
		return;
	}

	if (changeDelta>0)
	{
		manager->memoryAllocated(changeDelta);
	}
	else 
	{
		manager ->memoryReleased(-changeDelta);
	}
}


/*!
Constructor initializes member variables.

\param	nNumBytes	The number of bytes in the memory block.
*/
MemoryToken::MemoryToken(size_t nNumBytes) :
	m_nNumBytes(nNumBytes)
{
	assert(0 <= nNumBytes);

	// Tell memory manager that some memory has been allocated.
	auto manager = MemoryManager::getInstance();
	if (manager != 0)
	{
		manager->memoryAllocated(nNumBytes);
	}
}

/*!
Destructor notifies memory manager that resource has been
released.
*/
MemoryToken::~MemoryToken()
{
	// tell memory manager that the token has been released	
	auto manager = MemoryManager::getInstance();
	if (manager != 0)
	{
		manager->releaseResource(this);
	}
}

/*!
Constructor initializes member variables.

\param	ptr			The allocated memory.
\param	nNumBytes	The number of bytes in the memory block.
*/
MemoryResource::MemoryResource(char* ptr, size_t nNumBytes) :
	m_ptr(ptr),
	m_pMemoryToken(0)
{
	assert(0 != ptr);

	m_pMemoryToken = new MemoryToken(nNumBytes);
}

/*!
Destructor frees memory and notifies memory manager that resource has been
released.
*/
MemoryResource::~MemoryResource()
{
	// Delete the token.
	delete m_pMemoryToken;

	// free up memory
	delete[] m_ptr;
}

//! The unit test class
Tester<MemoryManager> gTester( knRunTestOnlyInForeground);
void MemoryManager::test()
{
	return;
	TRACE_INFO("MEMORY Manager Test");

	MemoryManagerTestConsumer memConsumer;
	MemoryManager* pMemManager = MemoryManager::getInstance();
	std::vector<char*> memPtrs(20); //Just a size to help kill memory.
	size_t nAllocSize = memConsumer.getAllocationSize();
 
	//Test Allocations. 
	{
		//Eat memory until a low memory condition exists.
		while (!memConsumer.getMemFreedFlag())
		{
			try
			{
				memConsumer.allocateAndWrite();
			}
			catch (...)
			{
				TEST_ASSERT (false && "memConsumer.allocateAndWrite() threw an exception.");
			}
			TEST_ASSERT(memConsumer.getAResource()->getByteCount() == 
				memConsumer.getAllocationSize());
		}
		
		// Test that allocations can still suceed in low memory.
		for (int nCount = 0; nCount < 5; ++nCount)
		{
			memConsumer.allocateAndWrite();
			TEST_ASSERT(memConsumer.getAResource()->getByteCount() == 
				memConsumer.getAllocationSize());
		}

		TEST_ASSERT(memConsumer.getMemFreedFlag());
	}

	memConsumer.resetMemFreedFlag();
	memConsumer.freeAllMemoryConsumed(); // Free all mem currently allocated.

	//Test allocations & Freeing of just tokens.
	{
		//Eat memory until a low memory condition exists.	
		while (!memConsumer.getMemFreedFlag())
		{
			memConsumer.allocateToken();
			int nByteCount = memConsumer.getAToken()->getByteCount();
			TEST_ASSERT(nByteCount == memConsumer.getAllocationSize());
		}
		
		// Test that allocations can still suceed in low memory.
		for (int nCount = 0; nCount < 5; ++nCount)
		{
			memConsumer.allocateToken();
			int nByteCount = memConsumer.getAToken()->getByteCount();
			TEST_ASSERT(nByteCount == memConsumer.getAllocationSize());
		}
		TEST_ASSERT(memConsumer.getMemFreedFlag()); // Test that Mem Manager asked use to free tokens.
	}

		memConsumer.freeAllMemoryConsumed(); // Free all mem currently allocated.
		memConsumer.resetMemFreedFlag();

	size_t nTokenSize = static_cast<size_t>(50E6); // 50 Meg
	int nAllocation = 0;
	//Test allocations with memory tokens.
	{
		//Eat memory.
		while (!memConsumer.getMemFreedFlag())
		{
			memConsumer.allocateAndWrite();
			TEST_ASSERT(memConsumer.getAResource()->getByteCount() == 
				memConsumer.getAllocationSize());
	
			//Allocate some memory.
			memPtrs[nAllocation] = new char[nTokenSize];
			
			//Write to the newly allocated memory.
			strcpy_s(memPtrs[nAllocation], nTokenSize, "Pyxis becauses hexagons are better.");
			
			//Request a token for the memory we just allocated.
		    memConsumer.allocateToken(nTokenSize);
			//int nBytes = memConsumer.getAToken(nAllocation)->getByteCount();

			//Ensure that we actually got a token of the right size.
			TEST_ASSERT(memConsumer.getAToken()->getByteCount() == nTokenSize);
			
			++nAllocation;

			//Make sure we don't smash the heap by over flowing our vector.
			if (nAllocation > static_cast<int>(memPtrs.size()))
			{
				memPtrs.resize(2 * memPtrs.size());
			}
		}

		// Test allocations in low memory conditions.
		for (int nCount = 0; nCount < 5; ++nCount)
		{
			memConsumer.allocateAndWrite();
			TEST_ASSERT(memConsumer.getAResource()->getByteCount() == 
				memConsumer.getAllocationSize());
	
			//Allocate some memory.
			memPtrs[nAllocation] = new char[nTokenSize];
			
			//Write to the newly allocated memory.
			strcpy_s(memPtrs[nAllocation], nTokenSize, "Pyxis becauses hexagons are better.");
			
			//Request a token for the memory we just allocated.
		    memConsumer.allocateToken(nTokenSize);
			//int nBytes = memConsumer.getAToken(nAllocation)->getByteCount();

			//Ensure that we actually got a token of the right size.
			TEST_ASSERT(memConsumer.getAToken()->getByteCount() == nTokenSize);
			
			++nAllocation;

			//Make sure we don't smash the heap by over flowing our vector.
			if (nAllocation > static_cast<int>(memPtrs.size()))
			{
				memPtrs.resize(2 * memPtrs.size());
			}

		}

		TEST_ASSERT(memConsumer.getMemFreedFlag()); //Test mem manager aksed us to free mem.
	}

	//Clean up memory that has been used. 
	{
		memConsumer.freeAllMemoryConsumed(); // Free all mem currently allocated.
		for (int nAllocation = 0; nAllocation < static_cast<int>(memPtrs.size()); ++nAllocation)
		{
			if (0 != memPtrs[nAllocation])
			{
				delete[] memPtrs[nAllocation];
				memPtrs[nAllocation] = 0;
			}
		}
	}

	//wait until memory manager finish with this consumer...
	boost::this_thread::sleep(boost::posix_time::seconds(5)); 
}

/*!
Get the singleton instance of the memory manager.

\return	The singleton instance of the memory manager.
*/
MemoryManager* MemoryManager::getInstance()
{
	assert(m_pInstance);
	return m_pInstance;
}

MemoryManager::MemoryManager() :
	m_releasingMemory(false),
	m_vecConsumers(),
	m_nMemoryAllocated(0),
	m_nMaximumAllocation(0),
	m_nNextConsumerToFreeMemory(0),
	m_ticksToUpdateOtherMemoryAllocated(0),
	m_nOtherMemoryAllocated(0)
{
	MemUtils::MemStatus memStatus;

	// Set all values to zero.
	memset(&memStatus, 0, sizeof(memStatus));

	MemUtils::getMemoryStatus(&memStatus);

	// Set maximum amount of memory we can allocate - half of available virtual memory
	m_nMaximumAllocation = (size_t)(memStatus.getTotalVirtual()-memStatus.getAvailVirtual()/2);

	{
		const size_t MB = 1024 * 1024;
		//!finner addjuestment of max allocation by the user
		AppProperty<unsigned int> defaultRatio("MemoryManager","MemoryUsageRatio",50,"amount of memory can be used by the application (between 50-100 percent of overwhole virtual memory)");

		//!make sure the value is in the right place
		size_t memoryRatio = std::max(std::min((size_t)defaultRatio,(size_t)100),(size_t)50);
		
		//! reasnoable enough to keep the program still working :D
		size_t safeDistance = 210 * MB;
		
		//! make sure we can allow maximumMemoryAllowed from available memory
		size_t maximumMemoryAllowed = std::min(memStatus.getAvailVirtual()-safeDistance,(size_t)(memStatus.getAvailVirtual()*(memoryRatio/100.0)));

		//! set the new memory limit
		//m_nMaximumAllocation = (size_t)(memStatus.getTotalVirtual()-memStatus.getAvailVirtual()+maximumMemoryAllowed);	

		m_nMaximumAllocation = MB * memoryRatio * 10;
	
		m_nTotalMemoryAvailable = memStatus.getTotalVirtual();
		m_nFreeMemory = memStatus.getAvailVirtual();

		m_nOSnCodeUsed = m_nTotalMemoryAvailable-m_nFreeMemory;
	
		m_nSqliteAllocated = 0;


		MemUtils::getHeapStatus(&m_heapAllocated,&m_heapUnallocated);
	}	

	guardMemoryUsage();

	TRACE_INFO("New instance of 'MemoryManager' created.  Allowed Memory = " +
		StringUtils::toString(m_nMaximumAllocation) + " Bytes.");
}

void MemoryManager::guardMemoryUsage()
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	if (m_ticksToUpdateOtherMemoryAllocated<0)
	{
		MemUtils::MemStatus memStatus;

		// Set all values to zero.
		memset(&memStatus, 0, sizeof(memStatus));

		MemUtils::getMemoryStatus(&memStatus);

		m_nFreeMemory = memStatus.getAvailVirtual();
		
		MemUtils::getHeapStatus(&m_heapAllocated,&m_heapUnallocated);
		m_nOtherMemoryAllocated = m_heapAllocated - m_nMemoryAllocated;

		m_ticksToUpdateOtherMemoryAllocated = 10000;
	}
	else
	{
		m_ticksToUpdateOtherMemoryAllocated--;
	}

	if (!m_releasingMemory && ((m_nMemoryAllocated + m_nOtherMemoryAllocated) > m_nMaximumAllocation) && (m_nMemoryAllocated > 0))
	{
		m_releasingMemory = true;
		PYXThreadPool::addSlowTask(boost::bind(&MemoryManager::freeConsumerMemory,this));
	}
}

VaryingMemoryUsed & MemoryManager::getMemoryUsageTopic(const std::string & name)
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	std::map<std::string,boost::shared_ptr<VaryingMemoryUsed>>::iterator it = m_usedSections.find(name);

	if (it != m_usedSections.end())
	{
		return *(it->second);
	}

	return *(m_usedSections[name] = boost::shared_ptr<VaryingMemoryUsed>(new VaryingMemoryUsed()));
}

std::map<std::string,long> MemoryManager::getMemoryStatus()
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	std::map<std::string,long> status;

	status["max"]     = (long)m_nTotalMemoryAvailable;
	status["code"]    = (long)m_nOSnCodeUsed;
	status["free"]    = (long)m_nFreeMemory;
	status["sqlite"]  = (long)m_nSqliteAllocated;
	status["managed"] = (long)m_nMemoryAllocated;
	status["other"]   = (long)m_nOtherMemoryAllocated;

	for(std::map<std::string,boost::shared_ptr<VaryingMemoryUsed>>::iterator it = m_usedSections.begin(); it != m_usedSections.end(); ++it)
	{
		status[it->first] = it->second->getByteCount();
	}

	return status;
}

void MemoryManager::freeConsumerMemory()
{
	try
	{
		{
			boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

			int used,peak;
			//we can check SQLITE_STATUS_PAGECACHE_USED and SQLITE_STATUS_SCRATCH_USED
			sqlite3_status(SQLITE_STATUS_MEMORY_USED,&used,&peak,false);
			m_nSqliteAllocated = used;

			//NOTE: we could sqlite3_soft_heap_limit64 if needed. but it seems sqllite is not using so much memory
			//sqlite3_int64 sqliteLimit =  sqlite3_soft_heap_limit64(-1);
			//TRACE_INFO("sqllite soft memory limit : " << sqliteLimit << " bytes");
		}

		if (m_nNextConsumerToFreeMemory >= m_vecConsumers.size())
		{
			m_nNextConsumerToFreeMemory = 0;
		}

		size_t nNumConsumersAskedToFree = 0;
		size_t nCnt = m_vecConsumers.size();

		// Go through all consumers as many times as necessary to free the needed memory.
		while (getMemoryUsagePercent() + getOtherMemoryUsagePercent() > 0.99 && nNumConsumersAskedToFree < nCnt)
		{
			// Iterate through all consumers until the amount we request has been freed.
			
			// Get the next consumer to free memory.
			MemoryConsumer* pConsumer = m_vecConsumers[m_nNextConsumerToFreeMemory];

			// ask the consumer to free up some memory.
			pConsumer->freeMemory();

			++m_nNextConsumerToFreeMemory;

			if (m_nNextConsumerToFreeMemory >= m_vecConsumers.size())
			{
				m_nNextConsumerToFreeMemory = 0;
			}
			++nNumConsumersAskedToFree;
		}
	}
	catch(...)
	{
	}

	m_releasingMemory = false;
}

/*!
New memory handler called when a 'new' fails.

\param	nAllocSize	Number of bytes 'new' tried to allocate.

\return	Non zero if allocation should be tried again, 0 to fail.
*/
int memoryManagerNewHandler(size_t nAllocSize)
{
	MemoryManager* pManager = MemoryManager::getInstance();

	// Have memory manager ask its consumers to free some memory.
//	pManager->freeConsumerMemory(nAllocSize);

	// Return 1 to try allocation again, 0 to fail.
	return 1;
}

/*!
Initialize any static data
*/
void MemoryManager::initStaticData()
{
	m_pInstance = new MemoryManager();

	// Set our new handler and get pointer to previous one.
//	m_oldNewHandler = _set_new_handler(memoryManagerNewHandler);

	//! Set new handler mode to 1 so malloc does not throw on failure to allocate.
//	_set_new_mode(1);
}

/*!
Free singleton instance of the memory manager.
*/
void MemoryManager::freeStaticData()
{
	// Set new handler back to original one.
//	_set_new_handler(m_oldNewHandler);
//	_set_new_handler(0);

	// Delete our instance.
	delete m_pInstance;
	m_pInstance = 0;
}

/*!
Request a memory resource from the memory manager. If sufficient memory is
available this method allocates and returns a memory resource immediately.
Otherwise it requests each memory consumer from the least recently used to most
recently used to free up memory resources until the requested memory is
available.

\param	nNumBytes	The number of bytes requested.

\return	A memory resource (ownership transferred) or 0 if insufficient memory.
*/
MemoryResource* MemoryManager::requestMemory(size_t nNumBytes)
{
	// initialize variables
	MemoryResource* pResource = 0;
	
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	// allocate memory
	char* ptr;
	try
	{
		ptr = new char[nNumBytes];
	}
	catch (...)
	{
		ptr = 0;
	}
	if (0 != ptr)
	{
		pResource = new MemoryResource(ptr, nNumBytes);
		if (0 == pResource)
		{
			TRACE_ERROR("FAILURE: Request to allocate '"  << nNumBytes << "' bytes of memory failed.");
			// memory allocation failed
			delete[] ptr;
		}
	}
	else
	{
		// We didn't get the memory so our max pool size it too big
		// or the amount requested was too big.
		if (m_nMaximumAllocation > (m_nMemoryAllocated + m_nOtherMemoryAllocated + nNumBytes - 1))
		{
			// Lower the maximum allocation size since we can't actually get that much.
			m_nMaximumAllocation = m_nMemoryAllocated + m_nOtherMemoryAllocated + nNumBytes - 1;

			TRACE_INFO("Memory Manager is out of memory, increasing maximum allocation to " << m_nMaximumAllocation );
			// If we have memory that we can free, then try 
			if (m_nMemoryAllocated > 0)
			{
				pResource = requestMemory(nNumBytes);
			}
		}
	}

	guardMemoryUsage();

	return pResource;
}

/*!
Request a memory token from the memory manager. Used to inform the
memory manager that memory has been allocated by another object.

\param	nNumBytes	The number of bytes requested.

\return	A memory resource (ownership transferred) or 0 if insufficient memory.
*/
MemoryToken* MemoryManager::requestToken(size_t nNumBytes)
{
	MemoryToken* pToken = new MemoryToken(nNumBytes);

	guardMemoryUsage();

	return pToken;
}

/*!
Release a memory resource. This method is called by the MemoryResource
destructor.

\param	pResource	The resource to release.
*/
void MemoryManager::releaseResource(MemoryResource* pResource)
{
	releaseResource(pResource->m_pMemoryToken);
}

/*!
Release a memory token. This method is called by the MemoryToken
destructor.

\param	pToken	The token to release.
*/
void MemoryManager::releaseResource(MemoryToken* pToken)
{
	memoryReleased(pToken->getByteCount());
}

/*!
Register a consumer with the memory manager.

\param	pConsumer Pointer to the consumer to register.
*/
void MemoryManager::registerConsumer(MemoryConsumer* pConsumer)
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);
	m_vecConsumers.push_back(pConsumer);
}

/*!
Unregister a consumer from the memory manager.

\param	pConsumer	Pointer to removed from the memory manager.
*/
void MemoryManager::unregisterConsumer(MemoryConsumer* pConsumer)
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	// remove token from the memory manager's list
	MemoryConsumerVector::iterator it = m_vecConsumers.begin();

	bool bFound = false;
	while (it != m_vecConsumers.end() && !bFound)
	{
		if ((*it) == pConsumer)
		{
			bFound = true;
		}
		else
		{
			++it;
		}
	}

	// Make sure we found a consumer.  If so, remove it from the vector.
	if (it != m_vecConsumers.end())
	{
		m_vecConsumers.erase(it);
	}
}

/*!
Notify memory manager that an amount of memory has been freed.

\param nAmtReleased	Amount of memory released in bytes.
*/
void MemoryManager::memoryReleased(size_t nAmtReleased)
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);

	if (nAmtReleased < m_nMemoryAllocated)
	{
		m_nMemoryAllocated -=nAmtReleased;
	}
	else
	{
		m_nMemoryAllocated = 0;
	}
}

/*!
Notify memory manager that an amount of memory has been allocated.

\param	nAmtAllocated	Number of bytes of memory allocated.
*/
void MemoryManager::memoryAllocated(size_t nAmtAllocatedDelta)
{
	boost::recursive_mutex::scoped_lock lock(s_memoryManagerMutex);
	m_nMemoryAllocated +=nAmtAllocatedDelta;

	guardMemoryUsage();
}



















/*!
Default constructor.
*/
MemoryManagerTestConsumer::MemoryManagerTestConsumer():
	m_queMemPtrs(),
	m_queMemTokens(),
	m_pMemManager(0),
	m_bMemFreed(false)
{
	m_pMemManager = MemoryManager::getInstance();
}

/*!
Default Destructor.
*/
MemoryManagerTestConsumer::~MemoryManagerTestConsumer()
{
	//Clean up any resources still allocated. 
	freeAllMemoryConsumed();
}

/*!
Requests a memory resource from the memory manager in the size of 
the predefined allocation size. The requested resource is pushed 
onto the back of the queue. The limit on the number of resources 
is limited by memory only. 
*/
void MemoryManagerTestConsumer::allocateAndWrite()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(m_pMemManager != 0 && "No instance of memory manager.");
	m_queMemPtrs.push(m_pMemManager->requestMemory(knAllocationSize));
	if (m_queMemPtrs.back() != 0)
	{
		strcpy_s(m_queMemPtrs.back()->getPtr(), knAllocationSize, "Don't be a GIS Square...");
	}
}

/*!
Frees a memory resource when called upon to do so by the memory manager. Memory
is freed by making a request to free a resource and free a token.
*/
void MemoryManagerTestConsumer::freeMemory()
{
	freeResource();
	freeToken();
	m_bMemFreed = true;
}

/*!
Gets a memory resource from a queue of resources that this being 
consumed by this consumer. The most recently allocated resource is 
returned.

\return A pointer to the most recently allocated memory resource. 
*/
MemoryResource* MemoryManagerTestConsumer::getAResource()
{
	return m_queMemPtrs.back();
}

/*!
Gets a memory token from a queue of token that this being 
consumed by this consumer. The most recently allocated token is 
returned.

\return A pointer to the most recently allocated memory token. 
*/
MemoryToken* MemoryManagerTestConsumer::getAToken()
{
	return m_queMemTokens.back();
}

/*!
Delete all memory currently allocated. No questions asked.
*/
void MemoryManagerTestConsumer::freeAllMemoryConsumed()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	for (size_t nCount = 0; nCount < m_queMemPtrs.size(); ++nCount)
	{
		freeResource();	
	}

	for (size_t nCount = 0; nCount < m_queMemTokens.size(); ++nCount)
	{
		freeToken();
	}
}

/*!
Requests a memory token from the memory manager in the size of 
the predefined allocation size. The requested resource is pushed 
onto the back of the queue. The limit on the number of resources 
is limited by memory only. 

\param	nTokenSize The size of the token to request memory for.
*/
void MemoryManagerTestConsumer::allocateToken(size_t nTokenSize)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_queMemTokens.push(m_pMemManager->requestToken(nTokenSize));
}

/*!
Frees a memory token that has been allocated. The oldest token is
the one that gets freed from the front of the queue.
*/
void MemoryManagerTestConsumer::freeToken()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (m_queMemTokens.size() > 0)
	{
		delete m_queMemTokens.front();
		m_queMemTokens.front() = 0; 
		m_queMemTokens.pop();
	}
}

/*!
Frees a memory resource that has been allocated. The oldest resource is
the one that gets freed from the front of the queue.
*/
void MemoryManagerTestConsumer::freeResource()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (m_queMemPtrs.size() > 0)
	{
		delete m_queMemPtrs.front();
		m_queMemPtrs.front() = 0; 
		m_queMemPtrs.pop();
	}
}
