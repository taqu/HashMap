/*
*  xxHash - Fast Hash algorithm
*  Copyright (C) 2012-2016, Yann Collet
*
*  BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are
*  met:
*
*  * Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above
*  copyright notice, this list of conditions and the following disclaimer
*  in the documentation and/or other materials provided with the
*  distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*  You can contact the author at :
*  - xxHash homepage: http://www.xxhash.com
*  - xxHash source repository : https://github.com/Cyan4973/xxHash
*/
/**
@file xxHash.cpp
@author t-sakai
@date 2017/05/14 create
*/
#include "xxHash.h"

namespace hashmap
{
//----------------------------------------------------
//---
//--- xxHash
//---
//----------------------------------------------------
    namespace
    {
        static const u32 PRIME32_1 = 2654435761U;
        static const u32 PRIME32_2 = 2246822519U;
        static const u32 PRIME32_3 = 3266489917U;
        static const u32 PRIME32_4 = 668265263U;
        static const u32 PRIME32_5 = 374761393U;

        static const u64 PRIME64_1 = 11400714785074694791UL;
        static const u64 PRIME64_2 = 14029467366897019727UL;
        static const u64 PRIME64_3 = 1609587929392839161UL;
        static const u64 PRIME64_4 = 9650029242287828579UL;
        static const u64 PRIME64_5 = 2870177450012600261UL;


        static inline u32 rotl32(u32 x, s32 r)
        {
            return (x<<r) | (x>>(32-r));
        }

        static inline u64 rotl64(u64 x, s32 r)
        {
            return (x << r) | (x >> (64 - r));
        }

        static inline u32 round32(u32 seed, u32 x)
        {
            seed += x * PRIME32_2;
            seed = rotl32(seed, 13);
            seed *= PRIME32_1;
            return seed;
        }

        static inline u64 round64(u64 seed, u64 x)
        {
            seed += x * PRIME64_2;
            seed = rotl64(seed, 31);
            seed *= PRIME64_1;
            return seed;
        }

        static inline u64 mergeRound64(u64 seed, u64 x)
        {
            x  = round64(0, x);
            seed ^= x;
            seed  = seed * PRIME64_1 + PRIME64_4;
            return seed;
        }

        static inline u32 read32LE(const void* ptr)
        {
            return *((u32*)ptr);
        }

        static inline u64 read64LE(const void* ptr)
        {
            return *((u64*)ptr);
        }

        static const s32 Aligned = 0;
        static const s32 Unaligned = 1;
        template<s32 T>
        inline u32 read32LEImpl(const u8* data)
        {
            return 0;
        }

        template<>
        inline u32 read32LEImpl<Aligned>(const u8* data)
        {
            return *((u32*)data);
        }

        template<>
        inline u32 read32LEImpl<Unaligned>(const u8* data)
        {
            u32 r0 = data[0];
            u32 r1 = data[1];
            u32 r2 = data[2];
            u32 r3 = data[3];
            return r0 | (r1<<8) | (r2<<16) | (r3<<24);
        }

        template<s32 T>
        inline u64 read64LEImpl(const u8* data)
        {
            return 0;
        }

        template<>
        inline u64 read64LEImpl<Aligned>(const u8* data)
        {
            return *((u64*)data);
        }

        template<>
        inline u64 read64LEImpl<Unaligned>(const u8* data)
        {
            u64 x;
            memcpy(&x, data, sizeof(u64));
            return x;
        }


        template<s32 T>
        inline u32 xxHash32Impl(const u8* data, s32 length, u32 seed)
        {
            const u8* ptr = data;
            const u8* end = ptr + length;
            u32 h32;

            if(length>=16){
                const u8* const limit = end - 16;
                u32 v1 = seed + PRIME32_1 + PRIME32_2;
                u32 v2 = seed + PRIME32_2;
                u32 v3 = seed + 0;
                u32 v4 = seed - PRIME32_1;

                do{
                    v1 = round32(v1, read32LEImpl<T>(ptr)); ptr+=4;
                    v2 = round32(v2, read32LEImpl<T>(ptr)); ptr+=4;
                    v3 = round32(v3, read32LEImpl<T>(ptr)); ptr+=4;
                    v4 = round32(v4, read32LEImpl<T>(ptr)); ptr+=4;
                }while(ptr<=limit);

                h32 = rotl32(v1, 1) + rotl32(v2, 7) + rotl32(v3, 12) + rotl32(v4, 18);
            }else{
                h32  = seed + PRIME32_5;
            }

            h32 += (u32)length;

            while((ptr+4)<=end){
                h32 += read32LEImpl<T>(ptr) * PRIME32_3;
                h32  = rotl32(h32, 17) * PRIME32_4;
                ptr+=4;
            }

            while(ptr<end){
                h32 += (*ptr) * PRIME32_5;
                h32 = rotl32(h32, 11) * PRIME32_1;
                ++ptr;
            }

            h32 ^= h32 >> 15;
            h32 *= PRIME32_2;
            h32 ^= h32 >> 13;
            h32 *= PRIME32_3;
            h32 ^= h32 >> 16;

            return h32;
        }

