#include <lv_cpp/meta/type_list.hpp>
#include <iostream>
#include <concepts>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <string_view>


//////////////////////////////////////////////////////
// Some Helper
//////////////////////////////////////////////////////
template <typename Target>
struct argument_cast_t;  // declration

template <typename CharT, typename Traits, typename Alloc>
struct argument_cast_t<std::basic_string<CharT, Traits, Alloc>>
{
    template <typename I>
    std::basic_string<CharT, Traits, Alloc> operator()(I first, I last) const
    {
        return std::basic_string<CharT, Traits, Alloc>{first, last};
    }

    template <typename R>
    std::basic_string<CharT, Traits, Alloc> operator()(R&& rg) const
    {
        return (*this)(std::begin(rg), std::end(rg));
    }
};

template <std::signed_integral SignedInteger>
struct argument_cast_t<SignedInteger>
{
    template <typename R>
    SignedInteger operator()(R&& rg) const
    {
        return static_cast<SignedInteger>(std::stoll(rg));
    }
};

template <std::unsigned_integral UnsignedInteger>
struct argument_cast_t<UnsignedInteger>
{
    template <typename R>
    UnsignedInteger operator()(R&& rg) const
    {
        return static_cast<UnsignedInteger>(std::stoull(rg));
    }
};

template <std::floating_point FloatingPoint>
struct argument_cast_t<FloatingPoint>
{
    template <typename R>
    FloatingPoint operator()(R&& rg) const
    {
        // return static_cast<FloatingPoint>(std::stold(rg)); long double ?
        return static_cast<FloatingPoint>(std::stod(rg));
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

struct longname : parameter<std::string> { };  // --version 
struct shortname : parameter<std::string> { };  // -v
struct default_value : parameter<std::string> { };  // ...
struct help : parameter<std::string> { };  // -v : version of...
struct argc : parameter<int> { };  // -l pthread libstdc++ ...
struct is_const : parameter<bool> { };  // for -v, it's unchangeable and must have default value
struct required : parameter<bool> { };   // optional params
struct choices : parameter<std::vector<std::string_view>> 
{
    using base = parameter<std::vector<std::string_view>>;
    choices(std::vector<std::string_view> c) : base{ std::move(c) } { }
};

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
        AssignArgToInfo(choices)

#undef AssignArgToInfo

        // check
        if (i.m_is_const && i.m_default_value.empty())
            throw std::invalid_argument("const attribute must have default value");

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

    void display() const 
    {
        for (auto& i : m_args)
            std::cout << i << '\n';
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
    parser.add_argument(longname("--epoch"), default_value("15"), choices({"15", "30", "45"}));
    parser.add_argument(longname("--lr"), default_value("1e-5"));
    parser.parse_args(argc, argv);
    std::cout << parser.get<int>("--epoch") << '\n';
    std::cout << parser.get<float>("--lr") << '\n';
    std::cout << parser.get<std::string>("-v") << '\n';
    parser.display();
    std::cout << "OK\n";
    return 0;
}


