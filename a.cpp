#include <iostream>
#include <lv_cpp/named_tuple.hpp>
#include <string_view>
#include <lv_cpp/string/fixed_string.hpp>
#include "argparser.hpp"


template <typename NamedTuple>
void parse_to_namespace(NamedTuple& t, argparse::ArgumentParser& parser)
{
    auto do_search = [&]<size_t I>() {
        auto name = t.template name_of<I>();
        using T = std::tuple_element_t<I, NamedTuple>;
        try 
        {
            t.template get_with<I>() = parser.template get<T>(name); 
        }
        catch(std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    };

    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (do_search.template operator()<Is>(), ...);
    }(std::make_index_sequence<std::tuple_size_v<NamedTuple>>());

}

using FiledA = field<"-a", int>;
using FiledB = field<"-b", int>;
using Namespace = ::named_tuple<
    FiledA,
    FiledB
>;


int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("program_name");

    program.add_argument("-a")
    .scan<'i', int>();

    program.add_argument("-b")
    .scan<'i', int>();


  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
  Namespace n;
  parse_to_namespace(n, program);

    std::cout << n["-a"_arg] + n["-b"_arg] << '\n';

  return 0;
}





