#include "INI.hpp"


namespace leviathan::INI
{



}  // namespace leviathan

int main()
{
    using namespace leviathan::INI;
    INI_handler ini;
    ini.load("./rulesmo.ini");
    ini.show();
    std::cout << "lines : " << ini.lines << std::endl;
    std::cout << "Test successfully\n";
}





