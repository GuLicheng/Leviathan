#include <string/fixed_string.hpp>
#include <meta/template_info.hpp>

#include <format>
// #include <span>
// #include <string>
#include <ranges>
#include <array>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <string>
#include <iostream>
#include <initializer_list>
// #include <unordered_map>

#include <assert.h>

namespace leviathan::config::cmd
{
    using leviathan::basic_fixed_string;

    struct command_error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    template <typename I>
    struct cmd_value
    {
        // dotnet new console -n CSharpApplication -o MyLearningFiles
        inline static std::vector<std::string_view> m_roots;                              // store ['dotnet', 'new', 'console]
        inline static std::unordered_map<std::string_view, std::pair<I, I>> m_values;     // store [('-n', 'CSharpApplication'), ('-o', 'MyLearningFiles')]
    
        static void add_root(std::string_view root)
        {
            m_roots.emplace_back(root);
        }

        static void add_value(std::string_view name, I first, I last)
        {
            m_values.try_emplace(name, first, last);
        }
    };

    // template <typename T>
    struct default_scanner
    {
        // using value_type = T;

        // How to store result?
        template <typename I, typename S>
        static constexpr I operator()(I first, S last)
        {
            std::cout << std::format("Current key is {} and value = {}\n", *first, *(first + 1));
            // return first + 2;
            return std::ranges::next(first, 2, last);
        }
    };

    struct none_scanner
    {
        // using value_type = void;

        template <typename I, typename S>
        static constexpr I operator()(I first, S last)
        {
            return std::ranges::next(first, 1, last);
        }
    };

    template <typename Scanner, basic_fixed_string... NameOrFlags>
    struct option
    {
        // using value_type = typename Scanner::value_type;

        static constexpr bool is_name_or_flag(std::string_view name)
        {
            auto names = { NameOrFlags.sv()... };
            return std::ranges::contains(names, name);
        }

        template <typename I, typename S>
        static constexpr I parse(I first, S last)
        {
            return Scanner()(first, last);
        }
    };

    template <typename T> 
    inline constexpr bool is_option_v = false;

    template <typename Scanner, basic_fixed_string... NameOrFlags>
    inline constexpr bool is_option_v<option<Scanner, NameOrFlags...>> = true;

    class suboptions_storage
    {
        using storage_type = std::tuple<std::string_view, const char**, const char**>;

    public:



    private:

        std::vector<storage_type> m_store;
    };

    // Options can be `class option` or `class options`
    template <fixed_string Name, typename... SubOptions>
    class options
    {
        static_assert(sizeof...(SubOptions) > 0);

        template <fixed_string S, typename... Ts>
        friend class options;
    
    public:

        template <typename I, typename S>
        static constexpr I parse(I first, S last)
        {
            if (first == last)
            {
                return first;
            }

            std::string_view name = *first;
            assert(is_name_or_flag(name));

            ++first;

            
            if constexpr (only_contains_option())
            {
                return parse_option_only(first, last);
            }
            else
            {
                return name.starts_with('-')   
                    ? parse_option_only(first, last)
                    : parse_suboptions(first, last); 
            }
        }
    
    private:

        consteval static bool only_contains_option()
        {
            return (... && is_option_v<SubOptions>);
        }

        static constexpr bool is_name_or_flag(std::string_view name)
        {
            return Name.sv() == name;
        }

        template <typename I, typename S>
        static constexpr I parse_option_only(I first, S last)
        {
            while (1)
            {
                I cur = parse_option_only_impl<0>(first, last);
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
        constexpr I static parse_option_only_impl(I first, S last)
        {
            if constexpr (Idx == sizeof...(SubOptions))
            {
                return first;
            }
            else
            {
                if (first == last)
                {
                    return first;
                }

                using current = std::tuple_element_t<Idx, std::tuple<SubOptions...>>;

                if constexpr (!is_option_v<current>)
                {
                    // Current argument is options
                    return parse_option_only_impl<Idx + 1>(first, last);
                }
                else
                {
                    // Current argument is option
                    return current::is_name_or_flag(*first)
                        ? current::parse(first, last)
                        : parse_option_only_impl<Idx + 1>(first, last);
                }
            }
        }
    
        template <size_t Idx, typename I, typename S>
        constexpr I static parse_suboptions_impl(I first, S last)
        {
            if constexpr (Idx == sizeof...(SubOptions))
            {
                return first;
            }
            else
            {
                if (first == last)
                {
                    return first;
                }

                using current = std::tuple_element_t<Idx, std::tuple<SubOptions...>>;
                // PrintTypeInfo(current);

                if constexpr (is_option_v<current>)
                {
                    // Current argument is option
                    return parse_suboptions_impl<Idx + 1>(first, last);
                }
                else
                {
                    // Current argument is options
                    auto a = current::is_name_or_flag(*first);
                    return a
                        ? current::parse(first, last)
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
                    throw command_error(std::format("Unknown Options {}.", *first));
                }
                if (cur == last)
                {
                    return cur;
                }
                first = cur;
            }
        }
    
        struct Storage
        {

        };
    };
}

namespace cmd = leviathan::config::cmd;

int main(int argc, char const *argv[])
{
    using DotnetNew = cmd::options<
        "new",
        cmd::option<cmd::default_scanner, "-o", "--output">,
        cmd::option<cmd::default_scanner, "-n", "--name">
    >;

    using Dotnet = cmd::options<
        "dotnet",
        cmd::option<cmd::default_scanner, "-h", "--help">,
        DotnetNew
    >;

    std::vector<const char*> args = {
        "dotnet",
        "new",
        "-n",
        "MyApplication",
    }; 

    Dotnet::parse(args.begin(), args.end());

    return 0;
}
