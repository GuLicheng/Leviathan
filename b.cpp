#include <regex>
#include <string>
#include <iostream>
#include <chrono>
#include <format>

static std::regex re(R"(^(?:(\d+)-(0[1-9]|1[012])-(0[1-9]|[12]\d|3[01]))?([\sTt])?(?:([01]\d|2[0-3]):([0-5]\d):([0-5]\d|60)(\.\d+)?((?:[Zz])|(?:[\+|\-](?:[01]\d|2[0-3])(?::[0-6][0-9])?(?::[0-6][0-9])?))?)?$)");

namespace chrono = std::chrono;

struct datetime
{
    int m_year;
    int m_month;
    int m_day;

    int m_hour;
    int m_minute;
    int m_second;
    
    // int m_millisecond;
    // int m_microsecond;
    std::string m_offset;

    friend std::ostream& operator<<(std::ostream& os, const datetime& dt)
    {
        return os << std::vformat("{}-{}-{}T{}:{}:{}{}", std::make_format_args(
            dt.m_year, dt.m_month, dt.m_day, dt.m_hour, dt.m_minute, dt.m_second, dt.m_offset));
    }

};

datetime report(std::string s) 
{
    std::smatch pieces_match;

    std::cout << std::format("S = {} and size is ", s);

    if (std::regex_match(s , pieces_match, re))
    {
        std::cout << std::format("{} ( ", pieces_match.size());
        for (size_t i = 1; i < pieces_match.size(); ++i)
        {
            if (i) std::cout << ", ";
            std::ssub_match sub_match = pieces_match[i];
            std::string piece = sub_match.str();
            std::cout << piece;
        }
        std::cout << ")\n";

        datetime dt;

        auto to_int = [](auto& slice) {
            return slice.str().empty() ? 0 : std::stoi(slice.str());
        };

        dt.m_year = to_int(pieces_match[2]);
        dt.m_month = to_int(pieces_match[3]);
        dt.m_day = to_int(pieces_match[4]);

        dt.m_hour = to_int(pieces_match[6]);
        dt.m_minute = to_int(pieces_match[7]);
        dt.m_second = to_int(pieces_match[8]);

        return dt;
    }
    else
    {
        std::cout << "Fail to match\n";
        return datetime();
    }
}

int main(int argc, char const *argv[])
{

    report("1979-05-27T07:32:00Z");
    report("1979-05-27T00:32:00-07:00");
    report("1979-05-27T00:32:00.999999-07:00");
    report("1979-05-27 07:32:00Z");
    report("1979-05-27T00:32:00.999999");
    report("1979-05-27T07:32:00");
    report("1979-05-27");
    report("07:32:00");
    report("07:32:00.999999");
    report("07:32:00..999999");
    report("07:32::00..999999");
    report("1979-05-27t07:32:00");
    report("1979-05-27A07:32:00");

    // std::chrono::system_clock::time_point tp;

    return 0;
}

