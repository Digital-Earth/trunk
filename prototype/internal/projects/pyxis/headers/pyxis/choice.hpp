#if !defined(PYXIS__CHOICE)
#define PYXIS__CHOICE

namespace Pyxis
{
	template < bool flag, typename True, typename False >
	struct Choice;

	template < typename True, typename False >
	struct Choice< true, True, False >
	{
	   typedef True type;
	};

	template < typename True, typename False >
	struct Choice< false, True, False >
	{
	   typedef False type;
	};
}

#endif
