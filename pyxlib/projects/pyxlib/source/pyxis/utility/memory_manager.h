#ifndef PYXIS__UTILITY__MEMORY_MANAGER_H
#define PYXIS__UTILITY__MEMORY_MANAGER_H
/******************************************************************************
memory_manager.h

begin		: 2004-09-02
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// boost includes
#include <boost/detail/atomic_count.hpp>
#include <boost/thread/mutex.hpp>

// standard includes
#include <new.h>
#include <queue>
#include <map>

// forward declarations
class MemoryResource;
class MemoryToken;

//! Pure virtual interface for a class that consumes memory.
/*!
The MemoryConsumer is a class the cooperates with the memory manager to
manage scarce RAM appropriately.
*/
class PYXLIB_DECL MemoryConsumer
{
public:

	/*!
	This method is called by the memory manager when a request for memory
	cannot be satisfied. It asks the MemoryConsumer to free up a memory
	resource.
	*/
	//! Called by the memory manager to free up memory.
	virtual void freeMemory() = 0;

protected:

	//! Constructor.
	MemoryConsumer();

	//! Destructor.
	virtual ~MemoryConsumer();

	// TODO: Move to private when no longer needed in DTED file.
	//! Unregister a memory consumer.
	void unregisterConsumer();
};

/*!
The memory token contains the amount of memory allocated.
It's purpose is assist the memory manager with tracking the amount of
memory allocated and freed.
*/
//! Represents a memory token.
class PYXLIB_DECL MemoryToken
{
public:

	//! Destructor
	virtual ~MemoryToken();

	//! The the number of bytes in the memory block
	inline size_t getByteCount() const { return m_nNumBytes; }

private:

	//! Disable default constructor
	MemoryToken();

	//! Disable copy constructor
	MemoryToken(const MemoryToken&);

	//! Disable copy assignment
	void operator=(const MemoryToken&);

protected:
	//! Constructor
	MemoryToken(size_t nNumBytes);

private:
	//! The number of bytes
	size_t m_nNumBytes;

	// ensure MemoryManager and MemoryResource can see private constructor
	friend class MemoryManager;
	friend class MemoryResource;
};

// TODO: Audit these classes for exception and thread safety, 
// and minimize use of bald owner pointers -- especially in public interfaces.
/*!
The memory resource contains some allocated memory and the MemoryConsumer that
owns the memory. It's purpose is twofold - to allow the memory manager to keep
track of MemoryConsumers and to ensure the MemoryResource is removed from the
MemoryManager's list when the object is deleted.
*/
//! Represents a memory resource.
class PYXLIB_DECL MemoryResource
{
public:

	//! Destructor
	virtual ~MemoryResource();

	// TODO: Use unsigned char
	//! Get the pointer to the memory
	inline char* getPtr() { return m_ptr; }

	//! The the number of bytes in the memory block
	inline size_t getByteCount() const { return m_pMemoryToken->getByteCount(); }

private:

	//! Disable default constructor
	MemoryResource();

	//! Disable copy constructor
	MemoryResource(const MemoryResource&);

	//! Disable copy assignment
	void operator=(const MemoryResource&);

	// TODO: Use unsigned char
	//! Constructor
	MemoryResource(char* ptr, size_t nNumBytes);

	// TODO: Use unsigned char; consider scoped_array
	//! The memory
	char* m_ptr;

	// TODO: Consider scoped_ptr
	//! The memory token.
	MemoryToken* m_pMemoryToken;

	// ensure MemoryManager can see private constructor
	friend class MemoryManager;
};

//Helper class to track usage of memory not using MemoryConsumer API
class PYXLIB_DECL MemoryUsed : public MemoryToken
{
public:
	MemoryUsed(size_t nNumBytes) : MemoryToken(nNumBytes) {;}

	virtual ~MemoryUsed() {;}
};

//Helper class to track usage of memory not using MemoryConsumer API
class PYXLIB_DECL VaryingMemoryUsed
{
private:
	int m_nNumBytes;

public:
	VaryingMemoryUsed(size_t nNumBytes = 0);

	virtual ~VaryingMemoryUsed();

	//note: this can be negative!
	inline int getByteCount() const { return m_nNumBytes; }

	void memoryChangedDelta(int changeDelta);

private:	
	//! Disable copy constructor
	VaryingMemoryUsed(const VaryingMemoryUsed &);

	//! Disable copy assignment
	VaryingMemoryUsed & operator=(const VaryingMemoryUsed &);
};

