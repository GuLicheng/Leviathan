#ifndef __PARSER_BASE_HPP__
#define __PARSER_BASE_HPP__

#include <string>
#include <iostream>

namespace leviathan::parser
{
    class error_log 
    {
    protected:
        int line;
        std::string contend;
        std::string error_info;
    public:
        error_log(int line, std::string contend, std::string error_info) 
            : line{line}, contend{std::move(contend)}, error_info{std::move(error_info)} { }
        
        void report() const 
        {
            std::cout << "Line: " << line << 
                ", error infomation:  " << error_info << 
                " Seeing here:" << contend << std::endl;  
        }

        ~error_log() { }
    };

    // entry, consist of key-value
    struct entry : public std::pair<std::string, std::string>
    {
        using std::pair<std::string, std::string>::pair;
        using std::pair<std::string, std::string>::operator=;
        const std::string& key() const noexcept
        {
            return this->first;
        }
        std::string& key() noexcept
        {
            return this->first;
        }
        const std::string& value() const noexcept
        {
            return this->second;
        }
        std::string& value() noexcept
        {
            return this->second;
        }
    };

    // overload ostream
    std::ostream& operator<<(std::ostream& os, const entry& e)
    { 
        return os << '(' << e.first << ", " << e.second << ')'; 
    }  

} // namespace leviathan::parser


#endif



