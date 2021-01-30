/*
    https://blog.csdn.net/kevinshq/article/details/8179252
*/
#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include <lv_cpp/type_list.hpp>
#include <lv_cpp/utils/template_info.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <concepts>


namespace leviathan::io
{

enum struct console_color : uint8_t
{
    black = 0,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white
};

enum struct console_style : uint8_t
{
    reset,
    hight_light,
    reduce_light,
    italic,
    under_line,
    bar
};

template <typename T>
concept number_c = ::std::integral<T> || ::std::floating_point<T>;

template <typename T>
concept string_c = meta::is_instance<::std::basic_string, T>::value 
                || meta::is_instance<std::basic_string_view, T>::value;

template <typename _Char> 
struct basic_console_base;

template <>
struct basic_console_base<char>
{
    inline static auto& os = ::std::cout;
    inline static auto& is = ::std::cin;
    inline static auto& err = ::std::cerr;
};

template <>
struct basic_console_base<wchar_t>
{
    inline static auto& os = ::std::wcout;
    inline static auto& is = ::std::wcin;
    inline static auto& err = ::std::wcerr;
};

template <typename _Char> 
class basic_console : private basic_console_base<_Char>
{
    using basic_console_base<_Char>::os;
    using basic_console_base<_Char>::is;
    using basic_console_base<_Char>::err;
    using base = ::std::basic_ostream<_Char>;

    // static class 
    basic_console() = delete;

    constexpr inline static const char* color[] = 
    {
        // foreground
        "\033[30m", // black
        "\033[31m", // red
        "\033[32m", // green
        "\033[33m", // yellow 
        "\033[34m", // blue
        "\033[35m", // magenta 
        "\033[36m", // cyan 
        "\033[37m", // white

        // background
        "\033[40m", // black
        "\033[41m", // red
        "\033[42m", // green
        "\033[43m", // yellow 
        "\033[44m", // blue
        "\033[45m", // magenta 
        "\033[46m", // cyan 
        "\033[47m" // white
    };

    constexpr inline static const char* style[] =
    {   
        "\033[0m", // reset
        "\033[1m", // hight_light
        "\033[2m", // reduce_light
        "\033[3m", // italic
        "\033[4m", // under_line
        "\033[5m", // bar
    };                     

public:
    using char_type = typename base::char_type;
    using int_type = typename base::int_type;
    using pos_type = typename base::pos_type;
    using off_type = typename base::off_type;
    using traits_type = typename base::traits_type;

    // method for output
    template <typename T> requires number_c<T> || string_c<T>
    static void write(T val)
    { os << val; }

    template <typename T> requires number_c<T> || string_c<T>
    static void write_line(T val)
    { 
        os << val; 
        write_line(); 
    }

    static void write(const _Char* str)
    { os << str; }

    static void write_line(const _Char* str)
    { 
        os << str; 
        write_line();
    }

    static void write(bool val)
    {
        os << (val ? "true" : "false");
    }

    static void write_line(bool val)
    {
        os << (val ? "true" : "false");
        write_line();
    }

    static void write_line()
    { os << ::std::endl; }

    static void write(const _Char* buffer, off_type count)
    { os.write(buffer, count); }

    static void write_line(const _Char* buffer, off_type count)
    { 
        write(buffer, count); 
        write_line();
    }

    static void write(const _Char* buffer, pos_type index, off_type count)
    { write(buffer + index, count); }

    static void write_line(const _Char* buffer, pos_type index, off_type count)
    {
        write(buffer, index, count);
        write_line();
    }

    template <typename T>
    static void write_type()
    {
        os << type_to_str<T>();
    }

    template <typename T>
    static void write_line_type()
    { 
        write_type<T>();
        write_line();
    }
    // method for input

    static void clear()
    { os << "\033[2J"; }

    static void set_foreground_color(console_color c)
    { os << color[int(c)]; }

    static void set_background_color(console_color c)
    { os << color[int(c) + 8]; }

    static void set_fontstyle(console_style s)
    { os << style[int(s)]; }

    static void reset_color()
    { os << "\033[49m"; }

    static void reset()
    { os << "\033[0m"; }

    // Here are some functions for screen
    template <typename _String = ::std::basic_string<_Char>>
    static _String read_line()
    {
        _String line;
        ::std::getline(is, line);
        return line;
    }

    template <typename _String = ::std::basic_string<_Char>>
    static _String read_line(_Char delim)
    {
        _String line;
        ::std::getline(is, line, delim);
        return line;
    }


    // attribute
    static auto& text_writer()
    { return os; }

    static auto& text_reader()
    { return is; }

    static auto& text_error()
    { return err; }

}; //  end of class 

using console = basic_console<char>;
using wconsole = basic_console<wchar_t>;

} //  namespace leviathan



#endif