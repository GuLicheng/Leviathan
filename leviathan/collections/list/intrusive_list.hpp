#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct list_member_hook : public list_node_interface
{
    std::array<list_node_interface*, 2> links;
};

// https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1intrusive_1_1list.html
template <std::meta::info MemberHook>
class list
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

    constexpr list() 
    {
        m_header.reset();
        m_size = 0;
    }

    void clear();

    void push_back(reference value)
    {
        m_header.insert_after(value.[:Hook:]);
    }


    void push_front(reference value);
    void pop_back();
    void pop_front();
    template<typename Disposer> void pop_back_and_dispose(Disposer);
    template<typename Disposer> void pop_front_and_dispose(Disposer);

private:

    list_member_hook m_header;
    size_type m_size;

};

}  // namespace cpp::collections::intrusive
