#ifndef __hashdef_h
#define __hashdef_h

#ifdef __GNUC__
  #define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef __clang__
  #if __has_include(<unordered_map>)
    #define HAS_STD_UNORDERED_MAP
  #elif __has_include(<tr1/unordered_map>)
    #define HAS_TR1_UNORDERED_MAP
  #endif
#elif defined(__GNUC__)
  #if __cplusplus >= 201103L && defined(__GLIBCXX__) && __GLIBCXX__ > 20090421
    #define HAS_STD_UNORDERED_MAP
  #elif GCC_VERSION >= 40201
    #define HAS_TR1_UNORDERED_MAP
  #else
    #define HAS_GCC_HASH_MAP
  #endif
#elif __cplusplus > 199711L || _MSC_VER >= 1600
  #define HAS_STD_UNORDERED_MAP
#else
  #error This library needs a compiler which supports either unordered_map or ext/hash_map.
#endif

#ifdef HAS_STD_UNORDERED_MAP
  #include <unordered_map>
  #include <unordered_set>
  #define _HASHMAP_NS std
  #define _HASHMAP_NAME unordered_map
  #define _HASHSET_NAME unordered_set
#elif defined(HAS_TR1_UNORDERED_MAP)
  #include <tr1/unordered_map>
  #include <tr1/unordered_set>
  #define _HASHMAP_NS std::tr1
  #define _HASHMAP_NAME unordered_map
  #define _HASHSET_NAME unordered_set
#elif defined(HAS_GCC_HASH_MAP)
  #include <ext/hash_map>
  #include <ext/hash_set>
  #define _HASHMAP_NS __gnu_cxx
  #define _HASHMAP_NAME hash_map
  #define _HASHSET_NAME hash_set
#endif

#define _HASHMAP _HASHMAP_NS::_HASHMAP_NAME
#define _HASHSET _HASHMAP_NS::_HASHSET_NAME

#ifdef HAS_GCC_HASH_MAP
namespace _HASHMAP_NS {
	template<> struct hash<std::string> {
		size_t operator()(const std::string& x) const {
			return hash<const char*>()( x.c_str() );
		}
	};
}
#endif

#endif // #ifndef __hashdef_h
