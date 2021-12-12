
#ifndef __SINGLE_HPP__
#define __SINGLE_HPP__


namespace leviathan 
{

    // thread safe after c++11, so you should keep your standard at least -std=c++11
    // your class shoule have default constructure
    template <typename _Derived_Type>
    class singleton
    {
    public:
        using derived = _Derived_Type;

        singleton(const singleton&) = delete;
        singleton& operator=(const singleton&) = delete;
        
        static derived* get_instance() noexcept
        {
            static derived instance;
            return &instance;
        }

    protected:

        constexpr singleton() noexcept = default;

    };

} // namespace leviathan

#endif