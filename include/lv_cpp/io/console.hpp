/*
    https://blog.csdn.net/kevinshq/article/details/8179252
    basic_console can work for range, string, pair, tuple 
    If you have overloaded basic_ostream for your own class, 
    basic_console will perform as what your have overloaded
*/
#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include <lv_cpp/utils/template_info.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
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
    reset = 0,
    hight_light,
    reduce_light,
    italic,
    under_line,
    bar
};

template <typename T>
concept number_c = ::std::integral<T> || ::std::floating_point<T>;

template <typename T>
concept range_c = ::std::ranges::range<T>;


template <typename T>
concept string_c = range_c<T> && requires (const T& str)
{
    str.substr(0, 0);
    str.length();
    str.size();
    str.npos;
    str.data();
};

static_assert(string_c<::std::string>);
static_assert(string_c<::std::string_view>);


template <typename T, typename Char, typename Traits = ::std::char_traits<Char>>
concept printable = requires(::std::basic_ostream<Char, Traits>& os, const T& obj)
{
    {os << obj} -> ::std::same_as<::std::basic_ostream<Char, Traits>&>;
};


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

    template <typename... Ts, size_t... Idx>
    static void print_tuple_impl(const ::std::tuple<Ts...>& __tuple, 
        ::std::index_sequence<Idx...>, const _Char* left, const _Char* right, const _Char* delim)
    {
        write(left);
        ((Idx == 0 ? 
            write(::std::get<0>(__tuple)) : 
            (write(delim), write(::std::get<Idx>(__tuple)))), ...);
        write(right);
    }


public:
    using char_type = typename base::char_type;
    using int_type = typename base::int_type;
    using pos_type = typename base::pos_type;
    using off_type = typename base::off_type;
    using traits_type = typename base::traits_type;

    static void write(bool val)
    {
        write(val ? "true" : "false");
    }

    static void write_line(bool val)
    {
        write(val);
        write_line();
    }

    template <typename T> requires (printable<T, _Char>)
    static void write(const T& val)
    {
        os << val; 
    }

    template <typename T> requires (printable<T, _Char>)
    static void write_line(const T& val)
    {
        write(val);
        write_line();
    }


    static void write_line()
    { ::std::endl(os); }

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

    // The iterator must be copyable
    template <typename T> requires (!printable<T, _Char> && range_c<T>)
    static void write(const T& rg)
    {
        write('[');
        auto first = ::std::ranges::begin(rg);
        auto last = ::std::ranges::end(rg);
        for (auto iter = first; iter != last; ++iter)
        {
            if (iter != first) 
                write(", ");
            write(*iter);
        }
        write(']');
    }

    template <typename T> requires (!printable<T, _Char> && range_c<T>)
    static void write_line(const T& rg)
    {
        write(rg);
        write_line();
    }

    template <typename T1, typename T2> requires (!printable<::std::pair<T1, T2>, _Char>)
    static void write(const ::std::pair<T1, T2>& __pair)
    {
        write('(');
        write(__pair.first);
        write(", ");
        write(__pair.second);
        write(')');
    }

    template <typename T1, typename T2> requires (!printable<::std::pair<T1, T2>, _Char>)
    static void write_line(const ::std::pair<T1, T2>& __pair)
    {
        write(__pair);
        write_line();
    }

    template <typename... Ts> requires (!printable<::std::tuple<Ts...>, _Char>)
    static void write(const ::std::tuple<Ts...>& __tuple)
    {
        print_tuple_impl(__tuple, ::std::make_index_sequence<sizeof...(Ts)>(), "(", ")", ", ");
    }

    template <typename... Ts> requires (!printable<::std::tuple<Ts...>, _Char>)
    static void write_line(const ::std::tuple<Ts...>& __tuple)
    {
        write(__tuple);
        write_line();
    }

    template <typename T> requires (!printable<T, _Char> && !range_c<T> && !string_c<T>)
    static void write(const T&)
    { write_type<T>(); }

    template <typename T> requires (!printable<T, _Char> && !range_c<T> && !string_c<T>)
    static void write_line(const T& val)
    {
        write(val);
        write_line();
    }

    // at least one parameter
    template <typename T1, typename... Ts>
    static void write_multi(const T1& t1, const Ts&... ts)
    {
        write(t1);
        ((write(' '), write(ts)), ...);
    }

    // at least one parameter
    template <typename T1, typename... Ts>
    static void write_line_multi(const T1& t1, const Ts&... ts)
    {
        write_multi(t1, ts...);
        write_line();
    }

    // at least one parameter
    template <typename T1, typename... Ts>
    static void write_lines_multi(const T1& t1, const Ts&... ts)
    {
        write_line(t1);
        (write_line(ts), ...);
    }

    // for type
    template <typename... Ts>
    static void write_type()
    {
        write_multi(type_to_str<Ts>()...);
    }

    template <typename... Ts>
    static void write_line_type()
    { 
        write_type<Ts...>();
        write_line();
    }

    template <typename... Ts>
    static void write_lines_type()
    { 
        (write_line_type<Ts>(), ...);
    }

    // for instance please use macro PrintTypeCategory
    // Since we cannot make sure whether (assume declearing: int a = 0;) 
    // object a is int or int&&



    // Here are some functions for screen/console
    static void clear()
    { write("\033[2J"); }

    static void set_foreground_color(console_color c)
    { write(color[int(c)]); }

    static void set_background_color(console_color c)
    { write(color[int(c) + 8]); }

    static void set_fontstyle(console_style s)
    { write(style[int(s)]); }

    static void reset_color()
    { write("\033[49m"); }

    static void reset()
    { set_fontstyle(console_style::reset); }


    // method for input
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

    template <typename T> requires number_c<T>
    static void read(T& val)
    { is >> val; }

    static void read(_Char* str)
    { is >> str; }

    template <typename _String> requires string_c<_String>
    static void read(_String& str)
    { is >> str; }

    // attribute
    static auto& text_writer()
    { return os; }

    static auto& text_reader()
    { return is; }

    static auto& text_error()
    { return err; }



}; //  end of class 


} //  namespace leviathan

using console = leviathan::io::basic_console<char>;
using wconsole = leviathan::io::basic_console<wchar_t>;


#endif