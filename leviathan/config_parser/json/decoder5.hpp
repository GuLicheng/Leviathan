/*
    https://spec.json5.org/
    https://github.com/kimushu/json5pp
    https://github.com/danielaparker/jsoncons
*/

#pragma once

#include <leviathan/config_parser/json/decoder.hpp>

namespace cpp::config::json::detail
{

// This decoder is for JSON5, which is a superset of JSON. 
// It allows for more relaxed syntax, such as unquoted keys, 
// single-quoted strings, and trailing commas. The implementation 
// is based on the JSON decoder, with modifications to support JSON5 features.
template <typename Context>
class decoder5 
{

    Context m_ctx;

public:

    decoder5(std::string_view sv) : m_ctx(sv) { }

};

}  // namespace cpp::config::json::detail

