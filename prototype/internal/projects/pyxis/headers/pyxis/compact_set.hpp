#if !defined(PYXIS__COMPACT_SET)
#define PYXIS__COMPACT_SET

namespace Pyxis
{
	template < typename Value > class CompactSet;
}

#include "pyxis/set.hpp"

// A set, optimized for space.
template < typename Value >
class Pyxis::CompactSet :
public virtual MutableSetInterface< Value >
{
	// TODO: See if there is a more space-optimal unspecialized implementation.
	Set< Value > set;

public:

	bool operator ==(CompactSet const & that) const
	{
		return this->set == that.set;
	}
	
	bool operator !=(CompactSet const & that) const
	{
		return this->set != that.set;
	}
	
	std::ostream & write(std::ostream & output) const
	{
		return this->set.write(output);
	}

	friend std::ostream & operator <<(std::ostream & output, CompactSet const & set)
	{
		return set.write(output);
	}

	void swap(CompactSet & that)
	{
		return this->set.swap(that.set);
	}

	// For each element in the set:
	// if it is not in results, insert it and call callback.
	bool visit(FunctorInterface< bool, Value > & callback,
		MutableSetInterface< Value > & results) const
	{
		return this->set.visit(callback, results);
	}
	
public: // CollectionInterface

	bool getIsEmpty() const
	{
		return this->set.getIsEmpty();
	}

	size_t getCount() const
	{
		return this->set.getCount();
	}

public: // MutableCollectionInterface

	void setIsEmpty()
	{
		this->set.setIsEmpty();
	}

public: // SetInterface< Value >

	boost::intrusive_ptr<
		ForwardRangeInterface< Value > > getElements() const
	{
		return this->set.getElements();
	}
	
	bool find(Value element) const
	{
		return this->set.find(element);
	}

public: // MutableSetInterface< Value >
	
	using MutableSetInterface< Value >::insert;
	
	void insert(Value element)
	{
		this->set.insert(element);
	}

	using MutableSetInterface< Value >::remove;
	
	void remove(Value element)
	{
		this->set.remove(element);
	}

	void intersect(SetInterface< Value > const & intersectee)
	{
		this->set.intersect(intersectee);
	}
};

// TODO: Specialize on size_t as sparse bit vector.
// Consider http://judy.sourceforge.net/doc/Judy1_3x.htm

#endif
