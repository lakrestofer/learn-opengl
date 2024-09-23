#ifndef UTIL_HEADER_DEFINED
#define UTIL_HEADER_DEFINED

#define FOR(v, n) for (int v = 0; v < (int) (n); v++)

#define DEFINE_SLICE_STRUCT(type) typedef struct { type* ptr; int len; } type##Slice;


#endif
