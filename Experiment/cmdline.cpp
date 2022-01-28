#include <lv_cpp/meta/type_list.hpp>
#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/string/opt.hpp>
#include <optional>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <string_view>
#include <regex>

#include <ctype.h>

template <typename... Ts>
void println(Ts... x)
{
    (std::cout << ... << x) << '\n';
}


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
std::optional<T> argument_cast(U&& u)
{
    try
    {
        return argument_cast_t<T>()((U&&) u);
    }
    catch(...)
    {
        return { };
    }
}

template <typename TParams>
struct parameter
{
    using base = parameter<TParams>;

    template <typename T>
    constexpr parameter(T&& t) : Value{(T&&)t} { }

    constexpr parameter(const parameter&) = delete;
    constexpr parameter& operator=(const parameter&) = delete;

    TParams Value;
};

struct arg_names : parameter<std::vector<std::string_view>> 
{ 
    constexpr arg_names(std::initializer_list<std::string_view> ls) : base { ls } { }
    constexpr arg_names(std::string_view sv) : base { sv } { }
};
struct default_value : parameter<std::string_view> { using base::base; };  // ...
struct help : parameter<std::string_view> { using base::base; };  // -v : version of...
struct nargs : parameter<int> { using base::base; };  // -l pthread libstdc++ ...
struct is_const : parameter<bool> { using base::base; };  // for -v, it's unchangeable and must have default value
struct required : parameter<bool> { using base::base; };   // optional params

struct check
{

    static bool check_optional(std::string_view sv)
    { return sv.starts_with('-'); }
    
    static bool check_name(const std::vector<std::string_view>& names)
    {
        if (names.size() > 1)
            return std::ranges::all_of(names, &check::check_optional);
        return true;
    }

};


struct parser_result
{
    // std::unordered_map<std::string, size_t, leviathan::string_hash, leviathan::string_key_equal> m_maps;
    leviathan::string_hashmap<std::unordered_map, size_t> m_maps;
    std::vector<std::string> m_values; // store value

    void display() const
    {
        for (auto& [k, v] : m_maps)
            println("k = ", k, " and v = ", m_values[v]);
    }

    template <typename T>
    std::optional<T> get(std::string_view name)
    {
        auto iter = m_maps.find(name);
        if (iter == m_maps.end())
        {
            return { };
        }
        return argument_cast<T>(m_values[iter->second]);
    }

};

class argument_parser
{
public:

    struct info
    {
        std::vector<std::string_view> m_arg_names;
        std::string_view m_help;
        std::string_view m_default_value; // val for type_cast
        int m_nargs = 1;
        bool m_is_const = false;
        bool m_required = false;

        friend std::ostream& operator<<(std::ostream& os, const info& i)
        {
            os.setf(std::ios_base::boolalpha);
            os << "Names = [";
            for (int k = 0; k < i.m_arg_names.size(); ++k)
            {
                if (k != 0) os << ", ";
                os << i.m_arg_names[k];
            }
            os 
                << "] help = " << i.m_help << " nargc = " << i.m_nargs << " const ?: " << i.m_is_const
                << " required ?: " << i.m_required << " value = " << i.m_default_value;
            os.unsetf(std::ios_base::boolalpha);
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

        info i;
        auto params = std::forward_as_tuple(std::move(args)...);

        auto register_name = [this]<typename T>(const T& arg)
        {
            using str = std::string;
            // check
            if constexpr (std::is_same_v<T, arg_names>)
            {
                for (auto name : arg.Value)
                    if (this->m_names.count(name))
                        this->m_errlog.emplace_back(str("Name") + str(name) + "Already Existed");
                    else this->m_names.emplace(name);
            }
        };

        (register_name(args), ...);

#define AssignArgToInfo(name)                                                             \
    {                                                                                     \
        constexpr auto idx = find_first_index_of<TypeList, name>::value;                  \
        if constexpr (idx != npos)                                                        \
            i.m_##name = std::move(std::get<idx>(params).Value);                          \
    }

        AssignArgToInfo(arg_names)
        AssignArgToInfo(default_value)
        AssignArgToInfo(help)
        AssignArgToInfo(nargs)
        AssignArgToInfo(is_const)
        AssignArgToInfo(required)

#undef AssignArgToInfo

        using str = std::string;

        // check const
        if (i.m_is_const && i.m_default_value.empty())
            this->m_errlog.emplace_back(str("Const Attribute") + str(i.m_arg_names[0]) + "Must Have Default Value");

        // check name, if short name supported, the long name must started with `--`, such as --version, -v
        // if short name not supported, longname could be `version` or `--version`
        if (!check::check_name(i.m_arg_names))
            this->m_errlog.emplace_back("Error names");


        // check_require

        m_args.emplace_back(std::move(i));
    }

