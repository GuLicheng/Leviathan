#include <pybind11/pybind11.h>
#include <leviathan/stopwatch.hpp>

namespace py = pybind11;

using Stopwatch = cpp::time::stopwatch;

PYBIND11_MODULE(cpp2py, m)
{
    py::class_<Stopwatch>(m, "Stopwatch")
        .def("start", &Stopwatch::start, "Start the stopwatch")
        .def("stop", &Stopwatch::stop, "Stop the stopwatch")
        .def("reset", &Stopwatch::reset, "Reset the stopwatch")
        .def("restart", &Stopwatch::restart, "Restart the stopwatch")
        .def("is_running", &Stopwatch::is_running, "Check if the stopwatch is running")
        .def("elapsed", &Stopwatch::elapsed<std::chrono::milliseconds>, "Get elapsed time in milliseconds")
        .def("elapsed_seconds", &Stopwatch::elapsed<std::chrono::seconds>, "Get elapsed time in seconds")
        .def("elapsed_nanoseconds", &Stopwatch::elapsed<std::chrono::nanoseconds>, "Get elapsed time in nanoseconds");
}
