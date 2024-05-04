#pragma once

#include <stdexcept>
#include <format>
#include <compare>

namespace leviathan
{

struct controllable_value_exception 
{ };

/*
Value  =>
    0  : No exception will be thrown
    >0 : Throw exception when the constructor count is greater than
    <0 : Disable constructor
*/
template <typename T, 
    int CopyCtorCount = 0,
    int MoveCtorCount = 0,
    int DefaultCtorCount = 0,
    int ValueCtorCount = 0>
struct controllable_value
{
    static constexpr bool copyable = CopyCtorCount >= 0;
    static constexpr bool moveable = MoveCtorCount >= 0;
    static constexpr bool defaultable = DefaultCtorCount >= 0;
    static constexpr bool valueable = ValueCtorCount >= 0;

    inline static int copy_ctor = 0;
    inline static int move_ctor = 0;
    inline static int value_ctor = 0;
    inline static int default_ctor = 0;

    inline static int copy_assn = 0;
    inline static int move_assn = 0;

    inline static int dtor = 0;

    template <int Count>
    static void throw_exception(int& num)
    {
        if (Count > 0)
        {
            if (num > Count)
            {
                throw controllable_value_exception();
            }
        }
        ++num;
    }

    static int total_construct()
    {
        return default_ctor + copy_ctor + move_ctor + value_ctor;
    }

    static int total_destruct()
    {
        return dtor;
    }

    explicit controllable_value(T x) : m_value(std::move(x)) 
    {
        ++value_ctor;
    }

    controllable_value() 
    requires (defaultable)
    {
        throw_exception<DefaultCtorCount>(default_ctor);
    }

    controllable_value(const controllable_value& rhs) 
    requires (copyable) : m_value(rhs.m_value)
    {
        throw_exception<CopyCtorCount>(copy_ctor);
    }

    controllable_value(controllable_value&& rhs) 
    requires (moveable) : m_value(std::move(rhs.m_value))
    {
        throw_exception<CopyCtorCount>(move_ctor);
    }

    controllable_value& operator=(const controllable_value& rhs)
    {
        m_value = rhs.m_value;
        ++copy_assn;
        return *this;
    }

    controllable_value& operator=(controllable_value&& rhs) 
    {
        m_value = std::move(rhs.m_value);
        ++move_assn;
        return *this;
    }

    ~controllable_value()
    {
        ++dtor;
    }

    bool operator==(const controllable_value&) const = default;
    
    auto operator<=>(const controllable_value&) const = default;

    T m_value;
};

} // namespace leviathan

template <typename T>
struct std::hash<leviathan::controllable_value<T>>
        : std::hash<T> { };











        