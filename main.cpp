#include <string>
#include <iostream>
#include <random>
#include <chrono>
#include <unordered_map>

//#define USE_DENSE_HASHMAP

#ifdef USE_DENSE_HASHMAP
#include <sparsehash/dense_hash_map>
#endif

#include "HashMap.h"
#include "xxHash.h"

static const char* ASCII = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

template<class T, class U>
void createRandomString(std::string& str, size_t length, T& random, U& distribution)
{
    HASSERT(0<=length);
    str.reserve(length+1);
    for(size_t i=0; i<length; ++i){
        int c = distribution(random);
        str.push_back(ASCII[c]);
    }
}

namespace hashmap
{
    namespace hash_detail
    {
        template<>
        inline u32 calcHash<std::string>(const std::string& x)
        {
            return hashmap::xxHash32(reinterpret_cast<const hashmap::u8*>(x.c_str()), static_cast<hashmap::s32>(x.size()));
        }

    }
}

struct Result
{
    void clear()
    {
        capacity_ = 0;
        insert_ = 0.0f;
        insertCount_ = 0;
        find0_ = 0.0f;
        find0Count_ = 0;
        erase_ = 0.0f;
        eraseCount_ = 0;
        find1_ = 0.0f;
        find1Count_ = 0;
    }

    Result& operator+=(const Result& x)
    {
        capacity_ += x.capacity_;
        insert_ += x.insert_;
        insertCount_ += x.insertCount_;
        find0_ += x.find0_;
        find0Count_ += x.find0Count_;
        erase_ += x.erase_;
        eraseCount_ += x.eraseCount_;
        find1_ += x.find1_;
        find1Count_ += x.find1Count_;
        return *this;
    }


    Result& operator*=(double x)
    {
        capacity_ *= x;
        insert_ *= x;
        insertCount_ *= x;
        find0_ *= x;
        find0Count_ *= x;
        erase_ *= x;
        eraseCount_ *= x;
        find1_ *= x;
        find1Count_ *= x;
        return *this;
    }
    size_t capacity_;

    double insert_;
    size_t insertCount_;
    double find0_;
    size_t find0Count_;
    double erase_;
    size_t eraseCount_;
    double find1_;
    size_t find1Count_;
};

typedef hashmap::HashMap<std::string, std::string> HashMap;
typedef hashmap::HopscotchHashMap<std::string, std::string> HopscotchHashMap;
typedef hashmap::RHHashMap<std::string, std::string> RHHashMap;
typedef std::unordered_map<std::string, std::string> UnorderedMap;
#ifdef USE_DENSE_HASHMAP
typedef google::dense_hash_map<std::string, std::string> DenseHashMap;
#endif

template<class T>
inline void initialize(T&)
{
}

template<class T>
inline bool insert(T& t, std::string& key, std::string& value)
{
    return t.insert(key, value);
}

inline bool insert(UnorderedMap& t, std::string& key, std::string& value)
{
    t[key] = value;
    return t.end() != t.find(key);
}

template<class T>
inline size_t capacity(T& t)
{
    return t.capacity();
}

inline size_t capacity(UnorderedMap& t)
{
    return t.size();
}

#ifdef USE_DENSE_HASHMAP
inline void initialize(DenseHashMap& t)
{
    t.set_empty_key(std::string());
    t.set_deleted_key(std::string(" "));
}

inline bool insert(DenseHashMap& t, std::string& key, std::string& value)
{
    t[key] = value;
    return t.end() != t.find(key);
}

inline size_t capacity(DenseHashMap& t)
{
    return t.size();
}
#endif

