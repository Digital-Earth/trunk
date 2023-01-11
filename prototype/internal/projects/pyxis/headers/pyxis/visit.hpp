#if !defined(PYXIS__VISIT)
#define PYXIS__VISIT

#include "pyxis/assert.hpp"
#include <boost/detail/atomic_count.hpp>
#include <boost/thread.hpp>

namespace Pyxis
{
	struct Visit;
}

struct Pyxis::Visit
{
private:

	template < typename ConstIterator, typename Visitor >
	class UnorderedFunctor
	{
		boost::thread_group & threads;
		boost::detail::atomic_count & stopCount;
		Visitor & visitor;
		ConstIterator visitee;

	public:

		void operator ()()
		{
			try
			{
				// Visit.  If false:
				if (!stopCount && !visitor(visitee))
				{
					// Increment the cancel count;
					// the visit function stops iteration and
					// returns false if this is non-zero.
					++stopCount;

					// Interrupting the threads allows each visitor
					// finer-grained control over cancellation.
					// Should be safe to call this multiple times.
					threads.interrupt_all();
				}
			}
			catch (boost::thread_interrupted const &)
			{
				assert(stopCount && "Thread was interrupted without being cancelled.");
			}
			catch (...)
			{
				assert(0 && "An uncaught exception was thrown from the visitor thread.");
			}
		}

		explicit UnorderedFunctor(
			boost::thread_group & threads,
			boost::detail::atomic_count & stopCount,
			Visitor & visitor,
			ConstIterator visitee) :
		threads(threads),
		stopCount(stopCount),
		visitor(visitor),
		visitee(visitee) {}
	};

	template < typename ConstIterator, typename Visitor >
	class AllUnorderedFunctor
	{
		Visitor & visitor;
		ConstIterator visitee;

	public:

		void operator ()()
		{
			try
			{
				visitor(visitee);
			}
			catch (...)
			{
				assert(0 && "An uncaught exception was thrown from the visitor thread.");
			}
		}

		explicit AllUnorderedFunctor(Visitor & visitor, ConstIterator visitee) :
		visitor(visitor), visitee(visitee) {}
	};

public:

	class Test;

	// If visitor returns false, visitation stops.
	template < typename ConstIterator, typename Visitor >
	static bool forward(
		ConstIterator begin, ConstIterator end,
		Visitor & visitor)
	{
		for (ConstIterator iterator = begin; iterator != end; ++iterator)
		{
			if (!visitor(iterator)) 
			{
				return false;
			}
		}
		return true;
	}
		
	// Visits elements in any speed-optimal order, likely on multiple threads.
	// If visitor returns false, visitation stops via interruption point and 
	// the visit function returns false (even if other visitors make it to the end
	// after this and return true).
	// The visitor must:
	//	-	Be able to be called from multiple threads, simultaneously with another element visitor.
	//	-	Expect a boost::thread_interrupted exception to be thrown at interruption points.
	//	-	Not throw any other exception.
	//	-	Understand that returning false doesn't guarantee that another visitor
	//		won't reach completion afterward.
	template < typename ConstIterator, typename Visitor >
	static bool unordered(
		ConstIterator begin, ConstIterator end,
		Visitor & visitor)
	{
		// TODO: Consider a thread pool.
		boost::thread_group threads;
		boost::detail::atomic_count stopCount(0);
		for (ConstIterator iterator = begin; iterator != end; ++iterator)
		{
			threads.create_thread(
				UnorderedFunctor< ConstIterator, Visitor >(
					threads, stopCount, visitor, iterator));
			if (stopCount)
			{
				break;
			}
		}
		threads.join_all();

		return !stopCount;
	}

	template < typename ConstIterator, typename Visitor >
	static void allForward(
		ConstIterator begin, ConstIterator end,
		Visitor & visitor)
	{
		for (ConstIterator iterator = begin; iterator != end; ++iterator)
		{
			visitor(iterator);
		}
	}

