#include <string>
#include "HashMap.h"

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

int main(void)
{
	{
		hashmap::SwissTable<std::string, std::string> swisstable;
		std::string key = "test";
		std::string value = "test";
		bool result = swisstable.insert(key, value);
		assert(result);
		hashmap::u32 pos = swisstable.find(key);
		assert(pos != swisstable.end());
		swisstable.erase(key);
		pos = swisstable.find(key);
		assert(pos == swisstable.end());
	}
	return 0;
}
