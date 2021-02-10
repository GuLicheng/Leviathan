#ifndef __CONSOLE_ENUM_HPP__
#define __CONSOLE_ENUM_HPP__



namespace leviathan::io
{

    enum struct console_color : unsigned char
    {
        black = 0,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,

        bright_black,
        bright_red,
        bright_green,
        bright_yellow,
        bright_blue,
        bright_magenta,
        bright_cyan,
        bright_white

    };

    enum struct console_style : unsigned char
    {
        reset = 0,
        bold,
        faint,
        italic,
        underline,
        slow_blink,
        rapid_blink
    };

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

        "\033[90m", // bright black
        "\033[91m", // bright red
        "\033[92m", // bright green
        "\033[93m", // bright yellow 
        "\033[94m", // bright blue
        "\033[95m", // bright magenta 
        "\033[96m", // bright cyan 
        "\033[97m", // bright white


        // background
        "\033[40m", // black
        "\033[41m", // red
        "\033[42m", // green
        "\033[43m", // yellow 
        "\033[44m", // blue
        "\033[45m", // magenta 
        "\033[46m", // cyan 
        "\033[47m" // white

        "\033[100m", // bright black
        "\033[101m", // bright red
        "\033[102m", // bright green
        "\033[103m", // bright yellow 
        "\033[104m", // bright blue
        "\033[105m", // bright magenta 
        "\033[106m", // bright cyan 
        "\033[107m", // bright white
    };

    constexpr inline static const char* style[] =
    {   
        "\033[0m", // reset
        "\033[1m", // bold 
        "\033[2m", // faint
        "\033[3m", // italic
        "\033[4m", // underline
        "\033[5m", // slow_blink
        "\033[6m", // rapid_blink
    };   


} //  namespcace io


#endif