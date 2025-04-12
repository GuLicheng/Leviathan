#pragma once

#include "common.hpp"

#include <concepts>

namespace cpp::config
{
    
template <typename SequenceType>
struct sequence_assessor
{
    template <typename Value>
    static auto&& operator()(Value&& value)
    {
        assert(value.template is<SequenceType>());
        return (Value&&(value)).template as<SequenceType>();
    }
};

template <typename AssociatedType>
struct associated_assessor
{
    template <typename Value>
    static auto&& operator()(Value&& value)
    {
        assert(value.template is<AssociatedType>());
        return (Value&&(value)).template as<AssociatedType>();
    }
};

template <typename Value, typename SequenceType, typename AssociatedType>
struct accessor
{
    static_assert(std::is_same_v<typename SequenceType::value_type, Value>);
    static_assert(std::is_same_v<typename AssociatedType::mapped_type, Value>);

    Value& m_value;

    accessor(Value& value) : m_value(value) { }

    static constexpr Value* access(Value* root, std::string_view key)
    {
        if (root->template is<AssociatedType>())
        {
            auto& map = root->template as<AssociatedType>();
            auto it = map.find(key);
            return it == map.end() ? nullptr : &(it->second);
        }
        return nullptr;
    }

    template <std::integral Integer>
    static constexpr Value* access(Value* root, std::string_view key)
    {
        if (root->template is<SequenceType>())
        {
            auto& map = root->template as<SequenceType>();
            return i >= map.size() ? nullptr : &map[i]; 
        }
        return nullptr;
    }

    // template <typename T1, typename... Ts> 
    // constexpr Value* operator[](T1&& t1, Ts&&... ts)
    // {
    //     if constexpr (sizeof...(Ts) == 1)
    //     {
    //         return access((T1&&)t1);
    //     }
    //     else
    //     {
    //         auto p = access((T1&&)t1);
    //         return p ? operator[]((Ts&&)ts...) : nullptr;
    //     }
    // }
};



} // namespace cpp::config

