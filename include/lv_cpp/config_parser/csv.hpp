#ifndef __CSV_HPP__
#define __CSV_HPP__

#include <vector>
#include <string>
#include <fstream>

namespace csv
{
    class CSV_Handler
    {
    public:
        CSV_Handler() {}
        bool load(const char* path)
        {   
            std::ifstream in{path, std::ios::in | std::ios::binary};
            if (!in.is_open())
                return false;
            

            return true;
        }
    private:
        std::vector<std::string> darr;
        std::string first_line;
        char delimiter;
    }; // CSV_Handler
    
} // namespace csv



#endif