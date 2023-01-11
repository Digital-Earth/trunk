#ifndef PYXIS__UTILITY__STL_UTILS_H
#define PYXIS__UTILITY__STL_UTILS_H
/******************************************************************************
stl_utils.h

begin		: 2005-08-08
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

/*!
DereferenceLess is a comparison function designed to be used with associative
containers of pointers. It ensures the pointers are dereferenced before
comparing. To use it, supply DereferenceLess as the Traits argument when
constructing your associative container. For example:

\code
// construct a set of pointers that sorts properly
std::set(PointerType, DereferenceLess> sortedSet;
\endcode
*/
//! Ensures associative containers of pointers are sorted properly.
struct PYXLIB_DECL DereferenceLess
{
    template <typename T>
    bool operator()(const T ptr1, const T ptr2) const
    {
        return *ptr1 < *ptr2;
    }
};

#endif // guard
