#include <pybind11/pybind11.h>
#include <leviathan/math/int128.hpp>

namespace py = pybind11;

using leviathan::math::int128_t;
using leviathan::math::uint128_t;

PYBIND11_MODULE(cpp2py, m)
{
    m.doc() = "Test for cpp int128";
    py::class_<uint128_t>(m, "uint128")
        .def(py::init<size_t, size_t>())
        .def(py::init<size_t>());
}
