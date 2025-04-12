#pragma once

namespace cpp::config::xml
{
    
enum class error
{
    success = 0,
    no_attribute,
    wrong_attribute_type,
    file_not_found,
    file_not_be_opened,
    file_read_error,
    error_parsing_element,
    error_parsing_attribute,
    error_parsing_text,
    error_parsing_cdata,
    error_parsing_comment,
    error_parsing_declaration,
    error_parsing_unknown,
    error_parsing_mismatched_element,
    error_parsing,
    cannot_find_convert_text,
    no_text_node,
    element_depth_exceeded,

    error_count,
};

} // namespace cpp::config::xml