        template<s32 T>
        inline u64 xxHash64Impl(const u8* data, s32 length, u64 seed)
        {
            const u8* ptr = data;
            const u8* end = ptr + length;
            u64 h64;

            if(length>=32){
                const u8* const limit = end - 32;
                u64 v1 = seed + PRIME64_1 + PRIME64_2;
                u64 v2 = seed + PRIME64_2;
                u64 v3 = seed + 0;
                u64 v4 = seed - PRIME64_1;

                do{
                    v1 = round64(v1, read64LEImpl<T>(ptr)); ptr+=8;
                    v2 = round64(v2, read64LEImpl<T>(ptr)); ptr+=8;
                    v3 = round64(v3, read64LEImpl<T>(ptr)); ptr+=8;
                    v4 = round64(v4, read64LEImpl<T>(ptr)); ptr+=8;
                }while(ptr<=limit);

                h64 = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
                h64 = mergeRound64(h64, v1);
                h64 = mergeRound64(h64, v2);
                h64 = mergeRound64(h64, v3);
                h64 = mergeRound64(h64, v4);

            }else{
                h64  = seed + PRIME64_5;
            }

            h64 += (u64)length;

            while((ptr+8)<=end){
                u64 const k1 = round64(0, read64LEImpl<T>(ptr));
                h64 ^= k1;
                h64  = rotl64(h64,27) * PRIME64_1 + PRIME64_4;
                ptr+=8;
            }

            if((ptr+4)<=end){
                h64 ^= (u64)(read32LEImpl<T>(ptr)) * PRIME64_1;
                h64 = rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
                ptr+=4;
            }

            while(ptr<end){
                h64 ^= (*ptr) * PRIME64_5;
                h64 = rotl64(h64, 11) * PRIME64_1;
                ++ptr;
            }

            h64 ^= h64 >> 33;
            h64 *= PRIME64_2;
            h64 ^= h64 >> 29;
            h64 *= PRIME64_3;
            h64 ^= h64 >> 32;

            return h64;
        }
    }

    //----------------------------------------------------
    //---
    //--- xxHash32
    //---
    //----------------------------------------------------
    u32 xxHash32(const u8* data, s32 length, u32 seed)
    {
        HASSERT(NULL != data);
        HASSERT(0<=length);
        return xxHash32Impl<Aligned>(data, length, seed);
    }


    u32 xxHash32_4(const u8* data, u32 seed)
    {
        static const u32 length = 4;
        const u8* ptr = data;
        u32 h32 = seed + PRIME32_5 + length;

        {
            h32 += read32LEImpl<Aligned>(ptr) * PRIME32_3;
            h32  = rotl32(h32, 17) * PRIME32_4;
        }

        h32 ^= h32 >> 15;
        h32 *= PRIME32_2;
        h32 ^= h32 >> 13;
        h32 *= PRIME32_3;
        h32 ^= h32 >> 16;

        return h32;
    }

    u32 xxHash32_8(const u8* data, u32 seed)
    {
        static const u32 length = 8;
        const u8* ptr = data;
        u32 h32 = seed + PRIME32_5 + length;

        {
            h32 += read32LEImpl<Aligned>(ptr) * PRIME32_3;
            h32  = rotl32(h32, 17) * PRIME32_4;
            h32 += read32LEImpl<Aligned>(ptr+4) * PRIME32_3;
            h32  = rotl32(h32, 17) * PRIME32_4;
        }

        h32 ^= h32 >> 15;
        h32 *= PRIME32_2;
        h32 ^= h32 >> 13;
        h32 *= PRIME32_3;
        h32 ^= h32 >> 16;

        return h32;
    }

    //----------------------------------------------------
    //---
    //--- xxHash64
    //---
    //----------------------------------------------------
    u64 xxHash64(const u8* data, s32 length, u64 seed)
    {
        HASSERT(NULL != data);
        HASSERT(0<=length);
        return xxHash64Impl<Aligned>(data, length, seed);
    }

    u64 xxHash64_4(const u8* data, u64 seed)
    {
        static const u64 length = 4;
        const u8* ptr = data;
        u64 h64 = seed + PRIME64_5 + length;

        {
            h64 ^= (u64)(read32LEImpl<Aligned>(ptr)) * PRIME64_1;
            h64 = rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
        }

        h64 ^= h64 >> 33;
        h64 *= PRIME64_2;
        h64 ^= h64 >> 29;
        h64 *= PRIME64_3;
        h64 ^= h64 >> 32;

        return h64;
    }

    u64 xxHash64_8(const u8* data, u64 seed)
    {
        static const u64 length = 8;
        const u8* ptr = data;
        u64 h64 = seed + PRIME64_5 + length;

        {
            u64 const k1 = round64(0, read64LEImpl<Aligned>(ptr));
            h64 ^= k1;
            h64  = rotl64(h64,27) * PRIME64_1 + PRIME64_4;
        }

        h64 ^= h64 >> 33;
        h64 *= PRIME64_2;
        h64 ^= h64 >> 29;
        h64 *= PRIME64_3;
        h64 ^= h64 >> 32;

        return h64;
    }
}

