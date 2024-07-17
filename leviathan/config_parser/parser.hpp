#pragma once

#include "common.hpp"

namespace leviathan::config
{
    
template <typename Decoder>
class parser
{
public:

    parser() = default;

    auto operator()(std::string source) 
    {
        m_content = std::move(source);
        return Decoder(m_content)();
    }

private:

    std::string m_content;
};


} // namespace leviathan::config

