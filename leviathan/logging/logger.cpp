#include <iostream>
#include "logger.hpp"

namespace logging = leviathan::logging;

int main()
{
    const char* path = "D:\\Library\\Leviathan\\a.log";

    logging::logger logger("root", logging::level::warning);

    // auto file_handler = logging::make_handler<logging::file_handler>("file", path);

    auto file_handler = logging::factory<logging::file_handler>::make("file", path);

    file_handler->set_formatter(logging::make_formatter<logging::pattern_formatter>
        ("pattern_formatter", "Time: %(time) - %(level) : %(message) %(file)%(line)."));

    auto console_handler = logging::make_handler<logging::console_handler>("console", std::clog);

    logger.add_handler(std::move(file_handler));

    logger.add_handler(std::move(console_handler));
    
    logger.debug("This is a debugging.");

    logger.info("This is a testing.");

    logger.warn("This is a warning.");

    logger.error("This is an error.");

    logger.critical("This is a fatal message.");

    std::cout << "Ok\n";

    return 0;
}



