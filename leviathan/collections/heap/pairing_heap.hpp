#pragma once

#include <leviathan/collections/hook/pairing_hook.hpp>

namespace cpp::collections
{

template <std::meta::info MemberHook, typename KeyOfValue, typename Compare = std::less<>>
class pairing_heap
{

public:

    using hook_type = typename [:type_of(MemberHook):];
    using value_type = typename [:parent_of(MemberHook):];
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr pairing_heap() = default;

private:

    MemberHook* m_root;
    Compare m_comp;

};



}  // namespace cpp::collections