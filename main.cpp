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
            return sph::sphash32(static_cast<uint32_t>(x.size()), reinterpret_cast<const void*>(x.c_str()));
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
        capacity_ = static_cast<size_t>(capacity_*x);
        insert_ *= x;
        insertCount_ = static_cast<size_t>(insertCount_*x);
        find0_ *= x;
        find0Count_ = static_cast<size_t>(find0Count_*x);
        erase_ *= x;
        eraseCount_ = static_cast<size_t>(eraseCount_*x);
        find1_ *= x;
        find1Count_ = static_cast<size_t>(find1Count_*x);
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
typedef hashmap::SwissTable<std::string, std::string> SwissTable;
typedef std::unordered_map<std::string, std::string> UnorderedMap;
#ifdef USE_DENSE_HASHMAP
typedef google::dense_hash_map<std::string, std::string> DenseHashMap;
#endif

template<class T>
inline void initialize(T&)
{
}

template<class T>
inline bool insert(T& t, const std::string& key, const std::string& value)
{
    return t.insert(key, value);
}

inline bool insert(UnorderedMap& t, const std::string& key, const std::string& value)
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
static const int MinKeyLength = 4;
static const int MaxKeyLength = 16;
static const int MaxValueLength = 64;
template<class T>
Result measure(size_t numSamples, const std::string* keys, const std::string* values)
{
    static constexpr double inv = 1.0/1000000000.0;
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
    result.insert_ = inv * std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    result.capacity_ = capacity(hashmap);

    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<numSamples; ++i){
        typename HashMapType::iterator pos = hashmap.find(keys[i]);
        if(hashmap.end() != pos){
            ++result.find0Count_;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    result.find0_ = inv * std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    HASSERT(numSamples == result.find0Count_);

    size_t halfSamples = numSamples>>1;
    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<halfSamples; ++i){
        HASSERT(MinKeyLength<=keys[i].length() && keys[i].length()<=MaxKeyLength);
        hashmap.erase(keys[i]);
        ++result.eraseCount_;
        HASSERT(MinKeyLength<=keys[i].length() && keys[i].length()<=MaxKeyLength);
    }
    end = std::chrono::high_resolution_clock::now();
    result.erase_ = inv * std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

    start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<numSamples; ++i){
        HASSERT(MinKeyLength<=keys[i].length() && keys[i].length()<=MaxKeyLength);
        typename HashMapType::iterator pos = hashmap.find(keys[i]);
        if(hashmap.end() != pos){
            ++result.find1Count_;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    result.find1_ = inv * std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
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
    size_t numSamples = 100000;//10000000;//1000;
    int count = 10;
    if(3<=argc){
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
    Result totalSwissTable;
    Result totalUnordered;
    totalHashMap.clear();
    totalHopscotch.clear();
    totalRobinHood.clear();
    totalSwissTable.clear();
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
            std::uniform_int_distribution<> distChars(0, (int32_t)(strlen(ASCII)-1));
            for(size_t i=0; i<numSamples; ++i){
                size_t keyLength = distKeyLength(random);
                HASSERT(MinKeyLength<=keyLength && keyLength<=MaxKeyLength);
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
        result = measure<SwissTable>(numSamples, keys, values);
        totalSwissTable += result;
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
    totalSwissTable *= inv;
    totalUnordered *= inv;

    print(totalHashMap, "HashMap");
    print(totalHopscotch, "Hopscotch");
    print(totalRobinHood, "RobinHood");
    print(totalSwissTable, "SwissTable");
    print(totalUnordered, "std::unordered_map");

#ifdef USE_DENSE_HASHMAP
    totalDenseHashMap *= inv;
    print(totalDenseHashMap, "dense_hash_map");
#endif
    return 0;
}
