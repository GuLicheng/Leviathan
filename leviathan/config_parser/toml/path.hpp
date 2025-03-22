#pragma once

#include <vector>

namespace leviathan::config
{

template <typename Value, typename List, typename Dict>
class path
{
    struct result
    {
        const Value* m_value;
        std::vector<const Value*> m_keys;
    };
};

} // namespace leviathan::config


