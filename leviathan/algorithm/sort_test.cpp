#include "benchmark_sorter.hpp"

#include "intro_sort.hpp"
#include "power_sort.hpp"
#include "tim_sort.hpp"
#include "pdq_sort.hpp"

using namespace cpp::ranges::detail;

using TimSorter = decltype(cpp::ranges::tim_sort);
using PythonPowerSorter = sorter<power_sorter<24, power_policy::python>>;
using JavaPowerSorter = sorter<power_sorter<24, power_policy::java>>;

int main()
{
    cpp::benchmark_sorter<>()
        .add_sorter(TimSorter(), "Tim Sort")
        .add_sorter(PythonPowerSorter(), "Python Power Sorter")
        .add_sorter(JavaPowerSorter(), "Java Power Sorter")
        .random(1e7)
        .random_string(1e6)
        .ascending()
        .descending()
        .all_zeros()
        .pipeline()
        ;
}
