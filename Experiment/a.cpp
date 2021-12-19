#include <iostream>
#include <concepts>
#include <type_traits>
#include <string>
#include <variant>
#include <vector>
#include <functional>
#include <unordered_set>
#include <lv_cpp/meta/type_list.hpp>
#include <string_view>

// name or flags (short or long name)
// nargs 
// const
// default
// type
// choices
// required
// help

/*

*/

//////////////////////////////////////////////////////
// Some Helper
//////////////////////////////////////////////////////
template <typename Target>
struct argument_cast_t;

template <typename T>
struct argument_choice_t
{
    argument_choice_t() = default;

    template <typename Proj, typename... Ts>
    void add_choices(Proj proj, Ts... ts)
    {
        (m_choices.emplace_back(static_cast<T>(std::invoke(proj, ts))), ...);
    }

    std::vector<T> m_choices;
};

template <>
struct argument_cast_t<std::string>
{
    template <typename I>
    std::string operator()(I first, I last) const
    {
        return std::string{first, last};
    }

    template <typename R>
    std::string operator()(R&& rg) const
    {
        return (*this)(std::begin(rg), std::end(rg));
    }
};

template <>
struct argument_cast_t<int>
{
    template <typename R>
    int operator()(R&& rg) const
    {
        return std::stoi(rg);
    }
};

template <typename T, typename U>
T argument_cast(U&& u)
{
    return argument_cast_t<T>()((U&&) u);
}

template <typename TParams>
struct parameter
{
    template <typename T>
    parameter(T&& t) : Value{(T&&)t} { }

    parameter(const parameter&) = delete;
    parameter& operator=(const parameter&) = delete;

    TParams Value;
};

struct longname : public parameter<std::string> { };
struct shortname : public parameter<std::string> { };
struct default_value : public parameter<std::string> { };
struct help : public parameter<std::string> { };
struct argc : public parameter<int> { };
struct is_const : public parameter<bool> { };
struct required : public parameter<bool> { };

class argument_parser
{
public:

    struct info
    {
        std::string m_longname;
        std::string m_shortname;
        std::string m_help;
        std::string m_default_value; // val for type_cast
        int m_argc = 1;
        bool m_is_const = false;
        bool m_required = false;
        std::vector<std::string_view> m_choices;

        friend std::ostream& operator<<(std::ostream& os, const info& i)
        {
            os << "m_longname = " << i.m_longname << " m_shortname = " << i.m_shortname 
                << " help = " << i.m_help << " nargc = " << i.m_argc << " const ?: " << i.m_is_const
                << " required ?: " << i.m_required << " value = " << i.m_default_value;
            return os;
        }
    };

    argument_parser() = default;
    argument_parser(const argument_parser&) = delete;
    argument_parser& operator=(const argument_parser&) = delete;

    template <typename... Args>
    void add_argument(Args... args)
    {
        using leviathan::meta::find_first_index_of;
        using TypeList = std::tuple<Args...>;

        constexpr auto npos = static_cast<size_t>(-1);
        constexpr auto longname_idx = find_first_index_of<TypeList, longname>::value;
        constexpr auto shortname_idx = find_first_index_of<TypeList, shortname>::value;

        static_assert(longname_idx < npos || shortname_idx < npos, "There at least one name");

        info i;
        auto params = std::forward_as_tuple(std::move(args)...);

        auto register_name = [this]<typename T>(const T& name)
        {
            if constexpr (std::is_same_v<T, longname> || std::is_same_v<T, shortname>)
            {
                if (this->m_names.count(name.Value))
                    throw std::invalid_argument("name already existed");
                this->m_names.emplace(name.Value);
            }
        };

        (register_name(args), ...);

#define AssignArgToInfo(name)                                                             \
    {                                                                                     \
        constexpr auto idx = find_first_index_of<TypeList, name>::value;                  \
        if constexpr (idx != npos)                                                        \
            i.m_##name = std::move(std::get<idx>(params).Value);                          \
    }

        AssignArgToInfo(longname)
        AssignArgToInfo(shortname)
        AssignArgToInfo(default_value)
        AssignArgToInfo(help)
        AssignArgToInfo(argc)
        AssignArgToInfo(is_const)
        AssignArgToInfo(required)

#undef AssignArgToInfo

        // check
        if (i.m_is_const && i.m_default_value.empty())
            throw std::invalid_argument("const attribute must have default value");

        std::cout << i << '\n';
        m_args.emplace_back(std::move(i));
    }

    void parse_args(int argc, char const *argv[]) 
    {
        m_prop_name = argv[0];
        for (int i = 1; i < argc; ++i)
        {

        }
    }

    template <typename T>
    T get(const std::string& name) 
    {
        auto iter = std::find_if(m_args.begin(), m_args.end(), [&](const info& i)
        {
            return i.m_longname == name || i.m_shortname == name;
        });
        if (iter == m_args.end())
        {
            std::string msg = "This is no key: ";
            msg += name;
            msg += " in Namespace";
            throw std::invalid_argument(std::move(msg));
        }
        return argument_cast<T>(iter->m_default_value);
    }

private:
    std::vector<info> m_args;
    std::string m_prop_name;
    std::unordered_set<std::string> m_names;
};

int main(int argc, char const *argv[])
{
    argument_parser parser;
    parser.add_argument(shortname("-v"), longname("--version"), is_const(true), default_value("0.0.0"));
    parser.add_argument(longname("--epoch"), default_value("15"));
    parser.add_argument(longname("--lr"), default_value("1e-5"));
    parser.parse_args(argc, argv);
    std::cout << parser.get<int>("--epoch") << '\n';
    std::cout << "OK\n";
    return 0;
}


