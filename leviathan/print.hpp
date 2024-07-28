#pragma once

#include <leviathan/operators.hpp>

#include <utility>
#include <format>
#include <cstdio>
#include <string>
#include <variant>

// Not support std::generator
template <std::ranges::range View, typename CharT>
struct std::formatter<View, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return m_fmt.parse(ctx);
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const View& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        const char* delimiter = "[";
        for (const auto& value : v)
        {
            *it++ = std::exchange(delimiter, ", ") ;
            it = m_fmt.format(value, ctx);
        }
        return *it++ = "]";
    }

    std::formatter<std::ranges::range_value_t<View>, CharT> m_fmt;
};

template <typename... Ts, typename CharT>
struct std::formatter<std::variant<Ts...>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const std::variant<Ts...>& v, FormatContext& ctx) const
    {
        return std::visit(
            [&](const auto& value) { 
                return std::format_to(ctx.out(), "{}", value);
            }, v
        );
    }
};

enum ConsoleColor : int
{
    Black = 1,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,

    BrightBlack,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
};

enum ConsoleFont : int
{
    Bold       = 1,
    Faint      = 2,
    Italic     = 4,
    Underline  = 8,
    SlowBlink  = 16,
    RapidBlink = 32,
};

template <>
inline constexpr bool leviathan::operators::enum_enable_pipe<ConsoleFont> = true;

// Static class
class Console
{
    struct WriteImpl
    {
        template <typename... Args>
            requires (sizeof...(Args) > 0)
        static void operator()(std::string_view fmt, Args&&... args) 
        {
            auto s = std::vformat(fmt, std::make_format_args(args...));
            ::printf("%s", s.c_str());
        }

        template <typename T>
        static void operator()(const T& x)
        {
            auto s = std::format("{}", x);
            ::printf("%s", s.c_str());
        }
    };

    inline static const char* ForegroundColors[] = 
    {
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
    };

    inline static const char* BackgroundColors[] =
    {
        "\033[40m",  // black
        "\033[41m",  // red
        "\033[42m",  // green
        "\033[43m",  // yellow 
        "\033[44m",  // blue
        "\033[45m",  // magenta 
        "\033[46m",  // cyan 
        "\033[47m",  // white
        "\033[100m", // bright black
        "\033[101m", // bright red
        "\033[102m", // bright green
        "\033[103m", // bright yellow 
        "\033[104m", // bright blue
        "\033[105m", // bright magenta 
        "\033[106m", // bright cyan 
        "\033[107m", // bright white
    };

    inline static const char* FontStyles[] =
    {   
        "\033[1m", // bold 
        "\033[2m", // faint
        "\033[3m", // italic
        "\033[4m", // underline
        "\033[5m", // slow_blink
        "\033[6m", // rapid_blink
    };   

    static void ChangeStyle()
    {
        if (auto value = std::to_underlying(ForegroundColor); 
            std::clamp(1, value, 16) == value && LastStyle.foreground != ForegroundColor)
        {
            WriteImpl::operator()(ForegroundColors[value - 1]);
            LastStyle.foreground = ForegroundColor;
        }

        if (auto value = std::to_underlying(BackgroundColor); 
            std::clamp(1, value, 16) == value && LastStyle.background != BackgroundColor)
        {
            WriteImpl::operator()(BackgroundColors[value - 1]);
            LastStyle.background = BackgroundColor;
        }

        if (auto value = std::to_underlying(Font); value && LastStyle.font != Font)
        {
            for (int i = 0; i < 6; ++i)
            {
                if (value & (1 << i))
                {
                    WriteImpl::operator()(FontStyles[i]);
                }
            }
            LastStyle.font = Font;
        }
    }

    struct Style
    {
        ConsoleColor foreground;
        ConsoleColor background;
        ConsoleFont font;

        constexpr Style() : 
            foreground(ConsoleColor(0)), background(ConsoleColor(0)), font(ConsoleFont(0)) { }
    };

    struct Guard
    {
        ~Guard()
        {
            Console::Reset();
        }
    };

    inline static Guard _{};

    inline static Style LastStyle{};

public:

    inline static ConsoleColor ForegroundColor = ConsoleColor(0);
    inline static ConsoleColor BackgroundColor = ConsoleColor(0);
    inline static ConsoleFont Font = ConsoleFont(0);

    static constexpr auto WriteLine = []<typename... Args>(Args&&... args)
    {
        ChangeStyle();
        WriteImpl::operator()((Args&&)args...);
        WriteImpl::operator()('\n');
    };

    static constexpr auto Write = []<typename... Args>(Args&&... args)
    {
        ChangeStyle();
        WriteImpl::operator()((Args&&)args...);
    };

    static void Reset()
    {
        ForegroundColor = BackgroundColor = ConsoleColor(0);
        Font = ConsoleFont(0);
        WriteImpl::operator()("\033[0m");
    }

    static void ResetColor()
    {
        auto font = Font;
        Reset();
        Font = font;
    }

    static void SetCodePoint()
    {
        system("chcp 65001");
    }
};