//! Manages memory allocation.
/*!
The memory manager works with memory consumers to steward the available RAM
on a computer. The process works as follows:

1) When a memory consumer wishes to allocate memory, it asks the memory manager
   for the desired number of bytes.
2) If sufficient memory is available, the memory manager allocates and returns
   a memory resource.
3) If memory is low, the memory manager asks each of the memory consumers from
   the least recently used to the most recently used to free up memory
   resources until sufficient memory is available to satisfy the request.
*/
class PYXLIB_DECL MemoryManager
{
public:

	//! Unit test method
	static void test();

	//! Get the singleton instance of the memory manager.
	static MemoryManager* getInstance();

	// TODO: Returns bald owner pointer; consider changing to auto_ptr.
	//! Request a memory resource from the memory manager.
	MemoryResource* requestMemory(size_t nBytes);

	// TODO: Returns bald owner pointer; consider changing to auto_ptr.
	//! Request a memory token stating we have allocated memory ourselves.
	MemoryToken* requestToken(size_t nBytes);

	double getOtherMemoryUsagePercent() const
	{
		return (100.0*(m_nOtherMemoryAllocated))/m_nMaximumAllocation;	
	}

	double getMemoryUsagePercent() const
	{
		return (100.0*(m_nMemoryAllocated))/m_nMaximumAllocation;	
	}

	VaryingMemoryUsed & getMemoryUsageTopic(const std::string & name);

	std::map<std::string,long> getMemoryStatus();

private:

	//! Default constructor
	MemoryManager();

	//! Destructor
	virtual ~MemoryManager() {}

	//! Disable copy constructor
	MemoryManager(const MemoryManager&);

	//! Disable copy assignment
	void operator=(const MemoryManager&);

	//! Ask memory consumers to free some memory.
	void freeConsumerMemory();

	//! Register a memory consumer.
	void registerConsumer(MemoryConsumer* pConsumer);

	//! Unregister a memory consumer.
	void unregisterConsumer(MemoryConsumer* pConsumer);

	//! Notify memory manager that an amount of memory has been freed.
	void memoryReleased(size_t nAmtReleased);

	//! Notify memory manager that an amount of memory has been allocated.
	void memoryAllocated(size_t nAmtAllocated);

	//! Update the memory allocated outside the process manager and call free memory if needed
	void guardMemoryUsage();

	//! Pointer to previous new handler function.
	static _PNH	m_oldNewHandler;

	//! Release a memory resource
	void releaseResource(MemoryResource* pResource);

	//! Release a memory token.
	void releaseResource(MemoryToken* pToken);

	//! Initialize any static data
	static void initStaticData();

	//! Free singleton instance
	static void freeStaticData();

	//! Singleton instance of the memory manager
	static MemoryManager* m_pInstance;

	//! Typedef for collection of memory consumers.
	typedef std::vector<MemoryConsumer*> MemoryConsumerVector;

	//! collection of memory consumers.
	MemoryConsumerVector m_vecConsumers;

	//! Amount of memory in bytes that has been allocated.
	size_t m_nMemoryAllocated;

	//! Maximum amount of memory that we can allocate.
	size_t m_nMaximumAllocation;

	//! Next consumer to free memory.
	size_t m_nNextConsumerToFreeMemory;

	//! Total memory allocated for the procues outside of the MemoryManager control
	size_t m_nOtherMemoryAllocated;

	//! Total memory availble for the process (max allocation)
	size_t m_nTotalMemoryAvailable;

	//! Initial memory used by the code and os when the memorymanager was created
	size_t m_nFreeMemory;

	//! Initial memory used by the code and os when the memorymanager was created
	size_t m_nOSnCodeUsed;

	//! Initial memory used by the code and os when the memorymanager was created
	size_t m_nSqliteAllocated;

	//! All allocated memory (including memory not managed by the memoery manager)
	size_t m_heapAllocated;

	//! All free memory space (defrag heap) in process heap
	size_t m_heapUnallocated;

	//! Number of allocations betweens updates of m_nOtherMemoryAllocated
	int m_ticksToUpdateOtherMemoryAllocated;

	//! not if we have memory release thread task running or not
	bool m_releasingMemory;

	std::map<std::string,boost::shared_ptr<VaryingMemoryUsed>> m_usedSections;

	//! Ensure MemoryResource and MemoryToken can see private methods.
	friend class MemoryResource;
	friend class MemoryToken;
	friend class MemoryConsumer;
	friend class VaryingMemoryUsed;


	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

template<typename T>
class ObjectMemoryUsageCounter
{
	struct State
	{
		boost::detail::atomic_count instanceCount;
		long previousInstanceCount;
		size_t objectSize;
		long thresholdCount;
		boost::mutex mutex;
		VaryingMemoryUsed * memoryUsed;

