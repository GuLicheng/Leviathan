#include <leviathan/meta/template_info.hpp>

#include <format>
#include <string>
#include <ranges>
#include <array>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <string>
#include <iostream>
#include <unordered_map>
#include <optional>

#include <string/fixed_string.hpp>

struct command_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

enum class parse_mode 
{
    optional,
    gather,
    just_one,
}; 

template <typename T, 
        leviathan::basic_fixed_string Shortname, 
        leviathan::basic_fixed_string Longname, 
        leviathan::basic_fixed_string Help = "",
        bool Required = false>
class option
{
    template <auto S1, auto... Ss>
    struct char_checker
    {
        constexpr static bool value = 
            (std::is_same_v<
            typename decltype(S1)::string_view_type, 
            typename decltype(Ss)::string_view_type> 
                && ... && true);
    };


    static_assert(char_checker<Longname, Shortname, Help>::value);
    using string_view_type = typename decltype(Longname)::string_view_type;

public:

    using value_type = T;

    consteval option() = default;

    consteval static string_view_type longname() 
    { return Longname.sv(); }

    consteval static string_view_type shortname() 
    { return Shortname.sv(); }

    consteval static string_view_type help()
    { return Help.sv(); }

    consteval static size_t max_length()
    { return std::max(Longname.size(), Shortname.size()); }

    consteval static bool required() 
    { return Required; }

    /**
     * @brief:
     * @param:
     *  idx: Current index of option values.
     *  argc: Count of arguments.
     *  argv: Arguments values.
    */
    template <typename Fn>
    constexpr static std::optional<T> parse(int argc, const char* argv[], Fn fn) 
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string_view name = argv[i];
            
            if (name == longname() || name == shortname())
            {
                if (i + 1 == argc)
                {
                    throw command_error("Argument should not in last.");
                }
                std::string_view value = argv[i + 1];
                return std::make_optional(fn(value));
            }
        }
        return std::nullopt;
    }
};

template <typename T> struct default_parser;

template <> 
struct default_parser<int>
{
    constexpr static int operator()(std::string_view name)
    {
        return std::stoi(std::string(name));
    }
};

template <> 
struct default_parser<std::string>
{
    constexpr static std::string operator()(std::string_view name)
    {
        return std::string(name);
    }
};

template <typename... Arguments>
class options
{
    static_assert(sizeof...(Arguments) > 0);

    constexpr static std::array shortnames = { Arguments::shortname()... };
    constexpr static std::array longnames = { Arguments::longname()... };
    constexpr static std::array helps = { Arguments::help()... };
    
    constexpr static const char* default_prog = "";
    constexpr static const char* default_description = "";

    template <leviathan::basic_fixed_string Name>
    struct str_wrapper
    {
        constexpr static auto name() { return Name; }
    };

    template <typename StrWrapper>
    struct helper
    {
        consteval static size_t get_return_value_index()
        {
            const auto name = StrWrapper::name();
            return name.starts_with("--") 
                ? std::ranges::distance(std::ranges::find(longnames, name), longnames.end()) 
                : std::ranges::distance(std::ranges::find(shortnames, name), shortnames.end());
        }
    };

    using storage = std::tuple<std::optional<typename Arguments::value_type>...>;

    struct parse_result : storage
    {
        using storage::storage;

        template <typename StrWrapper, typename Fn = void>
        auto& get() const
        {
            constexpr auto idx = helper<StrWrapper>::get_return_value_index();
            static_assert(idx != sizeof...(Arguments), "Unknown name.");
            return std::get<idx>(*this);
        }

        void show()
        {
            [this]<size_t... Idx>(std::index_sequence<Idx...>) {
                ((std::cout << std::format("Is {} has value ? {}\n", Idx, std::get<Idx>(*this).has_value())), ...);
            }(std::make_index_sequence<sizeof...(Arguments)>());
        }
    };

public:

    options(std::string prog = default_prog, std::string description = default_description)
        : m_prog(std::move(prog)), m_description(std::move(description)), m_length(calculate_length())
    { }

    void show() const
    {
        std::string usage;
        std::format_to(std::back_inserter(usage), "Usage: [optionals]\n\n{}\n\n", m_prog);
        std::format_to(std::back_inserter(usage), "Usage: {}:\n", m_description);

        for (size_t i = 0; i < sizeof...(Arguments); ++i)
        {
            std::format_to(
                std::back_inserter(usage), 
                "\t{}, {:{}}\t\t{}\n", 
                shortnames[i], 
                longnames[i], 
                m_length, 
                helps[i]);
        }

        std::cout << usage << '\n';
    }

    parse_result parse(int argc, const char* argv[]) const
    {
        return { Arguments::parse(argc, argv, default_parser<typename Arguments::value_type>())... };
    }

private:

    static size_t calculate_length()
    { return std::ranges::max({ Arguments::max_length()... }); }

    std::string m_prog;
    std::string m_description;
    size_t m_length;
};

int main(/* int argc, const char* argv[] */)
{
    using Arg1 = option<int, "-e", "--epoch", "Epoch">;
    using Arg2 = option<std::string, "-n", "--name", "Name of prog">;

    options<Arg1, Arg2> parser;


    parser.show();

    int argc = 3;
    const char* argv[] = {
        "prop", "-e", "15"
    };

    auto result = parser.parse(argc, argv);

    result.show();


    return 0;
}
