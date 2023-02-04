#pragma once

#include <exception> // std::terminate
#include <assert.h>  // assert

#ifndef panic
#define panic(x) do { assert(x && false); std::terminate(); } while (0)
#endif


#include <stdio.h>  // printf

struct FMT
{
    template <typename... Args>
    auto Printf(Args... args) const
    {
        return ::printf(args...);
    }
};

inline constexpr FMT fmt;


#include <expected>
#include <string>
#include <cstdint>
#include <optional>

namespace go2cpp
{
    // std::stoi without exception
    inline std::expected<std::int64_t, std::string> string_to_integer(const std::string& str, int base = 10)
    {   
        int& err = errno;    
        const char* ptr = str.c_str(); 
        char* end_ptr;
        err = 0;

        const auto ans = ::strtoll(ptr, &end_ptr, base);
        
        if (ptr == end_ptr)
            return std::unexpected<std::string>("invalid argument");
        if (err == ERANGE) 
            return std::unexpected<std::string>("out of range");
        return static_cast<std::int64_t>(ans);
    }


    // std::stod without exception
    inline std::expected<std::int64_t, std::string> string_to_floating(const std::string& str) 
    {
        int& err = errno;    
        const char* ptr = str.c_str(); 
        char* end_ptr;
        err = 0;

        const auto ans = ::strtod(ptr, &end_ptr);

        if (ptr == end_ptr)
            return std::unexpected<std::string>("invalid argument");
        if (err == ERANGE) 
            return std::unexpected<std::string>("out of range");
        return static_cast<double>(ans);
    }

    // std::optional version
    inline std::optional<double> string_to_floatingV2(const std::string& str) 
    {
        int& err = errno;    
        const char* ptr = str.c_str(); 
        char* end_ptr;
        err = 0;

        const auto ans = ::strtod(ptr, &end_ptr);

        if (ptr == end_ptr || err == ERANGE)
            return std::nullopt;

        return static_cast<double>(ans);
    }

    inline std::optional<std::int64_t> string_to_integerV2(const std::string& str, int base = 10) 
    {
        int& err = errno;    
        const char* ptr = str.c_str(); 
        char* end_ptr;
        err = 0;

        const auto ans = ::strtoll(ptr, &end_ptr, base);

        if (ptr == end_ptr || err == ERANGE)
            return std::nullopt;

        return static_cast<std::int64_t>(ans);
    }

} // namespace go2cpp

#include <cfloat>

namespace lua
{
    using int64 = std::int64_t;
    using uint64 = std::uint64_t;
    using int32 = std::int32_t;
    using uint32 = std::uint32_t;
    using float32 = float;
    using float64 = double;
    
} // namespace lua


// For functions that return a pair which the first argument 
// is return type and the second argument is a tag indicating 
// whether the first argument is available, we modify the return type from pair to expected
namespace lua
{
    class LuaError
    {

    public:

        constexpr static std::string_view InvalidArgument = "invalid argument";
        constexpr static std::string_view OutOfRange = "out of range";

        LuaError(std::string info) : error_info(std::move(info)) { }

    private:
        std::string error_info;  
    };

    template <typename T>
    using Expected = std::expected<T, LuaError>;

    using UnExpected = std::unexpected<LuaError>;

    inline Expected<int64> lua_stoi(const std::string& str, int base = 10)
    {
        return go2cpp::string_to_integer(str, base);   
    }

    inline Expected<float64> lua_stof(const std::string& str)
    {
        return go2cpp::string_to_floating(str);   
    }

} // namespace lua








