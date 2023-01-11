#if !defined(PYXIS__SET)
#define PYXIS__SET

#include "pyxis/set_interface.hpp"
#include "pyxis/visit.hpp"
#include <set>

namespace Pyxis
{
	template < typename Value > class Set;
}

#include "pyxis/forward_range_interface.hpp"

template < typename Value >
class Pyxis::Set :
public virtual MutableSetInterface< Value >
{
	typedef typename std::set< Value >::const_iterator Iterator;

	template < typename Return >
	class VisitorAdapter
	{
		FunctorInterface< Return, Value > & functor;

	public:

		Return operator ()(typename std::set< Value >::const_iterator visitee)
		{
			return functor(*visitee);
		}

		explicit VisitorAdapter(FunctorInterface< Return, Value > & functor) :
		functor(functor) {}
	};

	std::set< Value > set;

public:

	class Elements :
	public virtual ForwardRangeInterface< Value >
	{
		typename Set::Iterator iterator;
		typename Set::Iterator const end;

	public:

		explicit Elements(Set const & set) :
		iterator(set.set.begin()),
		end(set.set.end())
		{}

		bool getIsEmpty() const
		{
			return iterator == end;
		}
		
		// Asserts that it is non-empty.
		void popFront()
		{
			assert(iterator != end);
			++iterator;
		}

		// Asserts that it is non-empty.
		Value getFront() const
		{
			assert(iterator != end);
			return *iterator;
		}
	};

	explicit Set() : set() {}

	bool operator ==(Set const & that) const
	{
		return set == that.set;
	}
	
	bool operator !=(Set const & that) const
	{
		return set != that.set;
	}
	
	void swap(Set & that)
	{
		set.swap(that.set);
	}

	std::ostream & write(std::ostream & output) const
	{
		output << "{";
		if (!this->set.empty())
		{
			Iterator const end = this->set.end();
			for (Iterator iterator = this->set.begin(); ; )
			{
				assert(iterator != end);
				output << *iterator;
				if (++iterator == end) break;
				output << ", ";
			}
		}
		output << "}";
		return output;
	}

	friend std::ostream & operator <<(std::ostream & output, Set const & that)
	{
		return that.write(output);
	}

	void setIsEmpty()
	{
		set.clear();
	}

	// For each element in the set:
	// if it is not in results, insert it and call callback.
	bool visit(FunctorInterface< bool, Value > & callback,
		MutableSetInterface< Value > & results) const
	{
		// For each value mapped to this subtree, add it to the result set.
		for (typename Set< Value >::Elements elements(*this);
			!elements.getIsEmpty(); elements.popFront())
		{
			Value value = elements.getFront();
			if (!results.find(value))
			{
				results.insert(value);
				if (!callback(value))
				{
					return false;
				}
			}
		}
		return true;
	}

	bool visit(FunctorInterface< bool, Value > & callback) const
	{
		VisitorAdapter< bool > adapter(callback);
		return Visit::forward(set.begin(), set.end(), adapter);
	}

	void visitAll(FunctorInterface< void, Value > & callback) const
	{
		VisitorAdapter< void > adapter(callback);
		Visit::allForward(set.begin(), set.end(), adapter);
	}
	
public: // MutableSetInterface< Value >

	using MutableSetInterface< Value >::insert;
	
	void insert(Value element)
	{
		set.insert(element);
	}
	
	using MutableSetInterface< Value >::remove;
	
	void remove(Value element)
	{
		set.erase(element);
	}

	void intersect(SetInterface< Value > const & intersectee)
	{
		// Create intersection:
		// start with empty set, 
		// and add only those from this set that are in intersectee.
		Set intersection;
		Iterator const end = set.end();
		for (Iterator iterator = set.begin(); iterator != end; ++iterator)
		{
			Value value = *iterator;
			if (intersectee.find(value)) { intersection.insert(value); }
		}
		
		// Swap intersection with this.
		swap(intersection);
	}

public: // SetInterface< Value >

	boost::intrusive_ptr<
		ForwardRangeInterface< Value > > getElements() const
	{
		return new Elements(*this);
	}

	bool getIsEmpty() const
	{
		return set.empty();
	}

	size_t getCount() const
	{
		return set.size();
	}

	bool find(Value element) const
	{
		return set.find(element) != set.end();
	}
};
	
#endif