template<class T>
Result measure(size_t numSamples, std::string* keys, std::string* values)
{
    typedef T HashMapType;
    HashMapType hashmap;
    initialize(hashmap);

    Result result;
    result.clear();
    std::chrono::high_resolution_clock::time_point start, end;
    
    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<numSamples; ++i){
        if(insert(hashmap, keys[i], values[i])){
            ++result.insertCount_;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    result.insert_ = std::chrono::duration<double>(end-start).count();
    result.capacity_ = capacity(hashmap);

    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<numSamples; ++i){
        typename HashMapType::iterator pos = hashmap.find(keys[i]);
        if(hashmap.end() != pos){
            ++result.find0Count_;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    result.find0_ = std::chrono::duration<double>(end-start).count();

    size_t halfSamples = numSamples>>1;
    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<halfSamples; ++i){
        hashmap.erase(keys[i]);
        ++result.eraseCount_;
    }
    end = std::chrono::high_resolution_clock::now();
    result.erase_ = std::chrono::duration<double>(end-start).count();

    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<numSamples; ++i){
        typename HashMapType::iterator pos = hashmap.find(keys[i]);
        if(hashmap.end() == pos){
            ++result.find1Count_;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    result.find1_ = std::chrono::duration<double>(end-start).count();

    return result;
}

void print(const Result& result, const char* name)
{
    std::cout << name << std::endl;
    std::cout << " capacity: " << result.capacity_ << std::endl;
    std::cout << " insert: " << result.insert_ << " (" << result.insertCount_ << ")" << std::endl;
    std::cout << " find0 : " << result.find0_ << " (" << result.find0Count_ << ")" << std::endl;
    std::cout << " erase : " << result.erase_ << " (" << result.eraseCount_ << ")" << std::endl;
    std::cout << " find1 : " << result.find1_ << " (" << result.find1Count_ << ")" << std::endl;
}

int main(int argc, char** argv)
{
    static const int MinKeyLength = 4;
    static const int MaxKeyLength = 16;
    static const int MaxValueLength = 64;

    size_t numSamples = 1000;
    int count = 10;
    std::cout << argv[0] << std::endl;
    if(3<=argc){
        std::cout << argv[0] << std::endl;
        numSamples = atoll(argv[1]);
        count = atoi(argv[2]);
        if(count<=0){
            count = 1;
        }
    }
    std::cout << "num samples: " << numSamples << ", count: " << count << std::endl;

    Result totalHashMap;
    Result totalHopscotch;
    Result totalRobinHood;
    Result totalUnordered;
    totalHashMap.clear();
    totalHopscotch.clear();
    totalRobinHood.clear();
    totalUnordered.clear();

#ifdef USE_DENSE_HASHMAP
    Result totalDenseHashMap;
    totalDenseHashMap.clear();
#endif

    for(size_t n=0; n<count; ++n){
        std::string* keys = HNEW std::string[numSamples];
        std::string* values = HNEW std::string[numSamples];
        {
            std::random_device device;
            std::mt19937 random(device());
            std::uniform_int_distribution<> distKeyLength(MinKeyLength, MaxKeyLength);
            std::uniform_int_distribution<> distValueLength(0, MaxValueLength);
            std::uniform_int_distribution<> distChars(0, strlen(ASCII)-1);
            for(size_t i=0; i<numSamples; ++i){
                size_t keyLength = distKeyLength(random);
                createRandomString(keys[i], keyLength, random, distChars);

                size_t valueLength = distValueLength(random);
                createRandomString(values[i], valueLength, random, distChars);
            }
        }

        

        Result result;
        result = measure<HashMap>(numSamples, keys, values);
        totalHashMap += result;
        result = measure<HopscotchHashMap>(numSamples, keys, values);
        totalHopscotch += result;
        result = measure<RHHashMap>(numSamples, keys, values);
        totalRobinHood += result;
        result = measure<UnorderedMap>(numSamples, keys, values);
        totalUnordered += result;
#ifdef USE_DENSE_HASHMAP
        result = measure<DenseHashMap>(numSamples, keys, values);
        totalDenseHashMap += result;
#endif

        HDELETE_ARRAY(values);
        HDELETE_ARRAY(keys);
    }
    double inv = 1.0/count;
    totalHashMap *= inv;
    totalHopscotch *= inv;
    totalRobinHood *= inv;
    totalUnordered *= inv;

    print(totalHashMap, "HashMap");
    print(totalHopscotch, "Hopscotch");
    print(totalRobinHood, "RobinHood");
    print(totalUnordered, "std::unordered_map");

#ifdef USE_DENSE_HASHMAP
    totalDenseHashMap *= inv;
    print(totalDenseHashMap, "dense_hash_map");
#endif
    return 0;
}
