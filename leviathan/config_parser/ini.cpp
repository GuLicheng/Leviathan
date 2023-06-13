#include<leviathan/config_parser/ini.hpp>

int main()
{
    using namespace leviathan::config::ini;

    auto result = parse_configuration(R"(D:\Library\Leviathan\leviathan\config_parser\config\RA2.ini)");

    result.display();

    configuration writer(result);

    auto d = result.get_value("AREDDAWN.MAP", "LS640BriefLocX").cast<int>();

    writer.set_value("Boris", "Cost", 1500);
    writer.set_value("Boris", "Name", "Boris");

    assert(*d == 20);

    writer.save(std::cout);

}
