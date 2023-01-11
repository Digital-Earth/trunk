#if !defined(PYXIS__ASSERT)
#define PYXIS__ASSERT

#include <cassert>

// Causes an assertion failure and returns false, allowing use in conditionals.
#define PYXIS__ASSERT___FAIL(MESSAGE) (assert(0 && (MESSAGE)), false)

#endif
