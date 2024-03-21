#include <leviathan/meta/template_info.hpp>
#include <string/fixed_string.hpp>

#include <span>
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

#include <assert.h>

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

enum class option_type : int
{
    leaf,
    root,
};

inline std::string_view empty_value = "";

struct parse_result
{
    // dotnet new console -n "My Application" -o path-to-output
    using iter_pair = std::pair<const char*, const char*>;

    std::span<const char*> m_names;  // store [dotnet, new, console]
    std::vector<iter_pair> m_args;   // store 

    void collect_args()
    {

    }
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

    consteval static size_t count()
    { return 1; }

    constexpr static bool is_name(std::string_view name)
    { return name == longname() || name == shortname(); }

    /**
     * @brief: Try parse option.
     * 
     * @param:
     *  - first: Current token if commands.
     *  - last: Sentinel of commands.
     * 
     * @return: Next position of current option if succeed, otherwise first.
    */
    template <typename I, typename S>
    constexpr static I parse(I first, S last) 
    {
        if (!(first != last && is_name(*first)))
        {
            throw command_error(std::format("Unknown token {}", *first));
        }

        std::cout << std::format("Current options is {} and value = {}\n", longname(), *(first + 1));
        return first + 2;
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

template <leviathan::fixed_string RootName, typename... Arguments>
class options
{
    static_assert(sizeof...(Arguments) > 0);

public:

    consteval static size_t count()
    { return (1 + ... + Arguments::count()); }

    consteval static size_t max_length()
    { return RootName.size(); }

    constexpr static bool is_name(std::string_view name)
    { return RootName.sv() == name; }

    consteval static auto rootname() 
    { return RootName.sv(); }

    consteval static auto longname() 
    { return RootName.sv(); }

    consteval static auto shortname() 
    { return RootName.sv(); }

    consteval static auto help() 
    { return RootName.sv(); }

    template <typename I, typename S>
    constexpr I static parse_current_leaf(I first, S last) 
    {
        while (1)
        {
            I cur = parse_current_leaf_impl<0>(first, last);
            if (cur == first)
            {
                throw command_error("Unknown Option.");
            }
            if (cur == last)
            {
                return cur;
            }
            first = cur;
        }
    }

    template <size_t Idx, typename I, typename S>
    constexpr I static parse_current_leaf_impl(I first, S last)
    {
        constexpr auto N = sizeof...(Arguments);

        if constexpr (Idx == N)
        {
            return first;
        }
        else
        {
            if (first == last)
            {
                return first;
            }

            static_assert(Idx < N);

            using pack = std::tuple<Arguments...>;
            using current_type = std::tuple_element_t<Idx, pack>;

            if constexpr (current_type::count() != 1)
            {
                // Current argument is options
                return parse_current_leaf_impl<Idx + 1>(first, last);
            }
            else
            {
                // Current argument is option
                return current_type::is_name(*first)
                    ? current_type::parse(first, last)
                    : parse_current_leaf_impl<Idx + 1>(first, last);
            }
        }
    }

    template <size_t Idx, typename I, typename S>
    constexpr I static parse_suboptions_impl(I first, S last)
    {
        constexpr auto N = sizeof...(Arguments);

        if constexpr (Idx == N)
        {
            return first;
        }
        else
        {
            if (first == last)
            {
                return first;
            }

            static_assert(Idx < N);

            using pack = std::tuple<Arguments...>;
            using current_type = std::tuple_element_t<Idx, pack>;

            if constexpr (current_type::count() == 1)
            {
                // Current argument is option
                return parse_suboptions_impl<Idx + 1>(first, last);
            }
            else
            {
                // Current argument is options
                return current_type::is_name(*first)
                    ? current_type::parse(first + 1, last)
                    : parse_suboptions_impl<Idx + 1>(first, last);
            }
        }
    }

    template <typename I, typename S>
    constexpr I static parse_suboptions(I first, S last)
    {
        while (1)
        {
            I cur = parse_suboptions_impl<0>(first, last);
            if (cur == first)
            {
                throw command_error("Unknown Options.");
            }
            if (cur == last)
            {
                return cur;
            }
            first = cur;
        }
    }

    template <typename I, typename S>
    constexpr I static parse(I first, S last)
    {
        if (first == last)
        {
            return first;
        }

        std::string_view name = *first;

        // 1. Parse current options
        //  - Current name is equal to RootName.
        //  - Current name is starts with '-'.
        //  - Current option has no suboptions.
        //
        // 2. Parse suboptions
        //  Otherwise.

        if constexpr (count() == sizeof...(Arguments) - 1)
        {
            // No options
            return is_name(name) 
                ? parse_current_leaf(first + 1, last)
                : first;
        }
        else
        {
            // dotnet -v
            return name.starts_with('-')
                ? parse_current_leaf(first, last)
                : parse_suboptions(first, last);
        }
    }

private:

    static size_t calculate_length()
    { return std::ranges::max({ Arguments::max_length()... }); }

    

};

template <typename T>
struct parser
{
    struct result
    {
        size_t m_index; // Subvoption
    };

    constexpr static auto parse(int argc, const char* argv[])
    {
        return T::parse(argv + 1, argv + argc);
    }
};

int main(/* int argc, const char* argv[] */)
{
    using DotnetNewConsole = options<
        "console",
        option<std::string, "-o", "--output", "Location of output file">,
        option<std::string, "-n", "--name", "SDK instructions">
    >;

    using DotnetNew = options<
        "new",
        DotnetNewConsole
    >;

    using DotnetBuild = options<
        "build",
        option<std::string, "-o", "--output", "Location of output file">,
        option<std::string, "-v", "--verbosity", "Location of output file">
    >;
        
    using Dotnet = options<
        "dotnet",
        DotnetNew,
        DotnetBuild,
        option<int, "-v", "--version", "Version of SDK">,
        option<int, "-i", "--info", "Show the infomation of .Net">
    >;


    // int argc = 3;
    // const char* argv[] = {
    //     "dotnet", "-v", "15"
    // };

    std::vector<const char*> _1 = {
        "dotnet",
        "-v",
        "1.0",
    }; 

    std::vector<const char*> _2 = {
        "dotnet",
        "new",
        "console",
        "-n",
        "MyApplication",
    }; 

    auto& _ = _2;

    // Dotnet::parse(_.begin() + 1, _.end());
    parser<Dotnet>::parse(_.size(), _.data());

    return 0;
}
