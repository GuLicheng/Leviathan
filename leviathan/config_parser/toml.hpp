/*
    Reference:
        - https://mojotv.cn/2018/12/26/what-is-toml
        - https://toml.io/cn/v1.0.0
        - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
*/

#pragma once

#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/value.hpp>
#include <leviathan/string/string_extend.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include <ranges>
#include <chrono>
#include <format>

namespace leviathan::config::toml
{
    using leviathan::string::ltrim;
    using leviathan::string::rtrim;
    using leviathan::string::trim;
    using leviathan::string::arithmetic;
    using leviathan::string::string_hash_key_equal;
    // using leviathan::string::whitespace_delimiters;
    using leviathan::string::string_viewable;

    using std::string_view;
    using std::string;

    // Some exceptions
    struct toml_parse_error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct bad_toml_value_access : std::exception
    {
        const char* what() const noexcept override
        { return "bad_toml_value_access"; }
    };

    template <typename... Args>
    void throw_toml_parse_error(string_view fmt, Args&&... args)
    {
        auto msg = std::vformat(fmt, std::make_format_args((Args&&) args...));
        throw toml_parse_error(msg);
    }

    // Can we use std::chrono directly?
    // YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
    struct toml_data_time
    {
        int m_year = 0;
        int m_month = 0;
        int m_day = 0;

        int m_hour = 0;
        int m_minute = 0;
        int m_second = 0;
        int m_microsecond = 0;

        int m_hour_offset = 0;
        int m_minute_offset = 0;
    };

