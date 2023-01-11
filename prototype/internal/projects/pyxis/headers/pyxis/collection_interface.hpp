#if !defined(PYXIS__COLLECTION_INTERFACE)
#define PYXIS__COLLECTION_INTERFACE

namespace Pyxis
{
	struct CollectionInterface;

	struct MutableCollectionInterface;
}

#include "pyxis/pointee.hpp"

struct Pyxis::CollectionInterface : virtual Pointee
{
	virtual bool getIsEmpty() const = 0;

	virtual size_t getCount() const = 0;
};

struct Pyxis::MutableCollectionInterface : virtual CollectionInterface
{
	virtual void setIsEmpty() = 0;
};

#endif
