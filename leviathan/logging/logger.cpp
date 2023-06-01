#include "logger.hpp"

namespace logging = leviathan::logging;

int main(int argc, char const *argv[])
{
    const char* path = "D:\\Library\\Leviathan\\a.log";

    logging::logger logger("root");

    auto handler = logging::make_handler<logging::file_handler>("file", path);

    logger.add_handler(std::move(handler));
    
    logger.debug("This is a debugging.");

    logger.info("This is a testing.");

    logger.warn("This is a warning.");

    logger.error("This is an error.");

    logger.write();

    std::cout << "Ok\n";


    return 0;
}



