#ifndef __INI_HPP__
#define __INI_HPP__

#include <list>

namespace leviathan::ini
{
    template <typename Key, typename Value>
    struct section_node
    {
        section_node() = default;
        ~section_node() = default;
        using entry = std::pair<Key, Value>;

        std::list<entry> entries_;

        Key& operator[](const Key& section_name)
        {
            auto iter = ::std::find_if(entries_.begin(), entries_.end(), [&](auto&& e)
            {
                return e.first == section_name;
            });

            if (iter == entries_.end())
                return entries_.emplace_back(section_name, Value{ }).second;
            return iter->second;
        }

        Key& operator[](Key&& section_name)
        {
            auto iter = ::std::find_if(entries_.begin(), entries_.end(), [&](auto&& e)
            {
                return e.first == section_name;
            });

            if (iter == entries_.end())
                return entries_.emplace_back(section_name, Value{ }).second;
            return iter->second;
        }

    };
}





#endif