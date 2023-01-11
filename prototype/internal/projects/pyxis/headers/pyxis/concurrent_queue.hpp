#if !defined(PYXIS__CONCURRENT_QUEUE)
#define PYXIS__CONCURRENT_QUEUE

#include "pyxis/pointee.hpp"
#include <boost/thread.hpp>
#include <queue>

namespace Pyxis
{
	// A thread-safe queue that handles multiple consumers.
	template< typename Data > class ConcurrentQueue;
}

// Implementation from:
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template< typename Data > class Pyxis::ConcurrentQueue :
public virtual Pointee,
boost::noncopyable // TODO (mutex is noncopyable)
{
    std::queue< Data > queue;
    mutable boost::mutex mutex;
    boost::condition_variable condition;

public:

    void push(Data const & data)
    {
    	{
	        boost::mutex::scoped_lock lock(mutex);
    	    queue.push(data);
    	}
        condition.notify_one();
    }

    bool getIsEmpty() const
    {
        boost::mutex::scoped_lock lock(mutex);
        return queue.empty();
    }

    bool tryPop(Data & poppedValue)
    {
        boost::mutex::scoped_lock lock(mutex);
        if (queue.empty())
        {
            return false;
        }
        poppedValue = queue.front();
        queue.pop();
        return true;
    }

    void waitAndPop(Data & poppedValue)
    {
        boost::mutex::scoped_lock lock(mutex);
        while (queue.empty())
        {
            condition.wait(lock);
        }
        poppedValue = queue.front();
        queue.pop();
    }
};

#endif
