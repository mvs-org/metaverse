
#ifndef ___STRING_VIEW__
#define ___STRING_VIEW__

#ifdef _WIN32
#include <boost/utility/string_view_fwd.hpp>
#include <boost/utility/string_view.hpp>
#define string_view boost::string_view 

#else

#include <experimental/string_view>
#define string_view std::experimental::string_view 

#endif

#endif
