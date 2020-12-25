#ifndef __COUNT_H__
#define __COUNT_H__


#if defined(__clang__) || defined(__GNUC__)

#define GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, n, ...) n

#define COUNT_ARGS(...)      \
    GET_NTH_ARG(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

/*
    for GET_ARG_COUNT(a, b, c) will unfold as
    GET_NTH_ARG(a,  b,  c,  10,  9,  8,  7,  6,  5,  4,  3,  2,  1) compared with
    GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, n) -> n = 3
*/

#elif defined(_MSC_VER)

#define CONCAT_DETAIL(l, r) l##r
#define CONCAT(l, r) CONCAT_DETAIL(l, r)
#define COUNT_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define COUNT_M(...) EXPAND(COUNT_N( __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define CALL(X,Y) X Y
#define EXPAND(...) __VA_ARGS__
#define CLASS_BODY(...) CONCAT(EXPAND(COUNT(__VA_ARGS__)),_DERIVED)(__VA_ARGS__)
#define COUNT_ARGS(...) CALL(COUNT_M,(, __VA_ARGS__))

#endif 

#endif //  __COUNT_H__