    template <typename T>
    struct use_pointer 
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);

        constexpr static bool value = sizeof(T) > 16;

        using type = std::conditional_t<value, std::shared_ptr<T>, T>;
    };

    struct to_shared_ptr
    {
        template <typename T>
        constexpr auto operator()(T&& t) const
        {
            using U = std::remove_cvref_t<T>;
            if constexpr (use_pointer<U>::value)
            {
                return std::make_shared<U>((T&&)t);
            }
            else
            {
                return t;
            }
        }
    };

    class toml_value;

    using toml_boolean = bool;
    using toml_integer = int64_t;
    using toml_float = double;
    using toml_string = string;
    using toml_array = std::vector<toml_value>;
    using toml_table = std::unordered_map<toml_string, toml_value, string_hash_key_equal, string_hash_key_equal>;

    // Since the static array cannot be table array, we can add a bool tag in 
    // toml_array to distinct the array and table array. However, we want to
    // avoid creating wheel, so we use another type toml_array_table which
    // is same as toml_array but can add toml_table when parsing.
    using toml_table_array = std::vector<toml_table>;

    using toml_value_base = value_base<
        std::variant, 
        to_shared_ptr,
        toml_boolean,
        toml_integer,
        toml_float,
        toml_string,
        toml_array, 
        toml_table,
        toml_data_time
    >;

    class toml_value : public toml_value_base
    {
    public:

        toml_value() = delete;

        using toml_value_base::toml_value_base;
        using toml_value_base::operator=;

        template <typename T>
        bool is() const
        {
            using U = typename mapped<T>::type;
            return std::holds_alternative<U>(m_data);
        }

        template <typename T>
        T& as()
        {
            if constexpr (is_mapped<T>)
            {
                auto& fancy_ptr = std::get<typename mapped<T>::type>(m_data);
                return *std::to_address(fancy_ptr);
            }
            else
            {
                return std::get<T>(m_data);
            }
        }

        template <typename T>
        T* as_ptr()
        {
            auto ptr = std::get_if<typename mapped<T>::type>(m_data);
            return ptr ? std::to_address(ptr) : nullptr;
        }

        toml_table& as_table() 
        { return as<toml_table>(); }
        
        toml_array& as_array() 
        { return as<toml_array>(); }
        
        toml_boolean& as_boolean()
        { return as<toml_boolean>(); }

        toml_integer& as_integer()
        { return as<toml_integer>(); }

        toml_string& as_string()
        { return as<toml_string>(); }

        toml_float& as_float()
        { return as<toml_float>(); }

        toml_data_time& as_data_time()
        { return as<toml_data_time>(); }
    };

    namespace detail
    {
        struct config
        {
            constexpr static int value = 256;

            int operator()(size_t x) const
            {
                [[assume(x < value)]];
                static string_view sv = R"(_-. )";
                return isalnum(x) || sv.contains(x);
            }
        };

        /**
         * @brief Split key by '.'.
         * 
         * @return Result of split. The result will not be empty.
        */
        inline std::vector<string_view> split_keys(string_view key)
        {
            assert(key.size());

            std::vector<string_view> result;

            static auto table = make_character_table(config());

            auto parse_quote = [&](char ch) {

                auto sv = key;

                assert(sv.size() && sv.front() == ch);

                sv.remove_prefix(1);

                auto next_quote = sv.find(ch);

                if (next_quote == sv.npos)
                {
                    throw_toml_parse_error("Parsing quote key error.");
                }

                result.emplace_back(sv.substr(0, next_quote));

                sv.remove_prefix(1 + next_quote);

                key = ltrim(sv);
            };

            while (1)
            {
                char ch = key.front();

                if (ch == '"')
                {
                    parse_quote('"');
                }
                else if (ch == '\'')
                {
                    parse_quote('\'');
                }
                else if (table[ch])
                {
                    // pase bar string
                    auto head = key.begin(), tail = key.end();
                    for (; head != tail; ++head)
                    {
                        auto ch2 = *head;
                        if (ch2 == '.' || ch2 == ' ')
                        {
                            result.emplace_back(key.begin(), head);
                            key = ltrim(string_view(head, tail));
                            break;
                        }
                        else if (!table[*head])
                        {
                            throw_toml_parse_error("Invalid character when parsing bare key."); 
                        }
                    }
                }
                else
                {
                    throw_toml_parse_error("Invalid character when parsing bare key.");
                }

                if (key.empty())
                {
                    throw_toml_parse_error("Only key.");
                }
            
                ch = key.front();

                if (ch == '=')
                {
                    // OK
                    return result;
                }
                else if (ch == '.')
                {
                    key.remove_prefix(1);
                    key = ltrim(key);
                    continue;
                }
                else
                {
                    throw_toml_parse_error("Expected '=' or '.', but got {}", ch);
                }
            }
        }
    }

    class parser
    {
        /**
         * After parsing an entry, we must known the type of current root.
         * If current root is:
         * 
         * - toml_table(m_cur_array is nullptr): we add
         *   this entry to m_cur_table. 
         * 
         * - table array(m_cur_array is not nullptr): we add
         *   this entry to m_cur_table first and when parsing next table, we try
         *   append m_cur_table to m_cur_table.
        */
        struct table_root
        {
            toml_table* m_cur_table = nullptr;              // For singe table.
            toml_array* m_cur_array = nullptr;              // For table array.
        };

        string m_context;                                   // Save origin string.
        std::vector<std::string_view> m_lines;              // Split origin string line by line.
        std::vector<std::string_view>::iterator m_lineit;   // Current line.
        string_view m_line;                                 // Current line for parsing. For simplicity, we use string_view to replace iterator/pointer. 
        toml_table m_global;                                // Toml root table.
        table_root m_cur_root;                              // Current table root(section).

        static constexpr string_view linefeed = "\n";

        inline static const toml_table empty_table;         // For try_emplace

    public:

        parser(string context) 
            : m_context(std::move(context)) { }
    
        toml_value operator()() &&
        {
            read_lines();
            for (auto line : m_lines)
            {
                // std::cout << line << '\n';
                // std::cout << std::format("line = ({}) and last char is ({})\n", line, line.back());
            }
            parse();
            return toml_value(std::move(m_global));
        }

    private:

        /**
         * @brief Split context by line and remove empty line.
        */
        void read_lines()
        {
            // Every line may ends with \r\n on Windows.
            // We replace all \r\n to \n, this is not efficient but simple.
            m_context = leviathan::string::replace(std::move(m_context), "\r\n", "\n");

            // Directly set 
            // std::ranges::split_view<std::ranges::ref_view<std::string>, std::string_view>
            // as a field may be better.
            for (auto line : m_context | std::views::split(linefeed))
            {
                m_lines.emplace_back(line);
            }
        }

        bool getline()
        {
            if (m_lineit == m_lines.end())
            {
                return false;
            }
            m_line = *m_lineit++;
            return true;
        }

        void parse()
        {
            while (getline())
            {
                m_line = ltrim(m_line);

                // Empty line or just commit.
                if (m_line.empty() || m_line.front() == '#')
                {   
                    // U+0000 - U+0008 and U+000A - U+001F,U+007F should not appear in commit.
                    continue;
                }

                if (m_line.front() == '[')
                {
                    // [table] or [[table_array]]
                    // Maybe we should merge current root
                    parse_table_or_table_array();
                }
                else
                {
                    // parse key and value.
                    parse_entry();
                }
            }
        }

        void parse_entry()
        {
            skip_whitespace();

            assert(m_line.size());

            if (m_line.front() == '=')
            {
                throw_toml_parse_error("Empty Key.");
            }

            // parse key
            parse_key();

            skip_whitespace();

            // match '='
            if (!match_and_advance('='))
            {
                throw_toml_parse_error("Expected '=' after key.");
            }

            skip_whitespace();

            // parse value
            parse_value();
        }

        void parse_value();

        void parse_table_or_table_array()
        {
            assert(m_line.front() == '[');

            advance_unchecked(1); // eat [

            if (m_line.empty())
            {
                throw_toml_parse_error("Unexpected end of table.");
            }

            if (current() == '[')
            {
                parse_table_array();
            }
            else
            {
                parse_table();
            }
        }

        void parse_table_array()
        {
            advance_unchecked(1); // eat '['.

            const auto idx = m_line.find_first_not_of("]");

            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Unexpected end of table array");
            }

            auto table_name = m_line.substr(0, idx);

            try_create_table_array(table_name);

            m_line.remove_prefix(idx + 1); // remove table name and ']'.

            if (m_line.empty() || m_line.front() != ']')
            {
                throw_toml_parse_error("Unexpected end of table array");
            }

            advance_unchecked(1); // eat second ']'.

            consume_whitespace_and_comment();
        }

        void parse_table()
        {
            advance_unchecked(1); // eat '['.

            const auto idx = m_line.find_first_not_of(']');

            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Unexpected end of table.");
            }

            auto table_name = m_line.substr(0, idx);

            try_create_single_table(table_name);

            m_line.remove_prefix(idx + 1); // remove table name and ']'.

            consume_whitespace_and_comment();
        }

        void try_create_table_array(string_view table_name)
        {
            auto keys = detail::split_keys(table_name);

            toml_table* root = &m_global;

            bool ok = false;   // Check whether all keys are already exist.

            for (auto key : keys)
            {
                assert(root);
                auto [it, succeed] = root->try_emplace(toml_string(key), empty_table);
                ok = ok || succeed;
                root = it->second.as_ptr<toml_table>();
            }

            if (!ok)
            {
                throw_toml_parse_error("Redefinition table.");
            }

            m_cur_root.m_cur_table = root;
            m_cur_root.m_cur_array = nullptr;
        }

        void try_create_single_table(string_view table_name)
        {
            auto keys = detail::split_keys(table_name);

            toml_table* root = &m_global;

            bool ok = false;   // Check whether all keys are already exist.

            for (auto key : keys)
            {
                assert(root);
                auto [it, succeed] = root->try_emplace(toml_string(key), empty_table);
                
                if (!succeed && !it->second.is<toml_table>())
                {
                    /**
                     * [x]
                     * y = 1
                     * 
                     * [x.y] // error, x.y is integer.
                     * 
                    */
                    throw_toml_parse_error("Table conflict.");
                }

                ok = ok || succeed;
                root = it->second.as_ptr<toml_table>();
            }

            if (!ok)
            {
                throw_toml_parse_error("Redefinition table.");
            }

            m_cur_root.m_cur_table = root;
            m_cur_root.m_cur_array = nullptr;
        }

        /**
         * @brief Remove the blank and commit of current line.
         *  After removing, the m_line should be empty. This
         *  function is just for handing error case, and ignore 
         *  it will not cause parse error.
        */
        void consume_whitespace_and_comment()
        {
            m_line = ltrim(m_line);
            if (m_line.size() && m_line.front() != '#')
            {
                throw_toml_parse_error("Unknown character.");
            }
            m_line = "";
        }

        void parse_key()
        {
            struct raw_character_config
            {
                int operator()(size_t x) const
                {
                    return isalnum(x) || x == '_' || x == '-';
                }
            };

            static auto raw_characters = make_character_table(raw_character_config());

            throw_toml_parse_error("Not implement");
        }

        bool match_and_advance(char ch)
        {
            if (m_line.empty())
            {
                return false;
            }
            bool result = current() == ch;
            advance_unchecked(1);
            return result;
        }

        void skip_whitespace()
        {
            struct whitespace_config
            {
                constexpr int operator()(size_t i) const
                {
                    [[assume(i < 128)]];
                    constexpr std::string_view sv = " \r\n\t";
                    return sv.contains(i);
                }
            };

            static auto whitespaces = make_character_table(whitespace_config());

            auto is_whitespace = [](char ch) { return whitespaces[ch]; };
            for (; m_line.size() && is_whitespace(current()); advance_unchecked(1));
        }

        char peek(int n) const
        { return m_line[n]; }

        char current() const
        { return peek(0); }

        void advance_unchecked(size_t n)
        { m_line.remove_prefix(n); }

    };


} // namespace leviathan::config::toml
