		State(size_t size) : instanceCount(0), previousInstanceCount(0), objectSize(size), memoryUsed(nullptr)
		{
			//each object have 8 bytes of memory overhead + align to 8 bytes size.
			objectSize += (objectSize % 8) ? (16-(objectSize % 8)) : 8;
			thresholdCount = (long)(1024*1024 / objectSize); //notify memory manager on every 1MB memory usage
		}

		void notifyMemoryManager()
		{
			boost::mutex::scoped_lock lock(mutex);

			long newValue = instanceCount;
			//recheck the change...
			if (abs(newValue - previousInstanceCount) > thresholdCount)
			{				
				if (memoryUsed == nullptr) {
					memoryUsed = &(::MemoryManager::getInstance()->getMemoryUsageTopic(typeid(T).name()));
				}
				memoryUsed->memoryChangedDelta((newValue - previousInstanceCount)*objectSize);
				previousInstanceCount = newValue;
			}
		}

		void memoryChangedDelta(int size) 
		{
			boost::mutex::scoped_lock lock(mutex);

			if (memoryUsed == nullptr) {
				memoryUsed = &(::MemoryManager::getInstance()->getMemoryUsageTopic(typeid(T).name()));
			}

			memoryUsed->memoryChangedDelta(size);
		}
	};

protected:
	ObjectMemoryUsageCounter()
	{
		incrementMemoryUsage();
	}

	ObjectMemoryUsageCounter(const ObjectMemoryUsageCounter & other)
	{
		incrementMemoryUsage();
	}

	virtual ~ObjectMemoryUsageCounter()
	{
		decrementMemoryUsage();
	}

private:
	static State & getMemoryUsageState()
	{
		static State state(sizeof(T));
		return state;
	}

	static void incrementMemoryUsage()
	{
		State & state = getMemoryUsageState();

		if (abs((++state.instanceCount) - state.previousInstanceCount) > state.thresholdCount)
		{
			state.notifyMemoryManager();
		}
	}

	static void decrementMemoryUsage()
	{
		State & state = getMemoryUsageState();

		if (abs(state.previousInstanceCount - (--state.instanceCount)) > state.thresholdCount)
		{
			state.notifyMemoryManager();
		}

		assert(state.instanceCount>=0);
	}

public:
	static void consumeMemory(int size)
	{
		State & state = getMemoryUsageState();
		state.memoryChangedDelta(size);
	}

	static void releaseMemory(int size)
	{
		State & state = getMemoryUsageState();
		state.memoryChangedDelta(-size);
	}
};


/*!
Helper class designed to test the memory manager. By acting as a 
test memory consumer. This class behaves as a memory consumer should 
and conforms to the expecations of a memory manager. That is to say, 
that when a request is made to free memory it will always free 
memory. Every call to get a memory resource is guaranteed to work. 
Every call to get a memory token will also work.
*/
//! Designed for testing the memory management policy.
class PYXLIB_DECL MemoryManagerTestConsumer : public MemoryConsumer
{
public:
	
	//! Constants
	static const size_t knAllocationSize = 
		static_cast<size_t>(100E6); // allocate memory in 100MB slices.

	//! Default Constructor
	MemoryManagerTestConsumer();

	//! Default destructor
	virtual ~MemoryManagerTestConsumer();

	//! Allocate some memory and write to it.
	void allocateAndWrite();

	//! Allocate some memory that uses a token.
	void allocateToken(size_t nTokenSize = knAllocationSize);

	//! Called by the memory manager to free up memory.
	virtual void freeMemory();

	//! Method to free all memory currently allocated.
	void freeAllMemoryConsumed();

	//! Get a pointer to a resource that has been created.
	MemoryResource* getAResource();

	//! Get a pointer to a token in the token vector.
	MemoryToken* getAToken();

	//! Get the size of a request for memory.
	size_t getAllocationSize() const { return knAllocationSize; }
	
	//! Get flag indicating if we have freed memory.
	bool getMemFreedFlag() const { return m_bMemFreed; }

	//! Reset the flag indicating we have freed memory.
	void resetMemFreedFlag()
	{
		m_bMemFreed = false;
	}
	
private:

	//! Helper method to free resources.
	void freeResource(); 

	//! Helper method to free tokens.
	void freeToken();

	//! Queue to hold onto memory allocated. 
	std::queue<MemoryResource*> m_queMemPtrs;
	
	//! Queue to hold onto tokens allocated.
	std::queue<MemoryToken*> m_queMemTokens;

	//! Ptr to memory manager.
	MemoryManager* m_pMemManager;

	//! Indicate that the Memory Manager asked us to free mem.
	bool m_bMemFreed;

	boost::recursive_mutex m_mutex;
};

#endif // guard
