#pragma once

namespace leviathan
{

// Generate i++ by ++i.
struct postfix_increment_operation
{
    template <typename I>
    I operator++(this I& /* lvalue */ it, int)
    {
        auto old = it;
        ++it;
        return old;
    }
};

// Generate i-- by --i.
struct postfix_decrement_operation
{
    template <typename I>
    I operator--(this I& /* lvalue */ it, int)
    {
        auto old = it;
        --it;
        return old;
    }
};

// Generate i-> by *i.
struct arrow_operation
{
    template <typename I>
    auto operator->(this I&& it)
    {
        return std::addressof(*it);
    }
};

struct posfix_and_arrow 
    : postfix_increment_operation, postfix_decrement_operation, arrow_operation { };

} // namespace leviathan