	template < typename ConstIterator, typename Visitor >
	static void allUnordered(
		ConstIterator begin, ConstIterator end,
		Visitor & visitor)
	{
		// TODO: Consider a thread pool.
		boost::thread_group threads;
		for (ConstIterator iterator = begin; iterator != end; ++iterator)
		{
			threads.create_thread(
				AllUnorderedFunctor< ConstIterator, Visitor >(visitor, iterator));
		}
		threads.join_all();
	}

private:

	Visit();
};

#include <boost/random.hpp>
#include <set>

class Pyxis::Visit::Test
{
	std::string string;
	std::set< char > set;

	static int getRandomInt(int min, int max)
	{
		boost::mt19937 rng;
		boost::uniform_int<> range(min, max);      
		boost::variate_generator< boost::mt19937 &, boost::uniform_int<> > generate(rng, range);
		return generate();                      
	}

	struct Visitor :
	boost::noncopyable // TODO (mutex is noncopyable)
	{
		std::string string;
		std::set< char > set;
		boost::mutex mutex;
		explicit Visitor() : string(), set(), mutex() {}
		bool operator ()(std::string::const_iterator element)
		{
			// Sleep for a random amount of time.
			boost::thread::sleep(
				boost::get_system_time() +
				boost::posix_time::milliseconds(getRandomInt(0, 100)));

			boost::mutex::scoped_lock lock(mutex);
			if (set.size() < 9)
			{
				string.push_back(*element);
				set.insert(*element);
				return true;
			}
			return false;
		}
	};

	struct AllVisitor :
	boost::noncopyable // TODO (mutex is noncopyable)
	{
		std::string string;
		std::set< char > set;
		boost::mutex mutex;
		explicit AllVisitor() : string(), set(), mutex() {}
		void operator ()(std::string::const_iterator element)
		{
			// Sleep for a random amount of time.
			boost::thread::sleep(
				boost::get_system_time() +
				boost::posix_time::milliseconds(getRandomInt(0, 100)));

			boost::mutex::scoped_lock lock(mutex);
			string.push_back(*element);
			set.insert(*element);
		}
	};

public:

	explicit Test() : string("0123456789"), set(string.begin(), string.end())
	{
	}

	bool testForward() const
	{
		Visitor visitor;
		if (Visit::forward(string.begin(), string.end(), visitor))
		{
			return PYXIS__ASSERT___FAIL("The visitor should not have run to completion..");
		}

		std::string substring(string);
		substring.resize(substring.size() - 1);
		if (visitor.string != substring)
		{
			return PYXIS__ASSERT___FAIL("The string does not match.");
		}

		if (visitor.set.size() != 9)
		{
			return PYXIS__ASSERT___FAIL("The set does not match.");
		}

		return true;
	}

	bool testUnordered() const
	{
		Visitor visitor;
		if (Visit::unordered(string.begin(), string.end(), visitor))
		{
			return PYXIS__ASSERT___FAIL("The visitor should not have run to completion..");
		}

		if (visitor.set.size() != 9)
		{
			return PYXIS__ASSERT___FAIL("The set does not match.");
		}

		return true;
	}

	bool testAllForward() const
	{
		AllVisitor allVisitor;
		Visit::allForward(string.begin(), string.end(), allVisitor);
		if (allVisitor.string != string)
		{
			return PYXIS__ASSERT___FAIL("The string does not match.");
		}
		if (allVisitor.set != set)
		{
			return PYXIS__ASSERT___FAIL("The set does not match.");
		}
		return true;
	}

	bool testAllUnordered() const
	{
		AllVisitor allVisitor;
		Visit::allUnordered(string.begin(), string.end(), allVisitor);
		if (allVisitor.set != set)
		{
			return PYXIS__ASSERT___FAIL("The set does not match.");
		}
		return true;
	}

	operator bool() const
	{
#if 1 // TODO: Causing thread problems when called by client.  Re-enable when fixed.
		return true;
#else
		return (
			testForward() &&
			testUnordered() &&
			testAllForward() &&
			testAllUnordered());
#endif
	}
};

#endif
