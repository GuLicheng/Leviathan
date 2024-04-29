#pragma once

#include <format>
#include <cstdio>
#include <string>

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args)
{
    ::printf(std::format(fmt, (Args&&) args...).c_str());
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    ::puts(std::format(fmt, (Args&&) args...).c_str());
}


