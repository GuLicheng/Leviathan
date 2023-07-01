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
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "T should not be reference.");

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
     * @brief A simple helper for adjust type for some scripts value.
     * 
     * E.g.
     *  template <typename T> struct use_pointer : std::bool_constant<(sizeof(T) > 16)> { };
     *  template <typename T> struct to_raw_pointer : store_ptr<T*, use_pointer> { };
     * 
     *  using binder = config::bind<std::variant>::with<small_object, large_object>; => 
     *  using binder2 = typename binder::transform<to_raw_pointer>::type;
     *  using storage = typename binder2::type;
     * 
     * The small object will store as a value and large object will 
     * store as a pointer.(std::variant<small_object, large_object*>) 
    */
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

            // Used as constraint in construction.
            template <typename T>
            constexpr static bool contains = [](){
                auto sum = (false || ... || std::is_same_v<T, Ts>);
                return sum;
            }();
        };
    };

    template <template <typename...> typename Variant, typename Fn, typename... Ts>
    class value_base
    {
    protected:
    
        template <typename T>
        struct mapped
        {
            using type = std::invoke_result_t<Fn, T>;

            static_assert(std::is_same_v<type, std::remove_cvref_t<type>>);
        };

        template <typename T>
        struct declaration
        {
            constexpr static bool value = (false || ... || std::is_same_v<T, Ts>);
        };

        template <typename T>
        struct storage
        {
            constexpr static bool value = (false || ... || std::is_same_v<T, typename mapped<Ts>::type>);
        };

        using value_type = Variant<typename mapped<Ts>::type...>;

        value_type m_data;

    public:

        template <typename Arg>
            requires (declaration<Arg>::value)
        constexpr value_base(Arg arg) : m_data(Fn()(std::move(arg))) { }
    };

}


