#include <leviathan/string/string_extend.hpp>
#include <leviathan/config_parser/optional.hpp>

#include <string_view>

namespace leviathan::config
{
    /**
     * @brief This value is used for storing the string of parser.
     * 
     * @param T The storage type of parser, maybe string or string_view.
    */
    template <typename T>
    struct basic_item : optional<T>
    {
        using optional<T>::optional;

        template <typename U>
        constexpr optional<U> cast() const
        {
            if (!*this)
                return nullopt;
            return leviathan::string::lexical_cast<U>(**this);
        }
    };

    using item = basic_item<std::string_view>;

    /**
     * @brief A helper meta for checking whether use pointer.
     * 
     * We often use union to store variables, but some variables may occupy much
     * bytes(std::string/std::map). For these types we can store pointer.
     * 
     * @param Pointer Pointer type.
     * @param Fn Used for judging.
    */
    template <typename Pointer, template <typename> typename Fn>
    struct store_ptr 
    {
        using element_type = typename std::pointer_traits<Pointer>::element_type;
        
        constexpr static bool value = Fn<element_type>::value;

        using type = std::conditional_t<value, Pointer, element_type>;
    };

    template <typename T, template <typename> typename Fn>
    struct store_ptr<T*, Fn>
    {
        using element_type = T;
        
        constexpr static bool value = Fn<element_type>::value;

        using type = std::conditional_t<value, element_type*, element_type>;
    };

    template <typename T>
    struct is_large_than_raw_pointer : std::bool_constant<(sizeof(T) > 8)> { };

    template <typename T>
    struct to_raw_pointer : store_ptr<T*, is_large_than_raw_pointer> { };

    // template <typename T>
    // struct to_unique_ptr : config::store_ptr<std::unique_ptr<T>, config::is_large_than_raw_pointer> { };

    template <template <typename...> typename Container>
    struct bind
    {
        template <typename... Ts>
        struct with
        {
            using type = Container<Ts...>;

            template <template <typename> typename Fn>
            struct transform
            {
                using type = bind<Container>::with<typename Fn<Ts>::type...>;
            };

            // Used as constraint in construction
            template <typename T>
            constexpr static bool contains = [](){
                auto sum = (false || ... || std::is_same_v<T, Ts>);
                return sum;
            }();
        };
    };


}