    std::pair<parser_result, bool> parse_args(int argc, char const *argv[]) 
    {
        m_prop_name = argv[0];

        std::vector<size_t> indices;

        for (size_t i = 0; i < m_args.size(); ++i)
        {
            if (std::ranges::none_of(m_args[i].m_arg_names, &check::check_optional))
            {
                indices.emplace_back(i);
            }
        }

        size_t idx = 0;

        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];
            // println("argv = (", arg, ") with Length:", arg.size());
            std::match_results<std::string_view::iterator> base_match;
            if (std::regex_match(arg.begin(), arg.end(), base_match, Pattern))
            {
                if (base_match.size() != 3)
                    this->m_errlog.emplace_back("Error argv");
                std::string_view value1 = { base_match[1].first, base_match[1].second };
                std::string_view value2 = { base_match[2].first, base_match[2].second };
                auto iter = std::ranges::find_if(m_args, [=](const info& i) {
                    return std::ranges::find(i.m_arg_names, value1) != i.m_arg_names.end();
                });
                using str = std::string;
                if (iter == m_args.end())
                    this->m_errlog.emplace_back(str("Unknown Argument ") + str(value1)); 
                if (iter->m_is_const)
                    this->m_errlog.emplace_back(str("Const Argument") + str(iter->m_arg_names[0]) + "Cannot be Modified");
                iter->m_default_value = value2;
                // println("key = ", key, " value1 = ", value1, " value2 = ", value2);
            }
            else
            {
                if (idx >= indices.size())
                    this->m_errlog.emplace_back("Too much arguments");
                m_args[indices[idx]].m_default_value = arg;
            }

        }
    
        parser_result ret;

        auto lretrive = [](std::string_view sv) {
            auto idx = sv.find_first_not_of('-');
            return sv.substr(idx);
        };

        for (auto& info : m_args)
        {
            using str = std::string;
            if (info.m_required && info.m_default_value.empty())
                this->m_errlog.emplace_back(str("Required Argument") + str(info.m_arg_names[0]) + "is Null");
            ret.m_values.emplace_back(info.m_default_value);
            for (auto sv : info.m_arg_names)
                ret.m_maps[std::string(lretrive(sv))] = ret.m_values.size() - 1;
        }


        return { ret, m_errlog.empty() };
    }

    template <typename T>
    std::optional<T> get(const std::string& name)
    {
        auto iter = std::find_if(m_args.begin(), m_args.end(), [&](const info& i)
        {
            return std::ranges::find(i.m_arg_names, name) != i.m_arg_names.end();
        });
        if (iter == m_args.end())
        {
            return { };
        }
        return argument_cast<T>(iter->m_default_value);
    }

    void report_error() const 
    {
        for (auto& msg : m_errlog)
            std::cout << msg << '\n';
    }

private:

    inline static const std::regex Pattern{ "(.*)=(.*)" };

    std::vector<info> m_args;
    std::string m_prop_name;
    leviathan::string_hashset<std::unordered_set> m_names;
    std::vector<std::string> m_errlog; 
};

int main(int argc, char const *argv[])
{
    argument_parser parser;
    parser.add_argument(arg_names{ "--version", "-v" }, is_const(true), default_value("0.0.0"));
    parser.add_argument(arg_names("--epoch"), default_value("15"), help("epoch num of your training"));
    parser.add_argument(arg_names("--lr"), default_value("2e-5"));
    parser.add_argument(arg_names("nooptional"));
    auto [res, ok] = parser.parse_args(argc, argv);
    if (ok)
    {
        res.display();
        std::cout << *res.get<int>("epoch") << '\n';
        std::cout << *res.get<float>("lr") << '\n';
        std::cout << *res.get<std::string>("v") << '\n';
        std::cout << *res.get<std::string>("nooptional") << '\n';
    }
    else
    {
        parser.report_error();
    }
    std::cout << leviathan::cat_string("Hello", " World", 1, true, std::string_view{ "XXX\n" });
    std::cout << "OK\n";
    return 0;
}


