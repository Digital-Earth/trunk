#if !defined(PYXIS__SET_INTERFACE)
#define PYXIS__SET_INTERFACE

namespace Pyxis
{
	template < typename Value > struct ForwardRangeInterface;
	template < typename Return, typename Argument > struct FunctorInterface;

	template < typename Value > struct SetInterface;
	template < typename Value > struct MutableSetInterface;
}

#include "pyxis/collection_interface.hpp"
#include <boost/intrusive_ptr.hpp>

// A set of values.  Value is a value type.
template < typename Value >
struct Pyxis::SetInterface :
virtual CollectionInterface
{
	virtual boost::intrusive_ptr<
		ForwardRangeInterface< Value > > getElements() const = 0;

	virtual bool find(Value value) const = 0;
	
#if 0 // TODO: Add these back in as necessary
	virtual bool visit(FunctorInterface< bool, Value > & visitor) const = 0;
	virtual void visitAll(FunctorInterface< void, Value > & visitor) const = 0;
#endif
};

template < typename Value >
struct Pyxis::MutableSetInterface :
virtual SetInterface< Value >,
virtual MutableCollectionInterface
{
	virtual void insert(Value element) = 0;

	virtual void insert(SetInterface< Value > const & set)
	{
		boost::intrusive_ptr<
			ForwardRangeInterface< Value > > elements(set.getElements());
		assert(elements);
		for (; !elements->getIsEmpty(); elements->popFront())
		{
			this->insert(elements->getFront());
		}
	}

	virtual void remove(Value element) = 0;
	
	virtual void remove(SetInterface< Value > const & set)
	{
		boost::intrusive_ptr<
			ForwardRangeInterface< Value > > elements(set.getElements());
		assert(elements);
		for (; !elements->getIsEmpty(); elements->popFront())
		{
			this->remove(elements->getFront());
		}
	}

	virtual void intersect(SetInterface< Value > const & intersectee) = 0;
};

#endif
