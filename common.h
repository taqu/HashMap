#ifndef INC_HASHMAP_COMMON_H__
#define INC_HASHMAP_COMMON_H__
/**
@file common.h
@author t-sakai
@date 2017/05/14 create
*/
#include <cassert>
#include <cstring>
#include <utility>
#include <cstdint>
#include <malloc.h>

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void*)0)
    #endif
#endif

#define HASSERT(exp) assert(exp)

#define HNEW new
#define HPLACEMENT_NEW(ptr) new(ptr)
#define HDELETE(ptr) delete (ptr); (ptr)=NULL
#define HDELETE_RAW(ptr) delete (ptr)
#define HDELETE_ARRAY(ptr) delete[] (ptr); (ptr)=NULL

namespace hashmap
{
    //---------------------------------------------------------
#if defined(_MSC_VER)
    typedef char Char;
    typedef unsigned char UChar;

    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t size_t;

#elif defined(__GNUC__)
    typedef char Char;
    typedef unsigned char UChar;

    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t size_t;

#else
    typedef char Char;
    typedef unsigned char UChar;

    typedef char s8;
    typedef short s16;
    typedef long s32;
    typedef long long s64;

    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned long u32;
    typedef unsigned long long u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t lsize_t;

#endif

    //---------------------------------------------------------
    using std::move;
    using std::forward;
    using std::swap;
    using std::true_type;
    using std::false_type;
    using std::declval;

    //---------------------------------------------------------
    struct DefaultAllocator
    {
        static inline void* malloc(hashmap::u64 size)
        {
            return ::malloc(size);
        }

        static inline void free(void* mem)
        {
            ::free(mem);
        }
    };

#define HALLOCATOR_MALLOC(allocator, size) allocator::malloc(size)

#define HALLOCATOR_FREE(allocator, ptr) allocator::free((ptr));(ptr)=NULL


    template<class T>
    inline T* construct(void* ptr)
    {
        return HPLACEMENT_NEW(ptr) T();
    }

    template<class T>
    inline T* construct(void* ptr, const T& x)
    {
        return HPLACEMENT_NEW(ptr) T(x);
    }

    template<class T>
    inline T* construct(void* ptr, T&& x)
    {
        return HPLACEMENT_NEW(ptr) T(std::move(x));
    }

    template<class Itr>
    struct iterator_traits
    {
        typedef typename Itr::iterator_category iterator_category;
        typedef typename Itr::value_type value_type;
        typedef typename Itr::difference_type difference_type;
        typedef typename Itr::difference_type distance_type;
        typedef typename Itr::pointer pointer;
        typedef typename Itr::reference reference;
    };

    template<class T>
    struct iterator_traits<T*>
    {
        //typedef random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef ptrdiff_t distance_type;	// retained
        typedef T *pointer;
        typedef T& reference;
    };

    template<class FwdIt, class T>
    FwdIt lower_bound(FwdIt first, FwdIt last, const T& val)
    {
        typename iterator_traits<FwdIt>::difference_type count = last - first;
        while(0<count){
            typename iterator_traits<FwdIt>::difference_type d = count/2;
            FwdIt m = first + d;
            if(*m<val){
                first = m+1;
                count -= d+1;
            } else{
                count = d;
            }
        }
        return first;
    }
}
#endif //INC_HASHMAP_COMMON_H__
