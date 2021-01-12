
#ifndef __SINGLE_HPP__
#define __SINGLE_HPP__


namespace leviathan 
{

namespace mode
{
    struct lazy { };
    struct eager { };
} // namespace mode

// thread safe after c++11, so you should keep your standard at least -std=c++11
template <typename _Derived_Type, typename _Mode = mode::lazy>
class singleton
{
public:
    using derived = _Derived_Type;

    singleton(const singleton&) = delete;
    singleton& operator=(const singleton&) = delete;
    
    static derived* get_instance() noexcept
    {
        static derived* instance = new derived();
        return instance;
    }

protected:

    constexpr singleton() noexcept = default;

};

template <typename _Derived_Type>
class singleton<_Derived_Type, mode::eager>
{

public:
    using derived = _Derived_Type;

    singleton(const singleton&) = delete;
    singleton& operator=(const singleton&) = delete;
    
    static derived* get_instance() noexcept
    {
        return instance;
    }

protected:
    singleton() noexcept = default;

    inline static derived* instance = new derived();

};


} // namespace leviathan

#endif