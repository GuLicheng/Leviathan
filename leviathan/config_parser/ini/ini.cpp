#include <sstream>
#include <leviathan/config_parser/ini.hpp>
#include <catch2/catch_all.hpp>

using namespace cpp::config::ini;

read_only_configuration parse_configuration_by_context(std::string c)
{
    auto context = std::make_unique<string>(std::move(c));

    auto sections = *context
        | std::views::split(detail::linefeed)
        | std::views::transform(detail::get_section_or_entry_context)
        | std::views::filter(std::ranges::size)
        | std::views::chunk_by(detail::chunk_section_and_entries)  
        | std::views::transform(detail::to_section);

    hashmap<string_view, hashmap<string_view, string_view>> result;
    
    // If some entries shared same section name, we try to merge them.
    for (auto&& [section, entries] : sections)
    {
        auto [it, ok] = result.try_emplace(section, std::move(entries));
        if (!ok)
            it->second.merge(std::move(entries));
    }
    return { std::move(result), std::move(context) };
}

TEST_CASE("trim blank and comment")
{
    std::string context = R"delimiter(
            ; This is comment.
            # This is comment.
        [Boris]
        name = Boris ; Hero's name.
        cost = 1500  # Hero's cost.

    )delimiter";

    auto ini = parse_configuration_by_context(std::move(context));

    REQUIRE(ini.section_count() == 1);
    REQUIRE(ini.entry_count("Boris") == 2);
    REQUIRE(*ini["Boris", "name"] == "Boris");
    REQUIRE(*ini["Boris", "cost"] == "1500");
}

TEST_CASE("section only")
{
    std::string context = R"delimiter(
        [SUNB]
        [AREDDAWN.MAP2]
        [BEAG]
    )delimiter";

    auto ini = parse_configuration_by_context(std::move(context));

    REQUIRE(ini.section_count() == 3);
    REQUIRE(ini.entry_count("SUNB") == 0);
    REQUIRE(ini.entry_count("BEAG") == 0);
    REQUIRE(ini.entry_count("AREDDAWN.MAP2") == 0);
    REQUIRE(ini.entry_count("UnknownSection") == 0);
}

// No sections will cause assert.false in runtime.
// TEST_CASE("entry only")
// {
//     std::string context = R"delimiter(
//         Briefing=Brief:AREDDAWN
//         UIName=MAP:TITREDDAWN
//         LSLoadMessage=LoadMsg:AREDDAWN
//         LSLoadBriefing=LoadBrief:AREDDAWN
//         LS640BriefLocX=20
//         LS640BriefLocY=20
//         LS800BriefLocX=20
//         LS800BriefLocY=20
//         LS640BkgdName=A01B.SHP
//         LS800BkgdName=A01B.SHP
//         LS800BkgdPal=A01B.PAL
//         LoadScreenText.Color=LightGrey
//     )delimiter";

//     auto ini = parse_configuration_by_context(std::move(context));
// }

TEST_CASE("empty file")
{
    std::string context = R"""(
        ; This file only contains comments.
        ; Hi!
        ; What's your name?
        ; Do you like RA2?
    )""";

    auto ini = parse_configuration_by_context(std::move(context));

    REQUIRE(ini.section_count() == 0);
}

TEST_CASE("Mental Omega 3.3.6")
{
    auto ini = parse_configuration(R"(D:\Library\Leviathan\leviathan\config_parser\config\RA2.ini)");

    if (ini.section_count() != 0)
    {

        REQUIRE(ini.section_count() == 4725);

        REQUIRE(ini["General", "Name"] == "Mental Omega 3.3.6 --- Official Rules of Engagement");
        REQUIRE(ini["General", "RepairRate"] == ".03");
        REQUIRE(ini["General", "RepairPercent"] == "10%");
        REQUIRE(ini["General", "SmallVisceroid"] == "");
        REQUIRE(ini["General", "LargeVisceroid"] == "");
        REQUIRE(ini["General", "AIIonCannonConYardValue"] == "60,60,60");
        
        REQUIRE(ini.entry_count("AircraftTypes") == 51);

        REQUIRE(ini["BuildingTypes", "0"] == "GACNST");

        REQUIRE(ini["CABORS" ,"TechLevel"] == "-1");
        REQUIRE(ini["CABORS" ,"Strength"] == "500");
        REQUIRE(ini["BORISWHE" ,"Verses"] == "220%,210%,110%,70%,60%,50%,0%,0%,0%,100%,100%");
        REQUIRE(ini["BORISWHE" ,"Versus.libra"] == "105%");
    }
}


// int main()
// {
//     using namespace cpp::config::ini;

//     auto result = parse_configuration(R"(D:\Library\Leviathan\leviathan\config_parser\config\RA2.ini)");

//     result.display();

//     configuration writer(result);

//     // auto d = result.get_value("AREDDAWN.MAP", "LS640BriefLocX").cast<int>();
//     auto d = result["AREDDAWN.MAP", "LS640BriefLocX"].cast<int>();

//     // writer.set_value("Boris", "Cost", 1500);
//     // writer.set_value("Boris", "Name", "Boris");

//     writer["Boris", "Cost"] = "1500";
//     writer["Boris", "Name"] = "Boris";

//     assert(*d == 20);

//     writer.save(std::cout);

// }
