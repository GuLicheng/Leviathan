#ifndef __JSON_HPP__
#define __JSON_HPP__

#include "./base.hpp"
#include <lv_cpp/io/console.hpp>

#include <stdlib.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <string_view>


namespace leviathan::parser
{

    template <typename Key, typename Value>
    void regester_map(std::unordered_map<Key, Value>& map1, std::unordered_map<Value, Key>& map2, Key key, Value value)
    {   
        map1[key] = value;
        map2[std::move(value)] = std::move(key);
    }

    enum class JsonType
    {
        JSON_NULL,
        JSON_FALSE,
        JSON_TRUE,
        JSON_ARRAY,
        JSON_STRING,
        JSON_OBJECT,
        JSON_NUMBER
    };

    std::ostream& operator<<(std::ostream& os, JsonType json_type)
    {
        return os << static_cast<int>(json_type);
    }

    inline std::unordered_map<JsonType, std::string> JsonType2String;
    inline std::unordered_map<std::string, JsonType> String2JsonType;

    #define REGESTER_TYPE(var) regester_map(JsonType2String, String2JsonType, var, std::string(#var))

    void init()
    {
        REGESTER_TYPE(JsonType::JSON_ARRAY);
        REGESTER_TYPE(JsonType::JSON_FALSE);
        REGESTER_TYPE(JsonType::JSON_TRUE);
        REGESTER_TYPE(JsonType::JSON_NULL);
        REGESTER_TYPE(JsonType::JSON_NUMBER);
        REGESTER_TYPE(JsonType::JSON_OBJECT);
        REGESTER_TYPE(JsonType::JSON_STRING);
    }

    #undef REGESTER_TYPE

    enum class JsonToken
    {

    };

    class Lexer
    {
    public:
        std::string m_context;
        size_t m_cur;
        std::list<error_log> m_errors;
        std::vector<std::string> m_parse_result;
        
    public:

        Lexer(std::string context); 

        Lexer& parse();
        
        void parse_string();

        // void parse_number()
        // {
        //     auto begin = &m_context[m_cur];
        //     char* end;
        //     double res = ::strtod(begin, &end);
        //     if (begin != end)
        //     {
        //         m_parse_result.emplace_back(std::to_string(res));
        //         m_cur += end - begin;
        //     }
        //     else
        //     {

        //         while (!is_over())
        //         {
        //             char c = m_context[m_cur];
        //             if (c == '\n' || c == ' ' || c == '\t' || c == '\r') break;
        //             else m_cur++;
        //         }
        //     }
        // }

        int next_blank() const;
        void parse_number();
        void parse_array();
        void parse_object();
        void next();
        bool is_over() const noexcept;
        bool match(size_t len, const char* value);
        void parse_null();
        void parse_true();
        void parse_false();
        void skip_blank();

        void report()
        {
            console::write_line(m_parse_result);
            console::write_line(m_errors);
        }

    };

}


#endif