#ifndef __TUPLE_HASH_HPP__
#define __TUPLE_HASH_HPP__

// extend std::hash
// if you have overloaded already, please ignore it
namespace std
{
    template <typename... Ts>
    struct hash<std::tuple<Ts...>>
    {
        constexpr auto operator()(const std::tuple<Ts...>& t) const noexcept // assume hash<Ts...> is will not throw any exception
        {
            return this->hash_impl(t, std::make_index_sequence<sizeof...(Ts)>());
        }

    private:
        template <size_t... Idx>
        constexpr auto hash_impl(const std::tuple<Ts...>& t, std::index_sequence<Idx...>) const noexcept
        {
            return (((std::hash<Ts>()(std::get<Idx>(t))) << 17) ^ ...);
        }
    };

}  // namespace std

#endif